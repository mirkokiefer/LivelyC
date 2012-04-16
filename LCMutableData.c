
#include "LCMutableData.h"

#include "LCUtils.h"

typedef struct mutableData* mutableDataRef;

void mutableDataDealloc(LCObjectRef data);

struct mutableData {
  size_t length;
  size_t bufferLength;
  LCByte* data;
};

struct LCType typeMutableData = {
  .name = "LCMutableData",
  .immutable = false,
  .serializationFormat = LCBinary,
  .dealloc = mutableDataDealloc,
};

LCTypeRef LCTypeMutableData = &typeMutableData;

static void mutableDataEnsureLength(mutableDataRef dataStruct, size_t length) {
  if (dataStruct->bufferLength < length) {
    size_t newLength = dataStruct->bufferLength * 2 + length;
    LCByte* newBuffer = realloc(dataStruct->data, sizeof(LCByte) * newLength);
    dataStruct->data = newBuffer;
    dataStruct->bufferLength = newLength;
  }
}

static size_t mutableDataBufferLeft(mutableDataRef dataStruct) {
  return dataStruct->bufferLength - dataStruct->length;
}

static mutableDataRef dataCreateStruct() {
  mutableDataRef newData = malloc(sizeof(struct mutableData));
  if (newData) {
    newData->length = 0;
    newData->bufferLength = 0;
    newData->data = NULL;
  }
  return newData;
}

LCMutableDataRef LCMutableDataCreate(LCByte data[], size_t length) {
  mutableDataRef dataStruct = dataCreateStruct();
  mutableDataEnsureLength(dataStruct, length);
  memcpy(dataStruct->data, data, length*sizeof(LCByte));
  dataStruct->length = length;
  return objectCreate(LCTypeMutableData, dataStruct);
};

void LCMutableDataAppendFromFile(LCMutableDataRef object, FILE* fp, size_t fileLength) {
  if (fileLength == -1) {
    fileLength = 10;
  }
  mutableDataRef dataStruct = objectData(object);
  while (!feof(fp) && !ferror(fp)) {
    mutableDataEnsureLength(dataStruct, dataStruct->length + fileLength);
    size_t lengthRead = fread(&(dataStruct->data[dataStruct->length]), sizeof(LCByte), mutableDataBufferLeft(dataStruct), fp);
    dataStruct->length = dataStruct->length + lengthRead;
    fileLength = fileLength * 2;
  }
}

size_t LCMutableDataLength(LCMutableDataRef data) {
  mutableDataRef dataStruct = objectData(data);
  return dataStruct->length;
}

LCByte* LCMutableDataDataRef(LCMutableDataRef data) {
  mutableDataRef dataStruct = objectData(data);
  return dataStruct->data;
}

void LCMutableDataAppend(LCMutableDataRef object, LCByte data[], size_t length) {
  mutableDataRef dataStruct = objectData(object);
  mutableDataEnsureLength(dataStruct, length);
  memcpy(&(dataStruct->data[dataStruct->length]), data, length * sizeof(LCByte));
  dataStruct->length = dataStruct->length + length;
}

void LCMutableDataAppendAlt(void *object, LCByte data[], size_t length) {
  LCMutableDataAppend(object, data, length);
}

void mutableDataDealloc(LCObjectRef data) {
  mutableDataRef dataStruct = objectData(data);
  lcFree(dataStruct->data);
  lcFree(dataStruct);
}

