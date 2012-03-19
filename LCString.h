
#ifndef LivelyC_LCString_h
#define LivelyC_LCString_h

#include "LCCore.h"
#include "LCArray.h"
#include "LCData.h"

typedef LCObjectRef LCStringRef;
extern LCTypeRef LCTypeString;

LCStringRef LCStringCreate(char *string);
LCStringRef LCStringCreateFromHash(LCContextRef context, char hash[HASH_LENGTH]);
LCStringRef LCStringCreateFromChars(char* characters, size_t length);
LCStringRef LCStringCreateFromStrings(LCStringRef strings[], size_t length);
LCStringRef LCStringCreateFromStringArray(char* characters[], size_t length);
LCStringRef LCStringCreateFromData(LCDataRef data);
size_t LCStringLength(LCStringRef string);
bool LCStringEqual(LCStringRef string1, LCStringRef string2);
bool LCStringEqualCString(LCStringRef string, char* cString);
char* LCStringChars(LCStringRef string);
LCArrayRef LCStringCreateTokens(LCStringRef string, char delimiter);

#endif
