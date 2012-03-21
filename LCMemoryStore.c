
#include "LCMemoryStore.h"
#include "LCMutableDictionary.h"
#include "LCMemoryStream.h"
#include "LCString.h"
#include "LCMutableData.h"

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
  LCMutableDataRef data = LCMutableDataCreate(NULL, 0);
  LCStringRef hashObj = LCStringCreate(hash);
  LCMutableDictionarySetValueForKey(memoryStoreData(cookie), hashObj, data);
  objectRelease(data);
  objectRelease(hashObj);
  return createMemoryWriteStream(data, LCMutableDataAppendAlt);
}

void memoryStoreDelete(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]) {
  LCStringRef hashObj = LCStringCreate(hash);
  LCMutableDictionaryDeleteKey(memoryStoreData(cookie), hashObj);
  objectRelease(hashObj);
}

FILE* memoryStoreRead(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]) {
  LCStringRef hashObj = LCStringCreate(hash);
  LCMutableDataRef data = LCMutableDictionaryValueForKey(memoryStoreData(cookie), hashObj);
  objectRelease(hashObj);
  if (data) {
    return createMemoryReadStream(LCMutableDataDataRef(data), LCMutableDataLength(data));
  } else {
    return NULL;
  }
}

void memoryStoreDealloc(LCObjectRef store) {
  objectRelease(objectData(store));
}