
#include "LCMemoryStore.h"
#include "LCMutableDictionary.h"
#include "LCMemoryStream.h"
#include "LCString.h"
#include "LCMutableData.h"

FILE* memoryStoreWrite(void *cookie, LCTypeRef type, char *key);
void memoryStoreDelete(void *cookie, LCTypeRef type, char *key);
FILE* memoryStoreRead(void *cookie, LCTypeRef type, char *key);
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

FILE* memoryStoreWrite(void *cookie, LCTypeRef type, char *key) {
  LCMutableDataRef data = LCMutableDataCreate(NULL, 0);
  LCStringRef hashObj = LCStringCreate(key);
  LCMutableDictionarySetValueForKey(memoryStoreData(cookie), hashObj, data);
  objectRelease(data);
  objectRelease(hashObj);
  return createMemoryWriteStream(data, LCMutableDataAppendAlt, NULL);
}

void memoryStoreDelete(void *cookie, LCTypeRef type, char *key) {
  LCStringRef hashObj = LCStringCreate(key);
  LCMutableDictionaryDeleteKey(memoryStoreData(cookie), hashObj);
  objectRelease(hashObj);
}

FILE* memoryStoreRead(void *cookie, LCTypeRef type, char *key) {
  LCStringRef hashObj = LCStringCreate(key);
  LCMutableDataRef data = LCMutableDictionaryValueForKey(memoryStoreData(cookie), hashObj);
  objectRelease(hashObj);
  if (data) {
    return createMemoryReadStream(NULL, LCMutableDataDataRef(data), LCMutableDataLength(data), false, NULL);
  } else {
    return NULL;
  }
}

void memoryStoreDealloc(LCObjectRef store) {
  objectRelease(objectData(store));
}