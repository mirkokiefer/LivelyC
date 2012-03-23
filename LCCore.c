

#include "LCCore.h"
#include "LCUtils.h"
#include "LivelyC.h"

#define FILE_BUFFER_LENGTH 1024

struct LCObject {
  LCTypeRef type;
  LCInteger rCount;
  LCContextRef context;
  bool persisted;
  char *hash;
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
  LCInteger depth;
};

static char* _objectHash(LCObjectRef object) {
  return object->hash;
}

static void _objectSetHash(LCObjectRef object, char hash[HASH_LENGTH]) {
  if (!object->hash) {
    object->hash = malloc(sizeof(char)*HASH_LENGTH);
  }
  strcpy(object->hash, hash);
}

static bool _objectPersisted(LCObjectRef object) {
  return object->persisted;
}

static void _objectSetPersisted(LCObjectRef object, bool persisted) {
  object->persisted = persisted;
}

LCObjectRef objectCreate(LCTypeRef type, void* data) {
  LCObjectRef object = malloc(sizeof(struct LCObject));
  if (object) {
    object->rCount = 1;
    object->type = type;
    object->data = data;
    object->persisted = false;
    object->context = NULL;
    object->hash = NULL;
  }
  return object;
}

char *LCUnnamedObject = "LCUnnamedObject";

LCObjectRef objectCreateFromContext(LCContextRef context, LCTypeRef type, char hash[HASH_LENGTH]) {
  LCObjectRef object = objectCreate(type, NULL);
  object->persisted = true;
  object->context = context;
  if (hash) {
    _objectSetHash(object, hash);
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

void objectReleaseAlt(void *object) {
  objectRelease(object);
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

// todo: depth parameter not considered - should serialize children as composite objects accordingly
static void serializeChildCallback(void *cookie, char *key, LCObjectRef objects[], size_t length, LCInteger depth) {
  struct LCSerializationCookie *info = (struct LCSerializationCookie*)cookie;
  size_t currentDepth;
  if (info->depth > 0) {
    currentDepth = info->depth;
  } else {
    currentDepth = depth;
  }
  
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
    fprintf(info->fp, "\{\"type\": \"%s\", ", typeName(objectType(objects[i])));
    if (currentDepth > 0) {
      fprintf(info->fp, "\"object\": ");
      objectSerializeToDepth(objects[i], currentDepth-1, info->fp);
    } else {
      char hash[HASH_LENGTH];
      objectHash(objects[i], hash);
      fprintf(info->fp, "\"hash\": \"%s\"", hash);
    }
    fprintf(info->fp, "}");
  }
  fprintf(info->fp, "]");
}

static void objectSerializeWalkingChildren(LCObjectRef object, size_t depth, FILE *fpw) {
  struct LCSerializationCookie cookie = {
    .fp = fpw,
    .object = object,
    .first = true,
    .depth = depth
  };
  fprintf(fpw, "{");
  objectWalkChildren(object, &cookie, serializeChildCallback);
  fprintf(fpw, "}");
}

static FILE* objectSerializedFileToDepth(LCObjectRef object, LCInteger depth) {
  if (object->type->serializeData) {
    return object->type->serializeData(object);
  } else {
    LCMutableDataRef data = LCMutableDataCreate(NULL, 0);
    FILE *fpw = createMemoryWriteStream(data, LCMutableDataAppendAlt, NULL);
    objectSerializeWalkingChildren(object, depth, fpw);
    fclose(fpw);
    return createMemoryReadStream(data, LCMutableDataDataRef(data), LCMutableDataLength(data), false, objectReleaseAlt);
  }
}

static FILE* objectSerializedFile(LCObjectRef object) {
  return objectSerializedFileToDepth(object, 0);
}

void objectSerializeToDepth(LCObjectRef object, LCInteger depth, FILE *fpw) {
  if (object->type->serializeData) {
    FILE *fpr = object->type->serializeData(object);
    pipeFiles(fpr, fpw, FILE_BUFFER_LENGTH);
  } else {
    objectSerializeWalkingChildren(object, depth, fpw);
  }
}

void objectSerialize(LCObjectRef object, FILE *fpw) {
  objectSerializeToDepth(object, 0, fpw);
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
    for (LCInteger j=0; j<objectsLength; j++) {
      objectRelease(objects[j]);
    }
  }
}

static void objectInitData(LCObjectRef object) {
  object->data = object->type->initData();
}

void objectDeserialize(LCObjectRef object, FILE* fd) {
  if (object->type->deserializeData) {
    void* data = object->type->deserializeData(object, fd);
    object->data = data;
  } else {
    objectInitData(object);
    LCMutableDataRef data = LCMutableDataCreate(NULL, 0);
    LCMutableDataAppendFromFile(data, fd, fileLength(fd));
    LCMutableDataAppend(data, (LCByte*)"\0", 1);
    char *jsonString = (char*)LCMutableDataDataRef(data);
    json_value *json = json_parse(jsonString);
    deserializeJson(object, json);
    objectRelease(data);
  }
}

void objectHash(LCObjectRef object, char hashBuffer[HASH_LENGTH]) {
  if (!_objectHash(object) || !object->type->immutable) {
    LCPipeRef memoryStream = LCPipeCreate();
    void* context = createHashContext(memoryStream);
    FILE *fp = objectSerializedFile(object);
    pipeFileToFunction(context, fp, updateHashContext, FILE_BUFFER_LENGTH);
    finalizeHashContext(context, hashBuffer);
    objectRelease(memoryStream);
  } else {
    strcpy(hashBuffer, _objectHash(object));
  }
  if (object->type->immutable) {
    _objectSetHash(object, hashBuffer);
  }
}

static void storeChildCallback(void *cookie, char *key, LCObjectRef objects[], size_t length, LCInteger depth) {
  LCObjectRef parent = (LCObjectRef)cookie;
  for (LCInteger i=0; i<length; i++) {
    objectStore(objects[i], objectContext(parent));
  }
}

void objectStore(LCObjectRef object, LCContextRef context) {
  char hash[HASH_LENGTH];
  hash[0] = '\0';
  if (!object->type->immutable && object->persisted) {
    objectHash(object, hash);
    if (strcmp(hash, _objectHash(object)) != 0) {
      object->persisted = false;
    }
  }
  if (!object->persisted) {
    if (hash[0] == '\0') {
      objectHash(object, hash);
    }
    FILE* fp = context->store->writefn(context->store->cookie, objectType(object), hash);
    object->context = context;
    objectSerialize(object, fp);
    objectWalkChildren(object, object, storeChildCallback);
    object->persisted = true;
    fclose(fp);
    if (!object->type->immutable) {
      _objectSetHash(object, hash);
    }
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
      FILE* fp = context->store->readfn(context->store->cookie, objectType(object), _objectHash(object));
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