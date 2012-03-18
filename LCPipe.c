
#include "LCPipe.h"
#include "LCUtils.h"
#include "unistd.h"

typedef struct memoryStreamData* memoryStreamDataRef;

void memoryStreamDealloc(LCPipeRef object);

struct memoryStreamData {
  FILE* write;
  FILE* read;
};

struct LCType memoryStreamType = {
  .dealloc = memoryStreamDealloc,
};

LCTypeRef LCTypeMemoryStream = &memoryStreamType;

LCPipeRef LCPipeCreate() {
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

FILE* LCPipeWriteFile(LCPipeRef streamObj) {
  memoryStreamDataRef data = objectData(streamObj);
  return data->write;
}

FILE* LCPipeReadFile(LCPipeRef streamObj) {
  memoryStreamDataRef data = objectData(streamObj);
  return data->read;
}

size_t LCPipeLength(LCPipeRef streamObj) {
  return fileLength(LCPipeReadFile(streamObj));
}

void LCPipeData(LCPipeRef streamObj, LCByte buffer[], size_t length) {
  readFromFile(LCPipeReadFile(streamObj), buffer, length);
}

void memoryStreamDealloc(LCPipeRef object) {
  memoryStreamDataRef streamData = objectData(object);
  fclose(streamData->write);
  fclose(streamData->read);
  lcFree(streamData);
}
