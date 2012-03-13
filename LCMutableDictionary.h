
#ifndef LivelyStore_imac_LCMutableDictionary_h
#define LivelyStore_imac_LCMutableDictionary_h

#include "LCCore.h"
#include "LCMutableArray.h"
#include "LCKeyValue.h"

typedef LCObjectRef LCMutableDictionaryRef;
extern LCTypeRef LCTypeMutableDictionary;

LCMutableDictionaryRef LCMutableDictionaryCreate(LCKeyValueRef keyValues[], size_t length);
LCKeyValueRef LCMutableDictionaryEntryForKey(LCMutableDictionaryRef dict, LCObjectRef key);
LCObjectRef LCMutableDictionaryValueForKey(LCMutableDictionaryRef dict, LCObjectRef key);
void LCMutableDictionarySetValueForKey(LCMutableDictionaryRef dict, LCObjectRef key, LCObjectRef value);
void LCMutableDictionaryDeleteKey(LCMutableDictionaryRef dict, LCObjectRef key);
void LCMutableDictionaryAddEntry(LCMutableDictionaryRef dict, LCKeyValueRef keyValue);
void LCMutableDictionaryAddEntries(LCMutableDictionaryRef dict, LCKeyValueRef keyValues[], size_t length);
LCMutableDictionaryRef LCMutableDictionaryCopy(LCMutableDictionaryRef dict);
size_t LCMutableDictionaryLength(LCMutableDictionaryRef dict);
LCKeyValueRef* LCMutableDictionaryEntries(LCMutableDictionaryRef dict);
LCMutableArrayRef LCMutableDictionaryCreateChangesArray(LCMutableDictionaryRef original, LCMutableDictionaryRef new);
LCMutableArrayRef LCMutableDictionaryCreateAddedArray(LCMutableDictionaryRef original, LCMutableDictionaryRef new);
LCMutableArrayRef LCMutableDictionaryCreateUpdatedArray(LCMutableDictionaryRef original, LCMutableDictionaryRef new);
LCMutableArrayRef LCMutableDictionaryCreateDeletedArray(LCMutableDictionaryRef original, LCMutableDictionaryRef new);

#endif
