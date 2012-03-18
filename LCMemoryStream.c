
#include "LCMemoryStream.h"
#include "unistd.h"

typedef struct memoryStreamData* memoryStreamDataRef;

void memoryStreamDealloc(LCMemoryStreamRef object);
void memoryStreamSerialize(LCMemoryStreamRef object, FILE* fd);
static bool memoryStreamResizeBuffer(memoryStreamDataRef streamData, size_t length);
int memoryStreamWrite(void *cookie, const char *data, int length);
fpos_t memoryStreamSeek(void *cookie, fpos_t offset, int whence);
int memoryStreamRead(void *cookie, char *buffer, int length);
int memoryStreamClose(void *cookie);

struct memoryStreamData {
  char* buffer;
  size_t bufferLength;
  size_t dataWritten;
};

struct cookie {
  LCMemoryStreamRef stream;
  size_t position;
};

struct LCType memoryStreamType = {
  .dealloc = memoryStreamDealloc,
};
LCTypeRef LCTypeMemoryStreamLarge = &memoryStreamType;

LCMemoryStreamRef LCMemoryStreamCreate() {
  memoryStreamDataRef stream = malloc(sizeof(struct memoryStreamData));
  if (stream) {
    stream->dataWritten = 0;
    stream->buffer = NULL;
    memoryStreamResizeBuffer(stream, 10);
    return objectCreate(LCTypeMemoryStreamLarge, stream);
  }
  return NULL;
}

static struct cookie* memoryStreamCreateCookie(LCMemoryStreamRef stream, size_t position) {
  struct cookie *cookie = malloc(sizeof(struct cookie));
  if (cookie) {
    cookie->stream = objectRetain(stream);
    cookie->position = position;
  }
  return cookie;
}

FILE* LCMemoryStreamWriteFile(LCMemoryStreamRef streamObj) {
  // on linux use fopencookie
  memoryStreamDataRef data = objectData(streamObj);
  struct cookie *cookie = memoryStreamCreateCookie(streamObj, data->dataWritten);
  return funopen(cookie, memoryStreamRead, memoryStreamWrite, memoryStreamSeek, memoryStreamClose);
}

FILE* LCMemoryStreamReadFile(LCMemoryStreamRef streamObj) {
  struct cookie *cookie = memoryStreamCreateCookie(streamObj, 0);
  return funopen(cookie, memoryStreamRead, NULL, memoryStreamSeek, memoryStreamClose);
}

size_t LCMemoryStreamLength(LCMemoryStreamRef streamObj) {
  memoryStreamDataRef streamData = objectData(streamObj);
  return streamData->dataWritten;
}

char* LCMemoryStreamData(LCMemoryStreamRef streamObj) {
  memoryStreamDataRef streamData = objectData(streamObj);
  return streamData->buffer;
}

void memoryStreamDealloc(LCMemoryStreamRef object) {
  memoryStreamDataRef streamData = objectData(object);
  free(streamData->buffer);
}

void memoryStreamSerialize(LCMemoryStreamRef object, FILE* fd) {
  memoryStreamDataRef streamData = objectData(object);
  fwrite(streamData->buffer, sizeof(char), streamData->dataWritten, fd);
}

static bool memoryStreamResizeBuffer(memoryStreamDataRef streamData, size_t length) {
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
  memoryStreamDataRef streamData = objectData(streamObj);
  if (cookieStruct->position+length >= streamData->bufferLength) {
    memoryStreamResizeBuffer(streamData, (streamData->bufferLength+length)*2);
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
  memoryStreamDataRef streamData = objectData(streamObj);
  switch (whence) {
    case SEEK_SET:
      cookieStruct->position = offset;
      break;
    case SEEK_CUR:
      cookieStruct->position = cookieStruct->position + offset;
      break;
    case SEEK_END:
      cookieStruct->position = streamData->dataWritten + offset;
      break;
    default:
      break;
  }
  if (cookieStruct->position >= streamData->bufferLength) {
    size_t oldLength = streamData->bufferLength;
    memoryStreamResizeBuffer(streamData, cookieStruct->position*2);
    for (LCInteger i=oldLength; i<cookieStruct->position; i++) {
      streamData->buffer[i] = 0;
    }
  }
  return cookieStruct->position;
}

int memoryStreamRead(void *cookie, char *buffer, int length) {
  struct cookie *cookieStruct = (struct cookie*)cookie;
  LCMemoryStreamRef streamObj = cookieStruct->stream;
  memoryStreamDataRef streamData = objectData(streamObj);
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