
#include "LCKeyValue.h"

typedef struct keyValueData* keyValueDataRef;

void keyValueDealloc(LCObjectRef object);
LCCompare keyValueCompare(LCObjectRef object1, LCObjectRef object2);
void keyValueWalkChildren(LCObjectRef object, void *cookie, childCallback cb);
void keyValueStoreChildren(LCObjectRef object, char *key, LCObjectRef objects[], size_t length);

struct keyValueData {
  LCObjectRef key;
  LCObjectRef value;
};

struct LCType typeKeyValue = {
  .name = "LCKeyValue",
  .immutable = false,
  .dealloc = keyValueDealloc,
  .compare = keyValueCompare,
  .walkChildren = keyValueWalkChildren,
  .storeChildren = keyValueStoreChildren
};

LCTypeRef LCTypeKeyValue = &typeKeyValue;

LCKeyValueRef LCKeyValueCreate(LCObjectRef key, LCObjectRef value) {
  keyValueDataRef newKeyValue = malloc(sizeof(struct keyValueData));
  if (newKeyValue) {
    objectRetain(key);
    objectRetain(value);
    newKeyValue->key=key;
    newKeyValue->value=value;
    return objectCreate(LCTypeKeyValue, newKeyValue);
  } else {
    return NULL;
  }
};

void* LCKeyValueKey(LCKeyValueRef keyValue) {
  keyValueDataRef keyValueData = objectData(keyValue);
  return keyValueData->key;
}

void* LCKeyValueValue(LCKeyValueRef keyValue) {
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
  cb(cookie, "key", &key, 1, 0);
  cb(cookie, "value", &value, 1, 0);
}

void keyValueStoreChildren(LCObjectRef object, char *key, LCObjectRef objects[], size_t length) {
  keyValueDataRef data = objectData(object);
  if (strcmp(key, "key")) {
    data->key = objectRetain(*objects);
    return;
  } else
  if (strcmp(key, "value")) {
    data->value = objectRetain(*objects);
  }
}
