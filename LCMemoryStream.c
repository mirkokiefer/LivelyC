
#include "LCMemoryStream.h"
#include "unistd.h"

int memoryStreamWrite(void *cookie, const char data[], int length);
int memoryStreamRead(void *cookie, char *buffer, int length);
int memoryStreamWriteClose(void *cookie);
int memoryStreamReadClose(void *cookie);

struct internalWriteCookie {
  void *cookie;
  writeStreamFun writeFun;
  closeStreamFun closeFun;
};

struct internalReadCookie {
  void *cookie;
  LCByte *data;
  size_t length;
  LCInteger position;
  bool freeOnClose;
  closeStreamFun closeFun;
};

FILE* createMemoryReadStream(void *cookie, LCByte data[], size_t length, bool freeOnClose, closeStreamFun closeFun) {
  struct internalReadCookie *internalCookie = malloc(sizeof(struct internalReadCookie));
  if (internalCookie) {
    internalCookie->cookie = cookie;
    internalCookie->data = data;
    internalCookie->length = length;
    internalCookie->position = 0;
    internalCookie->freeOnClose = freeOnClose;
    internalCookie->closeFun = closeFun;
    return funopen(internalCookie, memoryStreamRead, NULL, NULL, NULL);
  } else {
    return NULL;
  }
}

FILE* createMemoryWriteStream(void *cookie, writeStreamFun writeFun, closeStreamFun closeFun) {
  struct internalWriteCookie *internalCookie = malloc(sizeof(struct internalWriteCookie));
  if (internalCookie) {
    internalCookie->cookie = cookie;
    internalCookie->writeFun = writeFun;
    internalCookie->closeFun = closeFun;
    return funopen(internalCookie, NULL, memoryStreamWrite, NULL, NULL);
  } else {
    return NULL;
  }
}

int memoryStreamWrite(void *cookie, const char data[], int length) {
  struct internalWriteCookie *internalCookie = (struct internalWriteCookie*)cookie;
  internalCookie->writeFun(internalCookie->cookie, (LCByte*)data, length);
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

int memoryStreamWriteClose(void *cookie) {
  struct internalWriteCookie *internalCookie = (struct internalWriteCookie*)cookie;
  if (internalCookie->closeFun) {
    internalCookie->closeFun(internalCookie->cookie);
  }
  lcFree(cookie);
  return 0;
}

int memoryStreamReadClose(void *cookie) {
  struct internalReadCookie *internalCookie = (struct internalReadCookie*)cookie;
  if (internalCookie->freeOnClose) {
    lcFree(internalCookie->data);
  }
  if (internalCookie->closeFun) {
    internalCookie->closeFun(cookie);
  }
  lcFree(cookie);
  return 0;
}
