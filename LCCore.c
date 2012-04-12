

#include "LCCore.h"
#include "LCUtils.h"
#include "LivelyC.h"
#include "JsonSerialization.h"

#define FILE_BUFFER_LENGTH 1024

void objectWalkChildren(LCObjectRef object, void *cookie, childCallback callback);
static void objectStoreWithCompositeParam(LCObjectRef object, bool composite, LCContextRef context);

struct LCObject {
  LCTypeRef type;
  LCInteger rCount;
  LCContextRef context;
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

static char* _objectHash(LCObjectRef object) {
  return object->hash;
}

static void _objectSetHash(LCObjectRef object, char hash[HASH_LENGTH]) {
  if (!hash) {
    if (object->hash) {
      lcFree(object->hash);
    }
    object->hash = NULL;
  }
  if (!object->hash) {
    object->hash = malloc(sizeof(char)*HASH_LENGTH);
  }
  strcpy(object->hash, hash);
}

LCObjectRef objectCreate(LCTypeRef type, void* data) {
  LCObjectRef object = malloc(sizeof(struct LCObject));
  if (object) {
    object->rCount = 1;
    object->type = type;
    object->data = data;
    object->context = NULL;
    object->hash = NULL;
  }
  return object;
}

char *LCUnnamedObject = "LCUnnamedObject";

LCObjectRef objectCreateFromContext(LCContextRef context, LCTypeRef type, char hash[HASH_LENGTH]) {
  LCObjectRef object = objectCreate(type, NULL);
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
  if (!object) {
    return NULL;
  }
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
  if (object) {
    object->rCount = object->rCount + 1;
  }
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
  if (object) {
    object->rCount = object->rCount - 1;
    if (object->rCount == 0) {
      objectDataDealloc(object);
      lcFree(object);
      return NULL;
    }
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

void objectWalkChildren(LCObjectRef object, void *cookie, childCallback callback) {
  if (object->type->walkChildren) {
    object->type->walkChildren(object, cookie, callback);
  }
}

static void objectSerializeWalkingChildren(LCObjectRef object, LCInteger levels, FILE *fpw) {
  objectSerializeJsonToLevels(object, levels, fpw, objectWalkChildren);
}

void objectSerializeToLevels(LCObjectRef object, LCInteger levels, FILE *fpw) {
  if (object->type->serializeDataBuffered) {
    fpos_t offset = 0;
    while (object->type->serializeDataBuffered(object, offset, FILE_BUFFER_LENGTH, fpw) == FILE_BUFFER_LENGTH) {
      offset = offset + FILE_BUFFER_LENGTH;
      fflush(fpw);
    }
  } else if (object->type->serializeData) {
    objectSerializeTextToJson(object, fpw);
  } else {
    objectSerializeWalkingChildren(object, levels, fpw);
  }
}

void objectSerializeAsComposite(LCObjectRef object, FILE *fpw) {
  objectSerializeToLevels(object, -1, fpw);
}

void objectSerialize(LCObjectRef object, FILE *fpw) {
  objectSerializeToLevels(object, 0, fpw);
}

void objectSerializeBinaryData(LCObjectRef object, FILE *fd) {
  object->type->serializeData(object, fd);
}

void objectStoreChildren(LCObjectRef object, char *key, LCObjectRef objects[], size_t length) {
  object->type->storeChildren(object, key, objects, length);
}

static void objectInitData(LCObjectRef object) {
  if (object->type->initData) {
    object->data = object->type->initData();
  }
}

void objectDeserialize(LCObjectRef object, FILE* fd) {
  if (object->type->deserializeData && object->type->serializationFormat == LCBinary) {
    objectDeserializeBinaryData(object, fd);
  } else {
    objectInitData(object);
    LCMutableDataRef data = LCMutableDataCreate(NULL, 0);
    LCMutableDataAppendFromFile(data, fd, fileLength(fd));
    LCMutableDataAppend(data, (LCByte*)"\0", 1);
    char *jsonString = (char*)LCMutableDataDataRef(data);
    json_value *json = json_parse(jsonString);
    objectDeserializeJson(object, json);
    objectRelease(data);
  }
}

void objectDeserializeBinaryData(LCObjectRef object, FILE *fd) {
  object->data = object->type->deserializeData(object, fd);
}

void objectHash(LCObjectRef object, char hashBuffer[HASH_LENGTH]) {
  if (!_objectHash(object) || !object->type->immutable) {
    void* context = createHashContext();
    FILE *fp = createMemoryWriteStream(context, updateHashContext, NULL);
    objectSerialize(object, fp);
    fclose(fp);
    finalizeHashContext(context, hashBuffer);
  } else {
    strcpy(hashBuffer, _objectHash(object));
  }
  if (object->type->immutable) {
    _objectSetHash(object, hashBuffer);
  }
}

LCStringRef objectCreateHashString(LCObjectRef object) {
  char hash[HASH_LENGTH];
  objectHash(object, hash);
  return LCStringCreate(hash);
}

static void storeChildCallback(void *cookie, char *key, LCObjectRef objects[], size_t length, bool composite) {
  LCContextRef context = (LCContextRef)cookie;
  for (LCInteger i=0; i<length; i++) {
    if (!composite) {
      objectStore(objects[i], context);
    }
  }
}

static void objectStoreWithCompositeParam(LCObjectRef object, bool composite, LCContextRef context) {
  char hash[HASH_LENGTH];
  objectHash(object, hash);
  if (!object->type->immutable) {
    _objectSetHash(object, hash);
  }
  if (storeFileExists(context->store, objectType(object), hash)) {
    return;
  }
  FILE* fp = storeWriteData(context->store, objectType(object), hash);
  if (composite) {
    objectSerializeAsComposite(object, fp);
  } else {
    objectSerialize(object, fp);
    objectWalkChildren(object, context, storeChildCallback);
  }
  fclose(fp);
}

void objectStore(LCObjectRef object, LCContextRef context) {
  objectStoreWithCompositeParam(object, false, context);
}

void objectStoreAsComposite(LCObjectRef object, LCContextRef context) {
  objectStoreWithCompositeParam(object, true, context);
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
      FILE* fp = storeReadData(context->store, objectType(object), _objectHash(object));
      objectDeserialize(object, fp);
      fclose(fp);
    }
  }
}

void objectDeleteCache(LCObjectRef object, LCContextRef context) {
  if (storeFileExists(context->store, objectType(object), _objectHash(object))) {
    object->context = context;
    objectDataDealloc(object);
  }
}

static int objectCompareFun(const void * elem1, const void * elem2) {
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

bool objectHashEqual(LCObjectRef object1, LCObjectRef object2) {
  char hash1[HASH_LENGTH];
  char hash2[HASH_LENGTH];
  objectHash(object1, hash1);
  objectHash(object2, hash2);
  return strcmp(hash1, hash2)==0;
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

bool typeBinarySerialized(LCTypeRef type) {
  if (type->serializeData || type->serializeDataBuffered) {
    return true;
  } else {
    return false;
  }
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

bool storeFileExists(LCStoreRef store, LCTypeRef type, char hash[HASH_LENGTH]) {
  FILE *fp = storeReadData(store, type, hash);
  if (fp) {
    fclose(fp);
    return true;
  } else {
    return false;
  }
}

FILE* storeWriteData(LCStoreRef store, LCTypeRef type, char hash[HASH_LENGTH]) {
  return store->writefn(store->cookie, type, hash);
}

void storeDeleteData(LCStoreRef store, LCTypeRef type, char hash[HASH_LENGTH]) {
  return store->deletefn(store->cookie, type, hash);
}

FILE* storeReadData(LCStoreRef store, LCTypeRef type, char hash[HASH_LENGTH]) {
  return store->readfn(store->cookie, type, hash);
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