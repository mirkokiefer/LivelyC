
#ifndef LivelyStore_imac_LCMemoryStream_h
#define LivelyStore_imac_LCMemoryStream_h

#include "LCCore.h"

typedef void(*writeStreamFun)(void *cookie, LCByte data[], size_t length);
typedef void(*closeStreamFun)(void *cookie);

FILE* createMemoryReadStream(void *cookie, LCByte data[], size_t length, bool freeOnClose, closeStreamFun closeFun);
FILE* createMemoryWriteStream(void *cookie, writeStreamFun writeFun, closeStreamFun closeFun);

#endif
