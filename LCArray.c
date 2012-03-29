
#include "LCArray.h"

typedef struct arrayData* arrayDataRef;

LCCompare arrayCompare(LCObjectRef object1, LCObjectRef object2);
void arrayDealloc(LCObjectRef object);
void arrayWalkChildren(LCObjectRef object, void *cookie, childCallback cb);
void arrayStoreChildren(LCObjectRef object, char *key, LCObjectRef objects[], size_t length);
static void* arrayInitData();

bool resizeBuffer(arrayDataRef array, size_t size);
void mutableArraySerialize(LCObjectRef object, void* cookie, callback flush, FILE* fd);

struct arrayData {
  size_t length;
  size_t bufferLength;
  LCObjectRef* objects;
};

struct LCType typeArray = {
  .name = "LCArray",
  .serializationFormat = LCText,
  .immutable = true,
  .dealloc = arrayDealloc,
  .compare = arrayCompare,
  .initData = arrayInitData,
  .walkChildren = arrayWalkChildren,
  .storeChildren = arrayStoreChildren
};

struct LCType typeMutableArray = {
  .name = "LCMutableArray",
  .serializationFormat = LCText,
  .immutable = false,
  .dealloc = arrayDealloc,
  .compare = arrayCompare,
  .initData = arrayInitData,
  .walkChildren = arrayWalkChildren,
  .storeChildren = arrayStoreChildren
};

LCTypeRef LCTypeArray = &typeArray;
LCTypeRef LCTypeMutableArray = &typeMutableArray;

static void* arrayInitData() {
  arrayDataRef newArray = malloc(sizeof(struct arrayData));
  if (newArray) {
    newArray->length = 0;
    newArray->objects = NULL;
  }
  return newArray;
}

static void arraySetObjects(arrayDataRef data, LCObjectRef objects[], size_t length) {
  for(LCInteger i=0; i<length; i++) {
    objectRetain(objects[i]);
  }
  resizeBuffer(data, length);
  memcpy(data->objects, objects, length * sizeof(LCObjectRef));
  data->length = length;
}

LCArrayRef LCArrayCreate(LCObjectRef objects[], size_t length) {
  if (!objectsImmutable(objects, length)) {
    perror(ErrorObjectImmutable);
    return NULL;
  }
  arrayDataRef newArray = arrayInitData();
  newArray->objects = NULL;
  arraySetObjects(newArray, objects, length);
  return objectCreate(LCTypeArray, newArray);
};

LCArrayRef LCArrayCreateAppendingObject(LCArrayRef array, LCObjectRef object) {
  return LCArrayCreateAppendingObjects(array, &object, 1);
}

LCArrayRef LCArrayCreateAppendingObjects(LCArrayRef object, LCObjectRef objects[], size_t length) {
  if (!objectsImmutable(objects, length)) {
    perror(ErrorObjectImmutable);
    return NULL;
  }
  arrayDataRef array = objectData(object);
  size_t totalLength = array->length + length;
  LCObjectRef* newObjects = malloc(totalLength * sizeof(LCObjectRef));
  if(newObjects) {
    memcpy(newObjects, array->objects, array->length * sizeof(LCObjectRef));
    memcpy(&(newObjects[array->length]), objects, length * sizeof(LCObjectRef));
    arrayDataRef data = arrayInitData();
    data->objects = newObjects;
    data->length = totalLength;
    for (LCInteger i=0; i<totalLength; i++) {
      objectRetain(newObjects[i]);
    }
    return objectCreate(LCTypeArray, data);
  } else {
    return NULL;
  }
}

LCArrayRef LCArrayCreateFromArrays(LCArrayRef arrays[], size_t length) {
  size_t totalLength = 0;
  for (LCInteger i=0; i<length; i++) {
    totalLength = totalLength + LCArrayLength(arrays[i]);
  }
  
  LCObjectRef* newObjects = malloc(totalLength * sizeof(LCObjectRef));
  if (newObjects) {
    arrayDataRef newArray = arrayInitData();
    newArray->objects = newObjects;
    newArray->length = totalLength;
    size_t copyPos = 0;
    for (LCInteger i=0; i<length; i++) {
      size_t copyLength = LCArrayLength(arrays[i]);
      memcpy(&(newArray->objects[copyPos]), LCArrayObjects(arrays[i]), copyLength * sizeof(void*));
      copyPos = copyPos+copyLength;
    }
    for (LCInteger i=0; i<totalLength; i++) {
      objectRetain(newArray->objects[i]);
    }
    return objectCreate(LCTypeArray, newArray);
  } else {
    return NULL;
  }
}

LCObjectRef* LCArrayObjects(LCArrayRef object) {
  arrayDataRef array = objectData(object);
  return array->objects;
}

LCObjectRef LCArrayObjectAtIndex(LCArrayRef object, LCInteger index) {
  arrayDataRef array = objectData(object);
  return array->objects[index];
}

size_t LCArrayLength(LCArrayRef object) {
  arrayDataRef array = objectData(object);
  return array->length;
}

LCArrayRef LCArrayCreateSubArray(LCArrayRef object, LCInteger start, size_t length) {
  size_t arrayLength = LCArrayLength(object);
  LCObjectRef* arrayObjects = LCArrayObjects(object);
  if (start >= arrayLength) {
    return LCArrayCreate(NULL, 0);
  }
  if (length == -1) {
    length = arrayLength - start;
  }
  return LCArrayCreate(&(arrayObjects[start]), length);
}

