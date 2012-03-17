
#include "LCMutableDictionary.h"

typedef struct mutableDictData* mutableDictDataRef;

void mutableDictionaryDealloc(LCObjectRef object);
void mutableDictionaryWalkChildren(LCObjectRef object, void *cookie, childCallback cb);
void mutableDictionaryStoreChildren(LCObjectRef object, char *key, LCObjectRef objects[], size_t length);

struct mutableDictData {
  LCMutableArrayRef keyValues;
};

struct LCType typeMutableDictionary = {
  .immutable = false,
  .dealloc = mutableDictionaryDealloc,
  .walkChildren = mutableDictionaryWalkChildren,
  .storeChildren = mutableDictionaryStoreChildren
};

LCTypeRef LCTypeMutableDictionary = &typeMutableDictionary;

LCMutableDictionaryRef LCMutableDictionaryCreate(LCKeyValueRef keyValues[], size_t length) {
  mutableDictDataRef newDict = malloc(sizeof(struct mutableDictData));
  if (newDict) {
    newDict->keyValues = LCMutableArrayCreate(keyValues, length);
    return objectCreate(LCTypeMutableDictionary, newDict);
  } else {
    return NULL;
  }
};

LCKeyValueRef LCMutableDictionaryEntryForKey(LCMutableDictionaryRef dict, LCObjectRef key) {
  LCKeyValueRef* keyValues = LCMutableDictionaryEntries(dict);
  for (LCInteger i=0; i<LCMutableDictionaryLength(dict); i++) {
    if(objectCompare(key, LCKeyValueKey(keyValues[i])) == LCEqual) {
      return keyValues[i];
    }
  }
  return NULL;
}

LCObjectRef LCMutableDictionaryValueForKey(LCMutableDictionaryRef dict, LCObjectRef key) {
  LCKeyValueRef entry = LCMutableDictionaryEntryForKey(dict, key);
  if (entry) {
    return LCKeyValueValue(entry);
  } else {
    return NULL;
  }
}

void LCMutableDictionaryDeleteKey(LCMutableDictionaryRef dict, LCObjectRef key) {
  LCKeyValueRef* keyValues = LCMutableDictionaryEntries(dict);
  for (LCInteger i=0; i<LCMutableDictionaryLength(dict); i++) {
    if(objectCompare(key, LCKeyValueKey(keyValues[i])) == LCEqual) {
      mutableDictDataRef dictData = objectData(dict);
      LCMutableArrayRemoveIndex(dictData->keyValues, i);
    }
  }
}

void LCMutableDictionarySetValueForKey(LCMutableDictionaryRef dict, LCObjectRef key, LCObjectRef value) {
  LCMutableDictionaryDeleteKey(dict, key);
  LCKeyValueRef keyValue = LCKeyValueCreate(key, value);
  mutableDictDataRef dictData = objectData(dict);
  LCMutableArrayAddObject(dictData->keyValues, keyValue);
}

void LCMutableDictionaryAddEntry(LCMutableDictionaryRef dict, LCKeyValueRef keyValue) {
  LCMutableDictionaryDeleteKey(dict, LCKeyValueKey(keyValue));
  if (LCKeyValueValue(keyValue) != NULL) {
    mutableDictDataRef dictData = objectData(dict);
    LCMutableArrayAddObject(dictData->keyValues, keyValue);
  }
}

void LCMutableDictionaryAddEntries(LCMutableDictionaryRef dict, LCKeyValueRef keyValues[], size_t length) {
  for (LCInteger i=0; i<length; i++) {
    LCMutableDictionaryAddEntry(dict, keyValues[i]);
  }
}

LCMutableDictionaryRef LCMutableDictionaryCopy(LCMutableDictionaryRef dict) {
  return LCMutableDictionaryCreate(LCMutableDictionaryEntries(dict), LCMutableDictionaryLength(dict));
}

size_t LCMutableDictionaryLength(LCMutableDictionaryRef dict) {
  mutableDictDataRef dictData = objectData(dict);
  return LCMutableArrayLength(dictData->keyValues);
}

LCKeyValueRef* LCMutableDictionaryEntries(LCMutableDictionaryRef dict) {
  mutableDictDataRef dictData = objectData(dict);
  return LCMutableArrayObjects(dictData->keyValues);
}

