
#include "LCMemoryStreamLarge.h"
#include "unistd.h"

typedef struct memoryStreamLargeData* memoryStreamLargeDataRef;

void memoryStreamLargeDealloc(LCMemoryStreamLargeRef object);
void memoryStreamLargeSerialize(LCMemoryStreamLargeRef object, FILE* fd);
static bool memoryStreamLargeResizeBuffer(memoryStreamLargeDataRef streamData, size_t length);
int memoryStreamLargeWrite(void *cookie, const char *data, int length);
fpos_t memoryStreamLargeSeek(void *cookie, fpos_t offset, int whence);
int memoryStreamLargeRead(void *cookie, char *buffer, int length);
int memoryStreamLargeClose(void *cookie);

struct memoryStreamLargeData {
  char* buffer;
  size_t bufferLength;
  size_t dataWritten;
};

struct cookie {
  LCMemoryStreamLargeRef stream;
  size_t position;
};

struct LCType memoryStreamLargeType = {
  .dealloc = memoryStreamLargeDealloc,
};
LCTypeRef LCTypeMemoryStreamLarge = &memoryStreamLargeType;

LCMemoryStreamLargeRef LCMemoryStreamLargeCreate() {
  memoryStreamLargeDataRef stream = malloc(sizeof(struct memoryStreamLargeData));
  if (stream) {
    stream->dataWritten = 0;
    stream->buffer = NULL;
    memoryStreamLargeResizeBuffer(stream, 10);
    return objectCreate(LCTypeMemoryStreamLarge, stream);
  }
  return NULL;
}

static struct cookie* memoryStreamLargeCreateCookie(LCMemoryStreamLargeRef stream, size_t position) {
  struct cookie *cookie = malloc(sizeof(struct cookie));
  if (cookie) {
    cookie->stream = objectRetain(stream);
    cookie->position = position;
  }
  return cookie;
}

FILE* LCMemoryStreamLargeWriteFile(LCMemoryStreamLargeRef streamObj) {
  // on linux use fopencookie
  memoryStreamLargeDataRef data = objectData(streamObj);
  struct cookie *cookie = memoryStreamLargeCreateCookie(streamObj, data->dataWritten);
  return funopen(cookie, memoryStreamLargeRead, memoryStreamLargeWrite, memoryStreamLargeSeek, memoryStreamLargeClose);
}

FILE* LCMemoryStreamLargeReadFile(LCMemoryStreamLargeRef streamObj) {
  struct cookie *cookie = memoryStreamLargeCreateCookie(streamObj, 0);
  return funopen(cookie, memoryStreamLargeRead, NULL, memoryStreamLargeSeek, memoryStreamLargeClose);
}

size_t LCMemoryStreamLargeLength(LCMemoryStreamLargeRef streamObj) {
  memoryStreamLargeDataRef streamData = objectData(streamObj);
  return streamData->dataWritten;
}

char* LCMemoryStreamLargeData(LCMemoryStreamLargeRef streamObj) {
  memoryStreamLargeDataRef streamData = objectData(streamObj);
  return streamData->buffer;
}

void memoryStreamLargeDealloc(LCMemoryStreamLargeRef object) {
  memoryStreamLargeDataRef streamData = objectData(object);
  free(streamData->buffer);
}

void memoryStreamLargeSerialize(LCMemoryStreamLargeRef object, FILE* fd) {
  memoryStreamLargeDataRef streamData = objectData(object);
  fwrite(streamData->buffer, sizeof(char), streamData->dataWritten, fd);
}

static bool memoryStreamLargeResizeBuffer(memoryStreamLargeDataRef streamData, size_t length) {
  void* buffer = realloc(streamData->buffer, sizeof(char) * length);
  if(buffer) {
    streamData->buffer = buffer;
    streamData->bufferLength = length;
    return true;
  } else {
    return false;
  }
}

int memoryStreamLargeWrite(void *cookie, const char *data, int length) {
  struct cookie *cookieStruct = (struct cookie*)cookie;
  LCMemoryStreamLargeRef streamObj = cookieStruct->stream;
  memoryStreamLargeDataRef streamData = objectData(streamObj);
  if (cookieStruct->position+length >= streamData->bufferLength) {
    memoryStreamLargeResizeBuffer(streamData, (streamData->bufferLength+length)*2);
  }
  memcpy(&(streamData->buffer[cookieStruct->position]), data, sizeof(char)*length);
  cookieStruct->position = cookieStruct->position + length;
  if (cookieStruct->position > streamData->dataWritten) {
    streamData->dataWritten = cookieStruct->position;
  }
  return length;
}

fpos_t memoryStreamLargeSeek(void *cookie, fpos_t offset, int whence) {
  struct cookie *cookieStruct = (struct cookie*)cookie;
  LCMemoryStreamLargeRef streamObj = cookieStruct->stream;
  memoryStreamLargeDataRef streamData = objectData(streamObj);
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
    memoryStreamLargeResizeBuffer(streamData, cookieStruct->position*2);
    for (LCInteger i=oldLength; i<cookieStruct->position; i++) {
      streamData->buffer[i] = 0;
    }
  }
  return cookieStruct->position;
}

int memoryStreamLargeRead(void *cookie, char *buffer, int length) {
  struct cookie *cookieStruct = (struct cookie*)cookie;
  LCMemoryStreamLargeRef streamObj = cookieStruct->stream;
  memoryStreamLargeDataRef streamData = objectData(streamObj);
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

int memoryStreamLargeClose(void *cookie) {
  struct cookie *cookieStruct = (struct cookie*)cookie;
  objectRelease(cookieStruct->stream);
  return 0;
}