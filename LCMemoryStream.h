
#ifndef LivelyStore_imac_LCMemoryStream_h
#define LivelyStore_imac_LCMemoryStream_h

#include "LCCore.h"

typedef LCObjectRef LCMemoryStreamRef;
extern LCTypeRef LCTypeMemoryStream;

LCMemoryStreamRef LCMemoryStreamCreate();
FILE* LCMemoryStreamFile(LCMemoryStreamRef streamObj);
size_t LCMemoryStreamLength(LCMemoryStreamRef streamObj);
char* LCMemoryStreamData(LCMemoryStreamRef streamObj);

#endif
