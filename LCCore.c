

#include "LCCore.h"
#include "LCUtils.h"
#include "LivelyC.h"

struct LCObject {
  LCTypeRef type;
  LCInteger rCount;
  LCContextRef context;
  bool persisted;
  char hash[HASH_LENGTH];
  void *data;
};

struct LCStore {
  void *cookie;
  writeData writefn;
  deleteData deletefn;
  readData readfn;
};

struct LCContext {
  LCStoreRef store;
  size_t translationFunsLength;
  stringToType translationFuns[];
};

struct LCSerializationCookie {
  FILE *fp;
  LCObjectRef object;
  bool first;
};

LCObjectRef objectCreate(LCTypeRef type, void* data) {
  LCObjectRef object = malloc(sizeof(struct LCObject));
  if (object) {
    object->rCount = 1;
    object->type = type;
    object->data = data;
    object->persisted = false;
    object->context = NULL;
    object->hash[0] = '\0';
  }
  return object;
}

char *LCUnnamedObject = "LCUnnamedObject";

LCObjectRef objectCreateFromContext(LCContextRef context, LCTypeRef type, char hash[HASH_LENGTH]) {
  LCObjectRef object = objectCreate(type, NULL);
  object->persisted = true;
  object->context = context;
  if (hash) {
    strcpy(object->hash, hash);
  }
  return object;
}

void* objectData(LCObjectRef object) {
  if (!object->data) {
    objectCache(object);
  }
  return object->data;
}

LCTypeRef objectType(LCObjectRef object) {
  return object->type;
}

bool objectImmutable(LCObjectRef object) {
  return objectType(object)->immutable;
}

bool objectsImmutable(LCObjectRef objects[], size_t length) {
  for (LCInteger i=0; i<length; i++) {
    if (objectImmutable(objects[i]) == false) {
      return false;
    }
  }
  return true;
}

LCObjectRef objectRetain(LCObjectRef object) {
  object->rCount = object->rCount + 1;
  return object;
}

static void objectDataDealloc(LCObjectRef object) {
  if (object->data) {
    if(object->type->dealloc) {
      object->type->dealloc(object);
    } else {
      lcFree(object->data);
    }
    object->data = NULL;
  }
}

LCObjectRef objectRelease(LCObjectRef object) {
  object->rCount = object->rCount - 1;
  if (object->rCount == 0) {
    objectDataDealloc(object);
    lcFree(object);
    return NULL;
  }
  return object;
}

LCInteger objectRetainCount(LCObjectRef object) {
  return object->rCount;
}

LCCompare objectCompare(LCObjectRef object1, LCObjectRef object2) {
  if (object1 == NULL) {
    return LCSmaller;
  } else if (object2 == NULL) {
    return LCGreater;
  }
  if(object1->type->compare == NULL) {
    if(object1 == object2) {
      return LCEqual;
    } else {
      if (object1 > object2) {
        return LCGreater;
      } else {
        return LCSmaller;
      }
    }
  } else {
    return object1->type->compare(object1, object2);    
  }
}

LCContextRef objectContext(LCObjectRef object) {
  return object->context;
}

static void objectWalkChildren(LCObjectRef object, void *cookie, childCallback callback) {
  if (object->type->walkChildren) {
    object->type->walkChildren(object, cookie, callback);
  }
}

static void objectSerializeDataWithCallback(LCObjectRef object, void *cookie, callback flushFunct, FILE *fp) {
  object->type->serializeData(object, cookie, flushFunct, fp);
}

// todo: depth parameter not considered - should serialize children as composite objects accordingly
static void serializeChildCallback(void *cookie, char *key, LCObjectRef objects[], size_t length, LCInteger depth) {
  struct LCSerializationCookie *info = (struct LCSerializationCookie*)cookie;
  if (info->first) {
    info->first = false;
  } else {
    fprintf(info->fp, ",\n");
  }
  fprintf(info->fp, "\"%s\": [", key);
  for (LCInteger i=0; i<length; i++) {
    if (i>0) {
      fprintf(info->fp, ",");
    }
    fprintf(info->fp, "\{\"type\": \"%s\", \"hash\": \"%s\"}", typeName(objectType(objects[i])), objectHash(objects[i]));
  }
  fprintf(info->fp, "]");
}

static void objectSerializeWithCallback(LCObjectRef object, void* cookie, callback cb, FILE *fp) {
  if (object->type->serializeData) {
    objectSerializeDataWithCallback(object, NULL, cb, fp);
  } else {
    struct LCSerializationCookie cookie = {
      .fp = fp,
      .object = object,
      .first = true
    };
    fprintf(fp, "{");
    objectWalkChildren(object, &cookie, serializeChildCallback);
    fprintf(fp, "}");
  }
}

void objectSerialize(LCObjectRef object, FILE *fp) {
  objectSerializeWithCallback(object, NULL, NULL, fp);
}

static void objectStoreChildren(LCObjectRef object, char *key, LCObjectRef objects[], size_t length) {
  object->type->storeChildren(object, key, objects, length);
}


