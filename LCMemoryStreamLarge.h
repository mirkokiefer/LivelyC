
#ifndef LivelyStore_imac_LCMemoryStreamLarge_h
#define LivelyStore_imac_LCMemoryStreamLarge_h

#include "LCCore.h"

typedef LCObjectRef LCMemoryStreamLargeRef;
extern LCTypeRef LCTypeMemoryStreamLarge;

LCMemoryStreamLargeRef LCMemoryStreamLargeCreate();
FILE* LCMemoryStreamLargeFile(LCMemoryStreamLargeRef streamObj);
size_t LCMemoryStreamLargeLength(LCMemoryStreamLargeRef streamObj);
char* LCMemoryStreamLargeData(LCMemoryStreamLargeRef streamObj);

#endif
