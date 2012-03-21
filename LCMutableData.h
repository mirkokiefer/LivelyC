
#ifndef LivelyC_LCMutableData_h
#define LivelyC_LCMutableData_h

#include "LCCore.h"

typedef LCObjectRef LCMutableDataRef;
extern LCTypeRef LCTypeMutableData;

LCMutableDataRef LCMutableDataCreate(LCByte data[], size_t length);
size_t LCMutableDataLength(LCMutableDataRef object);
LCByte* LCMutableDataDataRef(LCMutableDataRef object);
void LCMutableDataAppend(LCMutableDataRef object, LCByte data[], size_t length);
void LCMutableDataAppendFromFile(LCMutableDataRef object, FILE* fp, size_t fileLength);
#endif
