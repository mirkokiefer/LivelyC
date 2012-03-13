

#ifndef LivelyStore_LCArray_h
#define LivelyStore_LCArray_h

#include "LCCore.h"

typedef LCObjectRef LCArrayRef;
extern LCTypeRef LCTypeArray;

LCArrayRef LCArrayCreate(LCObjectRef objects[], size_t length);
LCArrayRef LCArrayCreateFromHash(LCContextRef context, char hash[HASH_LENGTH]);
LCArrayRef LCArrayCreateAppendingObject(LCArrayRef array, LCObjectRef object);
LCArrayRef LCArrayCreateAppendingObjects(LCArrayRef array, LCObjectRef objects[], size_t length);
LCArrayRef LCArrayCreateFromArrays(LCArrayRef arrays[], size_t length);
LCObjectRef* LCArrayObjects(LCArrayRef array);
LCObjectRef LCArrayObjectAtIndex(LCArrayRef array, LCInteger index);
size_t LCArrayLength(LCArrayRef array);
LCArrayRef LCArrayCreateSubArray(LCArrayRef array, LCInteger start, size_t length);
LCArrayRef LCArrayCreateArrayWithMap(LCArrayRef array, void* info, LCCreateEachCb each);

#endif
