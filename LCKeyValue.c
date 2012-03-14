
#include "LCKeyValue.h"

typedef struct keyValueData* keyValueDataRef;

void keyValueDealloc(LCObjectRef object);
LCCompare keyValueCompare(LCObjectRef object1, LCObjectRef object2);
void keyValueSerialize(LCObjectRef object, void* cookie, callback flush, FILE* fd);

struct keyValueData {
  LCObjectRef key;
  LCObjectRef value;
};

struct LCType typeKeyValue = {
  .dealloc = keyValueDealloc,
  .compare = keyValueCompare,
  .serialize = keyValueSerialize
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
}

void keyValueSerialize(LCObjectRef keyValue, void* cookie, callback flush, FILE* fd) {
  objectSerialize(LCKeyValueKey(keyValue), fd);
  fprintf(fd, ": ");
  objectSerialize(LCKeyValueValue(keyValue), fd);
}