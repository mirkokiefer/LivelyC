
#include "LCData.h"
#include "LCMemoryStream.h"

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

static dataRef dataStructCreate(LCByte data[], size_t length) {
  dataRef newData = malloc(sizeof(struct data) + length*sizeof(LCByte));
  if (newData) {
    newData->length = length;
    memcpy(newData->data, data, length*sizeof(LCByte));
    return newData;
  } else {
    return NULL;
  }
}

LCDataRef LCDataCreate(LCByte data[], size_t length) {
  return objectCreate(LCTypeData, dataStructCreate(data, length));
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
  LCMemoryStreamRef memoryStream = LCMemoryStreamCreate();
  FILE* writeStream = LCMemoryStreamFile(memoryStream);
  LCByte buffer[READ_BUFFER_SIZE];
  while(fread(buffer, sizeof(LCByte), READ_BUFFER_SIZE, fd) > 0) {
    fwrite(buffer, sizeof(LCByte), READ_BUFFER_SIZE, writeStream);
  }
  dataRef dataStruct = dataStructCreate((LCByte*)LCMemoryStreamData(memoryStream), LCMemoryStreamLength(memoryStream));
  objectRelease(memoryStream);
  return dataStruct;
}
