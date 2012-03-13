
#include "LCData.h"
#include "LCMemoryStream.h"
#include "LCUtils.h"

typedef struct data* dataRef;

#define READ_BUFFER_SIZE 1024

void dataSerialize(LCDataRef data, FILE *fd);
void* dataDeserialize(LCDataRef data, FILE *fd);

struct data {
  size_t length;
  LCByte data[];
};

struct LCType typeData = {
  .serialize = dataSerialize,
  .deserialize = dataDeserialize
};

LCTypeRef LCTypeData = &typeData;

LCDataRef LCDataCreate(LCByte data[], size_t length) {
  dataRef newData = malloc(sizeof(struct data) + length*sizeof(LCByte));
  if (newData) {
    newData->length = length;
    memcpy(newData->data, data, length*sizeof(LCByte));
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

void dataSerialize(LCDataRef data, FILE *fd) {
  fwrite(LCDataDataRef(data), sizeof(LCByte), LCDataLength(data), fd);
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
