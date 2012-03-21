
#ifndef LivelyStore_imac_LCMemoryStream_h
#define LivelyStore_imac_LCMemoryStream_h

#include "LCCore.h"

typedef void(*writeStreamFun)(void *cookie, LCByte data[], size_t length);

FILE* createMemoryReadStream(LCByte data[], size_t length);
FILE* createMemoryWriteStream(void *cookie, writeStreamFun fun);

#endif
