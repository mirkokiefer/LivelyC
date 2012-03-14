
#include "LCMutableArray.h"

typedef struct mutableArrayData* mutableArrayDataRef;

void mutableArrayDealloc(LCMutableArrayRef object);
bool resizeBuffer(mutableArrayDataRef array, size_t size);
void mutableArraySerialize(LCObjectRef object, void* cookie, callback flush, FILE* fd);

struct mutableArrayData {
  size_t length;
  size_t bufferLength;
  LCObjectRef* objects;
};

struct LCType typeMutableArray = {
  .immutable = false,
  .dealloc = mutableArrayDealloc,
  .serialize = mutableArraySerialize
};

LCTypeRef LCTypeMutableArray = &typeMutableArray;

LCMutableArrayRef LCMutableArrayCreate(LCObjectRef objects[], size_t length) {
  mutableArrayDataRef newArray = malloc(sizeof(struct mutableArrayData));
  if (newArray) {
    newArray->objects = NULL;
    if(length > 0) {
      for(LCInteger i=0; i<length; i++) {
        objectRetain(objects[i]);
      }
      resizeBuffer(newArray, length);
      memcpy(newArray->objects, objects, length * sizeof(void*));  
    } else {
      resizeBuffer(newArray, 10);
    }
    newArray->length = length;
    return objectCreate(LCTypeMutableArray, newArray);
  } else {
    return NULL;
  }
};

LCMutableArrayRef LCMutableArrayCreateFromArray(LCArrayRef array) {
  return LCMutableArrayCreate(LCArrayObjects(array), LCArrayLength(array));
}

LCObjectRef* LCMutableArrayObjects(LCMutableArrayRef array) {
  mutableArrayDataRef arrayData = objectData(array);
  return arrayData->objects;
}

LCObjectRef LCMutableArrayObjectAtIndex(LCMutableArrayRef array, LCInteger index) {
  return LCMutableArrayObjects(array)[index];
}

size_t LCMutableArrayLength(LCMutableArrayRef array) {
  mutableArrayDataRef arrayData = objectData(array);
  return arrayData->length;
}

LCArrayRef LCMutableArrayCreateSubArray(LCMutableArrayRef array, LCInteger start, size_t length) {
  if (length == -1) {
    length = LCMutableArrayLength(array)-start;
  }
  return LCArrayCreate(&(LCMutableArrayObjects(array)[start]), length);
}

LCArrayRef LCMutableArrayCreateArray(LCMutableArrayRef array) {
  return LCArrayCreate(LCMutableArrayObjects(array), LCMutableArrayLength(array));
}

LCMutableArrayRef LCMutableArrayCopy(LCMutableArrayRef array) {
  return LCMutableArrayCreate(LCMutableArrayObjects(array), LCMutableArrayLength(array));
}

void LCMutableArrayAddObject(LCMutableArrayRef array, LCObjectRef object) {
  mutableArrayDataRef arrayData = objectData(array);
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
  mutableArrayDataRef arrayData = objectData(array);
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

void mutableArrayDealloc(LCMutableArrayRef object) {
  mutableArrayDataRef arrayData = objectData(object);
  for (LCInteger i=0; i<LCMutableArrayLength(object); i++) {
    objectRelease(LCMutableArrayObjectAtIndex(object, i));
  }
  lcFree(arrayData->objects);
}

bool resizeBuffer(mutableArrayDataRef array, size_t length) {
  void* buffer = realloc(array->objects, sizeof(void*) * length);
  if(buffer) {
    array->objects = buffer;
    array->bufferLength = length;
    return true;
  } else {
    return false;
  }
}

void mutableArraySerialize(LCObjectRef array, void* cookie, callback flush, FILE* fd) {
  fprintf(fd, "{");
  for (LCInteger i=0; i<LCMutableArrayLength(array); i++) {
    objectSerialize(LCMutableArrayObjectAtIndex(array, i), fd);
    if (i<LCMutableArrayLength(array)-1) {
      fprintf(fd, ", ");
    }
  }
  fprintf(fd, "}");
}
