
#include "LCData.h"
#include "LCMemoryStream.h"
#include "LCUtils.h"

typedef struct data* dataRef;

#define READ_BUFFER_SIZE 1024

void dataSerialize(LCObjectRef object, void* cookie, callback flush, FILE* fd);
void* dataDeserialize(LCDataRef data, FILE *fd);

struct data {
  size_t length;
  LCByte* data;
};

struct LCType typeData = {
  .immutable = true,
  .serialize = dataSerialize,
  .deserialize = dataDeserialize
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

static LCByte* reallocBuffer(LCByte oldBuffer[], size_t length) {
  LCByte* newBuffer = realloc(oldBuffer, sizeof(void*) * length);
  return newBuffer;
}

LCDataRef LCDataCreateFromFile(FILE* fp, size_t length) {
  if (length == -1) {
    length = 10;
  }
  LCByte* buffer = reallocBuffer(NULL, length);
  size_t dataRead = 0;
  while (!feof(fp) && !ferror(fp)) {
    if (dataRead == length) {
      length = length * 2;
      buffer = reallocBuffer(buffer, length);
    }
    dataRead = dataRead + fread(buffer, sizeof(LCByte), length-dataRead, fp);
  }
  dataRef data = dataCreateStruct(dataRead);
  data->data = buffer;
  return objectCreate(LCTypeData, data);
}

size_t LCDataLength(LCDataRef data) {
  dataRef dataStruct = objectData(data);
  return dataStruct->length;
}

LCByte* LCDataDataRef(LCDataRef data) {
  dataRef dataStruct = objectData(data);
  return dataStruct->data;
}

void dataSerialize(LCObjectRef data, void* cookie, callback flush, FILE* fd) {
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
