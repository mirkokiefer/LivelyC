
#ifndef LivelyStore_imac_LCPipe_h
#define LivelyStore_imac_LCPipe_h

#include "LCCore.h"

typedef LCObjectRef LCPipeRef;
extern LCTypeRef LCTypeMemoryStream;

LCPipeRef LCPipeCreate();
FILE* LCPipeWriteFile(LCPipeRef streamObj);
FILE* LCPipeReadFile(LCPipeRef streamObj);
size_t LCPipeLength(LCPipeRef streamObj);
void LCPipeData(LCPipeRef streamObj, LCByte buffer[], size_t length);

#endif
