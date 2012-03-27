
#ifndef LivelyStore_LCKeyValue_h
#define LivelyStore_LCKeyValue_h

#include "LCCore.h"

typedef LCObjectRef LCKeyValueRef;
extern LCTypeRef LCTypeKeyValue;

LCKeyValueRef LCKeyValueCreate(LCObjectRef keyObject, LCObjectRef valueObject);
LCObjectRef LCKeyValueKey(LCKeyValueRef keyValue);
LCObjectRef LCKeyValueValue(LCKeyValueRef keyValue);

#endif
