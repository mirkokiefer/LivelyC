
#ifndef LivelyStore_imac_LCMemoryStream_h
#define LivelyStore_imac_LCMemoryStream_h

#include "LCCore.h"

typedef LCObjectRef LCMemoryStreamRef;
extern LCTypeRef LCTypeMemoryStream;

LCMemoryStreamRef LCMemoryStreamCreate();
LCMemoryStreamRef LCMemoryStreamCreateFromFile(FILE* fp);
FILE* LCMemoryStreamWriteFile(LCMemoryStreamRef streamObj);
FILE* LCMemoryStreamReadFile(LCMemoryStreamRef streamObj);
size_t LCMemoryStreamLength(LCMemoryStreamRef streamObj);
void LCMemoryStreamData(LCMemoryStreamRef streamObj, LCByte buffer[], size_t length);

#endif
