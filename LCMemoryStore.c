
#include "LCMemoryStore.h"
#include "LCMutableDictionary.h"
#include "LCMemoryStreamLarge.h"
#include "LCString.h"

FILE* memoryStoreWrite(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);
void memoryStoreDelete(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);
FILE* memoryStoreRead(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);
void memoryStoreDealloc(LCObjectRef store);

struct LCType typeMemoryStore = {
  .dealloc = memoryStoreDealloc
};

LCTypeRef LCTypeMemoryStore = &typeMemoryStore;

LCMemoryStoreRef LCMemoryStoreCreate() {
  return objectCreate(LCTypeMemoryStore, LCMutableDictionaryCreate(NULL, 0));
}

static LCMutableDictionaryRef memoryStoreData(LCMemoryStoreRef store) {
  return objectData(store);
}

LCStoreRef LCMemoryStoreStoreObject(LCMemoryStoreRef store) {
  return storeCreate(store, memoryStoreWrite, memoryStoreDelete, memoryStoreRead);
}

FILE* memoryStoreWrite(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]) {
  LCMemoryStreamLargeRef stream = LCMemoryStreamLargeCreate();
  LCStringRef hashObj = LCStringCreate(hash);
  LCMutableDictionarySetValueForKey(memoryStoreData(cookie), hashObj, stream);
  objectRelease(stream);
  objectRelease(hashObj);
  return LCMemoryStreamLargeWriteFile(stream);
}

void memoryStoreDelete(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]) {
  LCStringRef hashObj = LCStringCreate(hash);
  LCMutableDictionaryDeleteKey(memoryStoreData(cookie), hashObj);
  objectRelease(hashObj);
}

FILE* memoryStoreRead(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]) {
  LCStringRef hashObj = LCStringCreate(hash);
  LCMemoryStreamLargeRef stream = LCMutableDictionaryValueForKey(memoryStoreData(cookie), hashObj);
  objectRelease(hashObj);
  if (stream) {
    return LCMemoryStreamLargeReadFile(stream);
  } else {
    return NULL;
  }
}

void memoryStoreDealloc(LCObjectRef store) {
  objectRelease(objectData(store));
}