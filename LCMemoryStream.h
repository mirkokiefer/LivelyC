
#ifndef LivelyStore_imac_LCMemoryStream_h
#define LivelyStore_imac_LCMemoryStream_h

#include "LCCore.h"

typedef LCObjectRef LCMemoryStreamRef;
extern LCTypeRef LCTypeMemoryStreamLarge;

LCMemoryStreamRef LCMemoryStreamCreate();
FILE* LCMemoryStreamWriteFile(LCMemoryStreamRef streamObj);
FILE* LCMemoryStreamReadFile(LCMemoryStreamRef streamObj);
size_t LCMemoryStreamLength(LCMemoryStreamRef streamObj);
char* LCMemoryStreamData(LCMemoryStreamRef streamObj);

#endif
