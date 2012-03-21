
#include "LCMemoryStream.h"
#include "unistd.h"

int memoryStreamWrite(void *cookie, const char data[], int length);
int memoryStreamRead(void *cookie, char *buffer, int length);

struct internalWriteCookie {
  void *cookie;
  writeStreamFun fun;
};

struct internalReadCookie {
  LCByte *data;
  size_t length;
  LCInteger position;
};

FILE* createMemoryReadStream(LCByte data[], size_t length) {
  struct internalReadCookie *internalCookie = malloc(sizeof(struct internalReadCookie));
  if (internalCookie) {
    internalCookie->data = data;
    internalCookie->length = length;
    internalCookie->position = 0;
    return funopen(internalCookie, memoryStreamRead, NULL, NULL, NULL);
  } else {
    return NULL;
  }
}

FILE* createMemoryWriteStream(void *cookie, writeStreamFun fun) {
  struct internalWriteCookie *internalCookie = malloc(sizeof(struct internalWriteCookie));
  if (internalCookie) {
    internalCookie->cookie = cookie;
    internalCookie->fun = fun;
    return funopen(internalCookie, NULL, memoryStreamWrite, NULL, NULL);
  } else {
    return NULL;
  }
}

int memoryStreamWrite(void *cookie, const char data[], int length) {
  struct internalWriteCookie *internalCookie = (struct internalWriteCookie*)cookie;
  internalCookie->fun(internalCookie->cookie, (LCByte*)data, length);
  return length;
}

int memoryStreamRead(void *cookie, char *buffer, int length) {
  struct internalReadCookie *internalCookie = (struct internalReadCookie*)cookie;
  if (internalCookie->position >= internalCookie->length) {
    return 0;
  }
  if (internalCookie->position + length > internalCookie->length) {
    length = internalCookie->length - internalCookie->position;
  }
  memcpy(buffer, &(internalCookie->data[internalCookie->position]), sizeof(char)*length);
  internalCookie->position = internalCookie->position + length;
  return length;
}

int memoryStreamClose(void *cookie) {
  lcFree(cookie);
  return 0;
}