LCArrayRef LCArrayCreateArrayWithMap(LCArrayRef array, void* info, LCCreateEachCb each) {
  size_t arrayLength = LCArrayLength(array);
  LCArrayRef* arrayObjects = LCArrayObjects(array);
  LCArrayRef newObjects[arrayLength];
  for (LCInteger i=0; i<arrayLength; i++) {
    newObjects[i] = each(i, info, arrayObjects[i]);
  }
  LCArrayRef newArray = LCArrayCreate(newObjects, arrayLength);
  for (LCInteger i=0; i<arrayLength; i++) {
    objectRelease(newObjects[i]);
  }
  return newArray;
}

LCCompare arrayCompare(LCObjectRef array1, LCObjectRef array2) {
  return objectCompare(LCArrayObjectAtIndex(array1, 0), LCArrayObjectAtIndex(array2, 0));
}

void arrayDealloc(LCObjectRef object) {
  for (LCInteger i=0; i<LCArrayLength(object); i++) {
    objectRelease(LCArrayObjectAtIndex(object, i));
  }
  lcFree(objectData(object));
}

void arrayWalkChildren(LCObjectRef object, void *cookie, childCallback cb) {
  cb(cookie, "objects", LCArrayObjects(object), LCArrayLength(object), false);
}

void arrayStoreChildren(LCObjectRef object, char *key, LCObjectRef objects[], size_t length) {
  if (strcmp(key, "objects")==0) {
    arraySetObjects(objectData(object), objects, length);
  }
}

// LCMutableArray

LCMutableArrayRef LCMutableArrayCreate(LCObjectRef objects[], size_t length) {
  arrayDataRef newArray = arrayInitData();
  newArray->objects = NULL;
  if(length > 0) {
    arraySetObjects(newArray, objects, length);
  } else {
    resizeBuffer(newArray, 10);
  }
  return objectCreate(LCTypeMutableArray, newArray);
};

inline LCObjectRef* LCMutableArrayObjects(LCMutableArrayRef array) {
  return LCArrayObjects(array);
}

inline LCObjectRef LCMutableArrayObjectAtIndex(LCMutableArrayRef array, LCInteger index) {
  return LCArrayObjectAtIndex(array, index);
}

inline size_t LCMutableArrayLength(LCMutableArrayRef array) {
  return LCArrayLength(array);
}

inline LCMutableArrayRef LCMutableArrayCreateSubArray(LCMutableArrayRef array, LCInteger start, size_t length) {
  return LCArrayCreateSubArray(array, start, length);
}

LCMutableArrayRef LCMutableArrayCreateFromArray(LCArrayRef array) {
  return LCMutableArrayCreate(LCArrayObjects(array), LCArrayLength(array));
}

LCArrayRef LCMutableArrayCreateArray(LCMutableArrayRef array) {
  return LCArrayCreate(LCMutableArrayObjects(array), LCMutableArrayLength(array));
}

LCMutableArrayRef LCMutableArrayCopy(LCMutableArrayRef array) {
  return LCMutableArrayCreate(LCMutableArrayObjects(array), LCMutableArrayLength(array));
}

void LCMutableArrayAddObject(LCMutableArrayRef array, LCObjectRef object) {
  arrayDataRef arrayData = objectData(array);
  size_t arrayLength = LCMutableArrayLength(array);
  if(arrayLength+1 > arrayData->bufferLength) {
    resizeBuffer(arrayData, arrayData->bufferLength*2);
  }
  objectRetain(object);
  LCMutableArrayObjects(array)[arrayLength] = object;
  arrayData->length = arrayLength + 1;
}

void LCMutableArrayAddObjects(LCMutableArrayRef array, LCObjectRef objects[], size_t length) {
  for (LCInteger i=0; i<length; i++) {
    LCMutableArrayAddObject(array, objects[i]);
  }
}

void LCMutableArrayRemoveIndex(LCMutableArrayRef array, LCInteger index) {
  LCObjectRef* arrayObjects = LCMutableArrayObjects(array);
  size_t arrayLength = LCMutableArrayLength(array);
  arrayDataRef arrayData = objectData(array);
  objectRelease(arrayObjects[index]);
  if (index < (arrayLength-1)) {
    size_t objectsToCopy = arrayLength - (index+1);
    memmove(&(arrayObjects[index]), &(arrayObjects[index+1]), objectsToCopy*sizeof(LCObjectRef));
  }
  arrayData->length = arrayLength-1;
}

void LCMutableArrayRemoveObject(LCMutableArrayRef array, LCObjectRef object) {
  for (LCInteger i=0; i<LCMutableArrayLength(object); i++) {
    if(LCMutableArrayObjectAtIndex(array, i) == object) {
      return LCMutableArrayRemoveIndex(array, i);
    }
  }
}

void LCMutableArraySort(LCMutableArrayRef array) {
  objectsSort(LCMutableArrayObjects(array), LCMutableArrayLength(array));
}

LCMutableArrayRef LCArrayCreateMutableArrayWithMap(LCArrayRef array, void* info, LCCreateEachCb each) {
  size_t arrayLength = LCArrayLength(array);
  LCArrayRef* arrayObjects = LCArrayObjects(array);
  LCArrayRef newObjects[arrayLength];
  for (LCInteger i=0; i<arrayLength; i++) {
    newObjects[i] = each(i, info, arrayObjects[i]);
  }
  LCArrayRef newArray = LCMutableArrayCreate(newObjects, arrayLength);
  for (LCInteger i=0; i<arrayLength; i++) {
    objectRelease(newObjects[i]);
  }
  return newArray;
}

bool resizeBuffer(arrayDataRef array, size_t length) {
  void* buffer = realloc(array->objects, sizeof(void*) * length);
  if(buffer) {
    array->objects = buffer;
    array->bufferLength = length;
    return true;
  } else {
    return false;
  }
}
