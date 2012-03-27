
#include "LCKeyValue.h"

typedef struct keyValueData* keyValueDataRef;

void keyValueDealloc(LCObjectRef object);
LCCompare keyValueCompare(LCObjectRef object1, LCObjectRef object2);
void keyValueWalkChildren(LCObjectRef object, void *cookie, childCallback cb);
void keyValueStoreChildren(LCObjectRef object, char *key, LCObjectRef objects[], size_t length);
static void* keyValueInitData();

struct keyValueData {
  LCObjectRef key;
  LCObjectRef value;
};

struct LCType typeKeyValue = {
  .name = "LCKeyValue",
  .immutable = false,
  .dealloc = keyValueDealloc,
  .compare = keyValueCompare,
  .initData = keyValueInitData,
  .walkChildren = keyValueWalkChildren,
  .storeChildren = keyValueStoreChildren
};

LCTypeRef LCTypeKeyValue = &typeKeyValue;

LCKeyValueRef LCKeyValueCreate(LCObjectRef key, LCObjectRef value) {
  keyValueDataRef newKeyValue = keyValueInitData();
  newKeyValue->key=objectRetain(key);
  newKeyValue->value=objectRetain(value);
  return objectCreate(LCTypeKeyValue, newKeyValue);
};

static void* keyValueInitData() {
  keyValueDataRef newKeyValue = malloc(sizeof(struct keyValueData));
  if (newKeyValue) {
    newKeyValue->key = NULL;
    newKeyValue->value = NULL;
  }
  return newKeyValue;
}

LCObjectRef LCKeyValueKey(LCKeyValueRef keyValue) {
  keyValueDataRef keyValueData = objectData(keyValue);
  return keyValueData->key;
}

LCObjectRef LCKeyValueValue(LCKeyValueRef keyValue) {
  keyValueDataRef keyValueData = objectData(keyValue);
  return keyValueData->value;
}

LCCompare keyValueCompare(LCObjectRef object1, LCObjectRef object2) {
  return objectCompare(LCKeyValueKey(object1), LCKeyValueKey(object2));
}

void keyValueDealloc(LCObjectRef object) {
  keyValueDataRef keyValueData = objectData(object);
  objectRelease(keyValueData->key);
  objectRelease(keyValueData->value);
  lcFree(objectData(object));
}

void keyValueWalkChildren(LCObjectRef object, void *cookie, childCallback cb) {
  LCObjectRef key = LCKeyValueKey(object);
  LCObjectRef value = LCKeyValueValue(object);
  cb(cookie, "key", &key, 1, false);
  cb(cookie, "value", &value, 1, false);
}

void keyValueStoreChildren(LCObjectRef object, char *key, LCObjectRef objects[], size_t length) {
  keyValueDataRef data = objectData(object);
  if (strcmp(key, "key")==0) {
    data->key = objectRetain(*objects);
    return;
  } else
  if (strcmp(key, "value")==0) {
    data->value = objectRetain(*objects);
  }
}
