
#include "LCArray.h"

LCCompare arrayCompare(LCObjectRef object1, LCObjectRef object2);
void arrayDealloc(LCObjectRef object);
void arraySerialize(LCObjectRef object, void* cookie, callback flush, FILE* fd);

struct arrayData {
  size_t length;
  LCObjectRef objects[];
};

typedef struct arrayData* arrayDataRef;

struct LCType typeArray = {
  .immutable = true,
  .dealloc = arrayDealloc,
  .compare = arrayCompare,
  .serialize = arraySerialize
};

LCTypeRef LCTypeArray = &typeArray;

LCArrayRef LCArrayCreate(LCObjectRef objects[], size_t length) {
  if (!objectsImmutable(objects, length)) {
    perror(ErrorObjectImmutable);
    return NULL;
  }
  arrayDataRef newArray = malloc(sizeof(struct arrayData) + length * sizeof(LCObjectRef));
  if (newArray) {
    for(LCInteger i=0; i<length; i++) {
      objectRetain(objects[i]);
    }
    newArray->length = length;
    memcpy(newArray->objects, objects, length * sizeof(void*));
    
    return objectCreate(LCTypeArray, newArray);
  } else {
    return NULL;
  }
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
  arrayDataRef newArray = malloc(sizeof(struct arrayData) + totalLength * sizeof(LCObjectRef));
  if(newArray) {
    newArray->length = totalLength;
    memcpy(newArray->objects, array->objects, array->length * sizeof(LCObjectRef));
    memcpy(&(newArray->objects[array->length]), objects, length * sizeof(LCObjectRef));
    for (LCInteger i=0; i<totalLength; i++) {
      objectRetain(newArray->objects[i]);
    }
    return objectCreate(LCTypeArray, newArray);
  } else {
    return NULL;
  }
}

LCArrayRef LCArrayCreateFromArrays(LCArrayRef arrays[], size_t length) {
  size_t totalLength = 0;
  for (LCInteger i=0; i<length; i++) {
    totalLength = totalLength + LCArrayLength(arrays[i]);
  }
  
  arrayDataRef newArray = malloc(sizeof(struct arrayData) + totalLength * sizeof(LCObjectRef));
  if (newArray) {
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

void arraySerialize(LCObjectRef object, void* cookie, callback flush, FILE* fd) {
  fprintf(fd, "{");
  for (LCInteger i=0; i<LCArrayLength(object); i++) {
    objectSerialize(LCArrayObjectAtIndex(object, i), fd);
    if (i< LCArrayLength(object) -1) {
      fprintf(fd, ", ");
    }
  }
  fprintf(fd, "}");
}
