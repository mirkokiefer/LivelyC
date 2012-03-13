

#ifndef LivelyStore_LCMutableArray_h
#define LivelyStore_LCMutableArray_h

#include "LCCore.h"
#include "LCArray.h"

typedef LCObjectRef LCMutableArrayRef;
extern LCTypeRef LCTypeMutableArray;

LCMutableArrayRef LCMutableArrayCreate(LCObjectRef objects[], size_t length);
LCMutableArrayRef LCMutableArrayCreateFromArray(LCArrayRef array);
LCObjectRef* LCMutableArrayObjects(LCMutableArrayRef array);
LCObjectRef LCMutableArrayObjectAtIndex(LCMutableArrayRef array, LCInteger index);
size_t LCMutableArrayLength(LCMutableArrayRef array);
LCArrayRef LCMutableArrayCreateSubArray(LCMutableArrayRef array, LCInteger start, size_t length);

LCArrayRef LCMutableArrayCreateArray(LCMutableArrayRef array);
LCMutableArrayRef LCMutableArrayCopy(LCMutableArrayRef array);
void LCMutableArrayAddObject(LCMutableArrayRef array, LCObjectRef object);
void LCMutableArrayAddObjects(LCMutableArrayRef array, LCObjectRef objects[], size_t length);
void LCMutableArrayRemoveIndex(LCMutableArrayRef array, LCInteger index);
void LCMutableArrayRemoveObject(LCMutableArrayRef array, LCObjectRef object);
void LCMutableArraySort(LCMutableArrayRef array);

#endif