LCMutableArrayRef LCMutableDictionaryCreateChangesArray(LCMutableDictionaryRef original, LCMutableDictionaryRef new) {
  LCMutableDictionaryRef changes = LCMutableDictionaryCreate(NULL, 0);
  LCKeyValueRef* originalKeyValues = LCMutableDictionaryEntries(original);
  LCKeyValueRef* newKeyValues = LCMutableDictionaryEntries(new);

  for (LCInteger i=0; i<LCMutableDictionaryLength(original); i++) {
    LCObjectRef key = LCKeyValueKey(originalKeyValues[i]);
    LCObjectRef value = LCKeyValueValue(originalKeyValues[i]);
    LCObjectRef newValue = LCMutableDictionaryValueForKey(new, key);
    if (objectCompare(value, newValue) != LCEqual) {
      LCMutableDictionarySetValueForKey(changes, key, newValue);
    }
  }
  for (LCInteger i=0; i<LCMutableDictionaryLength(new); i++) {
    LCObjectRef key = LCKeyValueKey(newKeyValues[i]);
    LCObjectRef newValue = LCKeyValueValue(newKeyValues[i]);
    LCObjectRef originalValue = LCMutableDictionaryValueForKey(original, key);
    if (objectCompare(originalValue, newValue) != LCEqual) {
      LCMutableDictionarySetValueForKey(changes, key, newValue);
    }
  }
  mutableDictDataRef changesData = objectData(changes);
  LCMutableArrayRef result = objectRetain(changesData->keyValues);
  objectRelease(changes);
  return result;
}

LCMutableArrayRef LCMutableDictionaryCreateAddedArray(LCMutableDictionaryRef original, LCMutableDictionaryRef new) {
  LCMutableArrayRef changes = LCMutableArrayCreate(NULL, 0);
  LCKeyValueRef* newKeyValues = LCMutableDictionaryEntries(new);
  for (LCInteger i=0; i<LCMutableDictionaryLength(new); i++) {
    void* key = LCKeyValueKey(newKeyValues[i]);
    void* originalValue = LCMutableDictionaryValueForKey(original, key);
    if (originalValue == NULL) {
      LCMutableArrayAddObject(changes, newKeyValues[i]);
    }
  }
  return changes;
}

LCMutableArrayRef LCMutableDictionaryCreateUpdatedArray(LCMutableDictionaryRef original, LCMutableDictionaryRef new) {
  LCMutableArrayRef changes = LCMutableArrayCreate(NULL, 0);
  LCKeyValueRef* originalKeyValues = LCMutableDictionaryEntries(original);
  for (LCInteger i=0; i<LCMutableDictionaryLength(original); i++) {
    void* key = LCKeyValueKey(originalKeyValues[i]);
    void* value = LCKeyValueValue(originalKeyValues[i]);
    LCKeyValueRef newEntry = LCMutableDictionaryEntryForKey(new, key);
    void* newValue = LCKeyValueValue(newEntry);
    if ((objectCompare(value, newValue) != LCEqual) && (newValue != NULL)) {
      LCMutableArrayAddObject(changes, newEntry);
    }
  }
  return changes;
}

LCMutableArrayRef LCMutableDictionaryCreateDeletedArray(LCMutableDictionaryRef original, LCMutableDictionaryRef new) {
  LCMutableArrayRef changes = LCMutableArrayCreate(NULL, 0);
  LCKeyValueRef* originalKeyValues = LCMutableDictionaryEntries(original);
  for (LCInteger i=0; i<LCMutableDictionaryLength(original); i++) {
    void* key = LCKeyValueKey(originalKeyValues[i]);
    void* newValue = LCMutableDictionaryValueForKey(new, key);
    if (newValue == NULL) {
      LCMutableArrayAddObject(changes, key);
    }
  }
  return changes;
}


void mutableDictionaryDealloc(LCObjectRef object) {
  mutableDictDataRef dictData = objectData(object);
  objectRelease(dictData->keyValues);
  lcFree(dictData);
}

void mutableDictionaryWalkChildren(LCObjectRef object, void *cookie, childCallback cb) {
  cb(cookie, "entries", LCMutableDictionaryEntries(object), LCMutableDictionaryLength(object), 0);
}

void mutableDictionaryStoreChildren(LCObjectRef object, char *key, LCObjectRef objects[], size_t length) {
  if (strcmp(key, "entries")==0) {
    mutableDictDataRef data = objectData(object);
    data->keyValues = LCMutableArrayCreate(objects, length);
  }
}
