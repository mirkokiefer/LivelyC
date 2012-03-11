
#include "LCMemoryStream.h"
#include "unistd.h"

void memoryStreamDealloc(LCMemoryStreamRef object);
void memoryStreamSerialize(LCMemoryStreamRef object, FILE* fd);
static bool memoryStreamResizeBuffer(LCMemoryStreamRef stream, size_t length);
int memoryStreamWrite(void *cookie, const char *data, int length);
fpos_t memoryStreamSeek(void *cookie, fpos_t offset, int whence);
int memoryStreamRead(void *cookie, char *buffer, int length);
int memoryStreamClose(void *cookie);

struct memoryStreamData {
  char* buffer;
  size_t bufferLength;
  size_t dataWritten;
};
typedef struct memoryStreamData* memoryStreamDataRef;

struct cookie {
  LCMemoryStreamRef stream;
  size_t position;
};

struct LCType memoryStreamType = {
  .dealloc = memoryStreamDealloc,
};
LCTypeRef LCTypeMemoryStream = &memoryStreamType;


LCMemoryStreamRef LCMemoryStreamCreate(LCContextRef context) {
  memoryStreamDataRef stream = malloc(sizeof(struct memoryStreamData));
  if (stream) {
    LCMemoryStreamRef object = objectCreate(context, LCTypeMemoryStream, NULL);
    objectSetData(object, stream);
    stream->dataWritten = 0;
    memoryStreamResizeBuffer(object, 10);
    return object;
  }
  return NULL;
}

FILE* LCMemoryStreamFile(LCMemoryStreamRef streamObj) {
  // on linux use fopencookie
  struct cookie *cookie = malloc(sizeof(struct cookie));
  if (cookie) {
    cookie->stream = objectRetain(streamObj);
    cookie->position = 0;
    return funopen(cookie, memoryStreamRead, memoryStreamWrite, memoryStreamSeek, memoryStreamClose);
  } else {
    return NULL;
  }
}

size_t LCMemoryStreamLength(LCMemoryStreamRef streamObj) {
  memoryStreamDataRef streamData = objectGetData(streamObj);
  return streamData->dataWritten;
}

char* LCMemoryStreamData(LCMemoryStreamRef streamObj) {
  memoryStreamDataRef streamData = objectGetData(streamObj);
  return streamData->buffer;
}

void memoryStreamDealloc(LCMemoryStreamRef object) {
  memoryStreamDataRef streamData = objectGetData(object);
  free(streamData->buffer);
}

void memoryStreamSerialize(LCMemoryStreamRef object, FILE* fd) {
  memoryStreamDataRef streamData = objectGetData(object);
  fwrite(streamData->buffer, sizeof(char), streamData->dataWritten, fd);
}

static bool memoryStreamResizeBuffer(LCMemoryStreamRef stream, size_t length) {
  memoryStreamDataRef streamData = objectGetData(stream);
  void* buffer = realloc(streamData->buffer, sizeof(char) * length);
  if(buffer) {
    streamData->buffer = buffer;
    streamData->bufferLength = length;
    return true;
  } else {
    return false;
  }
}

int memoryStreamWrite(void *cookie, const char *data, int length) {
  struct cookie *cookieStruct = (struct cookie*)cookie;
  LCMemoryStreamRef streamObj = cookieStruct->stream;
  memoryStreamDataRef streamData = objectGetData(streamObj);
  if (cookieStruct->position+length >= streamData->bufferLength) {
    memoryStreamResizeBuffer(streamObj, (streamData->bufferLength+length)*2);
  }
  memcpy(&(streamData->buffer[cookieStruct->position]), data, sizeof(char)*length);
  cookieStruct->position = cookieStruct->position + length;
  if (cookieStruct->position > streamData->dataWritten) {
    streamData->dataWritten = cookieStruct->position;
  }
  return length;
}

fpos_t memoryStreamSeek(void *cookie, fpos_t offset, int whence) {
  struct cookie *cookieStruct = (struct cookie*)cookie;
  LCMemoryStreamRef streamObj = cookieStruct->stream;
  memoryStreamDataRef streamData = objectGetData(streamObj);
  switch (whence) {
    case SEEK_SET:
      cookieStruct->position = offset;
      break;
    case SEEK_CUR:
      cookieStruct->position = cookieStruct->position + offset;
    case SEEK_END:
      cookieStruct->position = streamData->dataWritten + offset;
    default:
      break;
  }
  if (cookieStruct->position >= streamData->bufferLength) {
    size_t oldLength = streamData->bufferLength;
    memoryStreamResizeBuffer(streamObj, cookieStruct->position*2);
    for (LCInteger i=oldLength; i<cookieStruct->position; i++) {
      streamData->buffer[i] = 0;
    }
  }
  return cookieStruct->position;
}

int memoryStreamRead(void *cookie, char *buffer, int length) {
  struct cookie *cookieStruct = (struct cookie*)cookie;
  LCMemoryStreamRef streamObj = cookieStruct->stream;
  memoryStreamDataRef streamData = objectGetData(streamObj);
  if (cookieStruct->position >= streamData->dataWritten) {
    return 0;
  }
  if ((cookieStruct->position + length) >= streamData->dataWritten) {
    length = streamData->dataWritten - cookieStruct->position;
  }
  memcpy(buffer, &(streamData->buffer[cookieStruct->position]), sizeof(char)*length);
  cookieStruct->position = cookieStruct->position + length;
  return length;
}

int memoryStreamClose(void *cookie) {
  struct cookie *cookieStruct = (struct cookie*)cookie;
  objectRelease(cookieStruct->stream);
  return 0;
}