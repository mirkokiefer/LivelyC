

#include "LCCore.h"
#include "LCUtils.h"
#include "LCMemoryStream.h"
#include "LCSHA.h"

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
};

LCObjectRef objectCreate(LCTypeRef type, void* data) {
  LCObjectRef object = malloc(sizeof(struct LCObject));
  if (object) {
    object->rCount = 1;
    object->type = type;
    object->data = data;
    object->persisted = false;
  }
  return object;
}

LCObjectRef objectCreateFromContext(LCContextRef context, LCTypeRef type, char hash[HASH_LENGTH]) {
  LCObjectRef object = objectCreate(type, NULL);
  object->persisted = true;
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

static void objectSerializeWithCallback(LCObjectRef object, void* cookie, callback flushFunct, FILE* fp) {
  object->type->serialize(object, cookie, flushFunct, fp);
}

void objectSerialize(LCObjectRef object, FILE* fp) {
  objectSerializeWithCallback(object, NULL, NULL, fp);
}

void objectDeserialize(LCObjectRef object, FILE* fd) {
  void* data = object->type->deserialize(object, fd);
  object->data = data;
}

char* objectHash(LCObjectRef object) {
  if ((object->hash[0] == '\0') || !object->type->immutable) {
    LCMemoryStreamRef memoryStream = LCMemoryStreamCreate();
    void* context = createHashContext(memoryStream);
    objectSerializeWithCallback(object, context, updateHashContext, LCMemoryStreamWriteFile(memoryStream));
    finalizeHashContext(context, object->hash);
    objectRelease(memoryStream);
  }
  return object->hash;
}

void objectStore(LCObjectRef object, LCContextRef context) {
  FILE* fp = context->store->writefn(context->store->cookie, objectType(object), objectHash(object));
  objectSerialize(object, fp);
  object->persisted = true;
  object->context = context;
  fclose(fp);
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

LCContextRef contextCreate(LCStoreRef store) {
  LCContextRef context = malloc(sizeof(struct LCContext));
  if (context) {
    context->store = store;
  }
  return context;
}

void lcFree(void* object) {
  free(object);
}