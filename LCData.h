

#ifndef LivelyStore_LCData_h
#define LivelyStore_LCData_h

#include "LCCore.h"

typedef LCObjectRef LCDataRef;
extern LCTypeRef LCTypeData;

LCDataRef LCDataCreate(LCByte data[], size_t length);
LCDataRef LCDataCreateFromFile(FILE* fp, size_t length);
size_t LCDataLength(LCDataRef data);
LCByte* LCDataDataRef(LCDataRef data);
#endif