static void deserializeJson(LCObjectRef object, json_value *json) {
  for (LCInteger i=0; i<json->u.object.length; i++) {
    char *key = json->u.object.values[i].name;
    json_value *value = json->u.object.values[i].value;
    json_value **objectsInfo = value->u.array.values;
    
    size_t objectsLength = value->u.array.length;
    LCObjectRef objects[objectsLength];
    
    for (LCInteger j=0; j<objectsLength; j++) {
      json_value *objectInfo = objectsInfo[j];
      char *typeString;
      char *hash;
      for (LCInteger k=0; k<objectInfo->u.object.length; k++) {
        char* objectInfoKey = objectInfo->u.object.values[k].name;
        json_value *objectInfoValue = objectInfo->u.object.values[k].value;
        if (strcmp(objectInfoKey, "type")==0) {
          typeString = objectInfoValue->u.string.ptr;
        } else
          if (strcmp(objectInfoKey, "hash")==0) {
            hash = objectInfoValue->u.string.ptr;
          }
      }
      LCContextRef context = objectContext(object);
      objects[j] = objectCreateFromContext(context, contextStringToType(context, typeString), hash);
    }
    objectStoreChildren(object, key, objects, objectsLength);
  }
}

void objectDeserialize(LCObjectRef object, FILE* fd) {
  if (object->type->serializeData) {
    void* data = object->type->deserializeData(object, fd);
    object->data = data;
  } else {
    object->data = object->type->initData();
    LCDataRef data = LCDataCreateFromFile(fd, fileLength(fd));
    char *jsonString = (char*)LCDataDataRef(data);
    json_value *json = json_parse(jsonString);
    deserializeJson(object, json);
  }
}

char* objectHash(LCObjectRef object) {
  if ((object->hash[0] == '\0') || (!object->type->immutable && !object->persisted)) {
    LCPipeRef memoryStream = LCPipeCreate();
    void* context = createHashContext(memoryStream);
    objectSerializeWithCallback(object, context, updateHashContext, LCPipeWriteFile(memoryStream));
    finalizeHashContext(context, object->hash);
    objectRelease(memoryStream);
  }
  return object->hash;
}

static void storeChildCallback(void *cookie, char *key, LCObjectRef objects[], size_t length, LCInteger depth) {
  LCObjectRef parent = (LCObjectRef)cookie;
  for (LCInteger i=0; i<length; i++) {
    objectStore(objects[i], objectContext(parent));
  }
}

void objectStore(LCObjectRef object, LCContextRef context) {
  if (!object->persisted) {
    FILE* fp = context->store->writefn(context->store->cookie, objectType(object), objectHash(object));
    object->context = context;
    objectSerialize(object, fp);
    objectWalkChildren(object, object, storeChildCallback);
    object->persisted = true;
    fclose(fp);
  }
}

void objectsStore(LCObjectRef objects[], size_t length, LCContextRef context) {
  for (LCInteger i=0; i<length; i++) {
    objectStore(objects[i], context);
  }
}

void objectCache(LCObjectRef object) {
  if (!object->data) {
    LCContextRef context = objectContext(object);
    if (context) {
      FILE* fp = context->store->readfn(context->store->cookie, objectType(object), objectHash(object));
      objectDeserialize(object, fp);
      fclose(fp);
    }
  }
}

void objectDeleteCache(LCObjectRef object) {
  if (object->persisted) {
    objectDataDealloc(object);
  }
}

int objectCompareFun(const void * elem1, const void * elem2) {
  void** object1 = (void**)elem1;
  void** object2 = (void**)elem2;
  LCCompare result = objectCompare(*object1, *object2);
  if (result == LCGreater) {
    return  1; 
  }
  if (result == LCSmaller) {
    return -1; 
  }
  return 0;
}

void objectsSort(LCObjectRef objects[], size_t length) {
  qsort(objects, length, sizeof(void*), objectCompareFun);
}

char* typeName(LCTypeRef type) {
  if (type->name) {
    return type->name;
  } else {
    return LCUnnamedObject;
  }
}

bool typeImmutable(LCTypeRef type) {
  return type->immutable;
}

LCFormat typeSerializationFormat(LCTypeRef type) {
  return type->serializationFormat;
}

LCStoreRef storeCreate(void *cookie, writeData writefn, deleteData deletefn, readData readfn) {
  LCStoreRef store = malloc(sizeof(struct LCStore));
  if (store) {
    store->cookie = cookie;
    store->writefn = writefn;
    store->deletefn = deletefn;
    store->readfn = readfn;
  }
  return store;
}

LCContextRef contextCreate(LCStoreRef store, stringToType funs[], size_t length) {
  if (!funs) {
    stringToType coreFun = &coreStringToType;
    funs = &coreFun;
    length = 1;
  }
  LCContextRef context = malloc(sizeof(struct LCContext) + length * sizeof(stringToType));
  if (context) {
    context->store = store;
    context->translationFunsLength = length;
    for (LCInteger i=0; i<length; i++) {
      context->translationFuns[i] = funs[i];
    }
  }
  return context;
}

LCTypeRef contextStringToType(LCContextRef context, char *typeString) {
  for (LCInteger i=0; i<context->translationFunsLength; i++) {
    LCTypeRef type = context->translationFuns[i](typeString);
    if (type) {
      return type;
    }
  }
  return NULL;
}

LCTypeRef coreStringToType(char *typeString) {
  LCTypeRef coreTypes[] = {LCTypeArray, LCTypeData, LCTypeKeyValue, LCTypeMutableArray, LCTypeMutableDictionary, LCTypeString};
  for (LCInteger i=0; i<6; i++) {
    if (strcmp(typeString, typeName(coreTypes[i]))==0) {
      return coreTypes[i]; 
    }
  }
  return NULL;
}

void lcFree(void* object) {
  free(object);
}