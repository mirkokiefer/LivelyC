
#include "LCPipe.h"
#include "LCUtils.h"
#include "unistd.h"

typedef struct pipeData* pipeDataRef;

void pipeDealloc(LCPipeRef object);

struct pipeData {
  FILE* write;
  FILE* read;
};

struct LCType pipeType = {
  .dealloc = pipeDealloc,
};

LCTypeRef LCTypeMemoryStream = &pipeType;

LCPipeRef LCPipeCreate() {
  pipeDataRef stream = malloc(sizeof(struct pipeData));
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
  pipeDataRef data = objectData(streamObj);
  return data->write;
}

FILE* LCPipeReadFile(LCPipeRef streamObj) {
  pipeDataRef data = objectData(streamObj);
  return data->read;
}

size_t LCPipeLength(LCPipeRef streamObj) {
  return fileLength(LCPipeReadFile(streamObj));
}

void LCPipeData(LCPipeRef streamObj, LCByte buffer[], size_t length) {
  readFromFile(LCPipeReadFile(streamObj), buffer, length);
}

void pipeDealloc(LCPipeRef object) {
  pipeDataRef streamData = objectData(object);
  fclose(streamData->write);
  fclose(streamData->read);
  lcFree(streamData);
}
