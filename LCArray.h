

#ifndef LivelyStore_LCArray_h
#define LivelyStore_LCArray_h

#include "LCCore.h"

typedef LCObjectRef LCArrayRef;
extern LCTypeRef LCTypeArray;

typedef LCObjectRef LCMutableArrayRef;
extern LCTypeRef LCTypeMutableArray;

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

LCMutableArrayRef LCMutableArrayCreate(LCObjectRef objects[], size_t length);
LCObjectRef* LCMutableArrayObjects(LCMutableArrayRef array);
LCObjectRef LCMutableArrayObjectAtIndex(LCMutableArrayRef array, LCInteger index);
size_t LCMutableArrayLength(LCMutableArrayRef array);
LCArrayRef LCMutableArrayCreateSubArray(LCMutableArrayRef array, LCInteger start, size_t length);

LCMutableArrayRef LCMutableArrayCreateFromArray(LCArrayRef array);
LCArrayRef LCMutableArrayCreateArray(LCMutableArrayRef array);
LCMutableArrayRef LCMutableArrayCopy(LCMutableArrayRef array);
void LCMutableArrayAddObject(LCMutableArrayRef array, LCObjectRef object);
void LCMutableArrayAddObjects(LCMutableArrayRef array, LCObjectRef objects[], size_t length);
void LCMutableArrayRemoveIndex(LCMutableArrayRef array, LCInteger index);
void LCMutableArrayRemoveObject(LCMutableArrayRef array, LCObjectRef object);
void LCMutableArraySort(LCMutableArrayRef array);
#endif
