
#include "LCMemoryStream.h"
#include "LCUtils.h"
#include "unistd.h"

typedef struct memoryStreamData* memoryStreamDataRef;

void memoryStreamDealloc(LCMemoryStreamRef object);

struct memoryStreamData {
  FILE* write;
  FILE* read;
};

struct LCType memoryStreamType = {
  .dealloc = memoryStreamDealloc,
};

LCTypeRef LCTypeMemoryStream = &memoryStreamType;

LCMemoryStreamRef LCMemoryStreamCreate() {
  memoryStreamDataRef stream = malloc(sizeof(struct memoryStreamData));
  if (stream) {
    int filedes[2];
    pipe(filedes);
    stream->write = fdopen(filedes[1], "w");
    stream->read = fdopen(filedes[0], "r");
    return objectCreate(LCTypeMemoryStream, stream);
  }
  return NULL;
}

FILE* LCMemoryStreamWriteFile(LCMemoryStreamRef streamObj) {
  memoryStreamDataRef data = objectData(streamObj);
  return data->write;
}

FILE* LCMemoryStreamReadFile(LCMemoryStreamRef streamObj) {
  memoryStreamDataRef data = objectData(streamObj);
  return data->read;
}

size_t LCMemoryStreamLength(LCMemoryStreamRef streamObj) {
  return fileLength(LCMemoryStreamReadFile(streamObj));
}

void LCMemoryStreamData(LCMemoryStreamRef streamObj, LCByte buffer[], size_t length) {
  readFromFile(LCMemoryStreamReadFile(streamObj), buffer, length);
}

void memoryStreamDealloc(LCMemoryStreamRef object) {
  memoryStreamDataRef streamData = objectData(object);
  fclose(streamData->write);
  fclose(streamData->read);
}
