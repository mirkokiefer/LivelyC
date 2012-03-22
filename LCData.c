
#include "LCData.h"
#include "LCPipe.h"
#include "LCUtils.h"
#include "LCMemoryStream.h"

typedef struct data* dataRef;

#define READ_BUFFER_SIZE 1024

FILE* dataSerialize(LCObjectRef object);
void* dataDeserialize(LCDataRef data, FILE *fd);
void dataDealloc(LCObjectRef data);

struct data {
  size_t length;
  LCByte* data;
};

struct LCType typeData = {
  .name = "LCData",
  .immutable = true,
  .serializationFormat = LCBinary,
  .dealloc = dataDealloc,
  .serializeData = dataSerialize,
  .deserializeData = dataDeserialize
};

LCTypeRef LCTypeData = &typeData;

static dataRef dataCreateStruct(size_t length) {
  dataRef newData = malloc(sizeof(struct data));
  if (newData) {
    newData->length = length;
  }
  return newData;
}

LCDataRef LCDataCreate(LCByte data[], size_t length) {
  LCByte* dataBuffer = malloc(sizeof(LCByte)*length);
  if (dataBuffer) {
    memcpy(dataBuffer, data, length*sizeof(LCByte));
    dataRef newData = dataCreateStruct(length);
    newData->data = dataBuffer;
    return objectCreate(LCTypeData, newData);
  } else {
    return NULL;
  }
};

size_t LCDataLength(LCDataRef data) {
  dataRef dataStruct = objectData(data);
  return dataStruct->length;
}

LCByte* LCDataDataRef(LCDataRef data) {
  dataRef dataStruct = objectData(data);
  return dataStruct->data;
}

void dataDealloc(LCObjectRef data) {
  dataRef dataStruct = objectData(data);
  lcFree(dataStruct->data);
  lcFree(dataStruct);
}

FILE* dataSerialize(LCObjectRef data) {
  objectRetain(data);
  return createMemoryReadStream(data, LCDataDataRef(data), LCDataLength(data), false, objectReleaseAlt);
}

void* dataDeserialize(LCDataRef data, FILE *fd) {
  size_t length = fileLength(fd);
  dataRef newData = malloc(sizeof(struct data) + length*sizeof(LCByte));
  if (newData) {
    newData->length = length;
    readFromFile(fd, newData->data, length);
    return newData;
  } else {
    return NULL;
  }
}