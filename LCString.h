
#ifndef LivelyC_LCString_h
#define LivelyC_LCString_h

#include "LCCore.h"

typedef LCObjectRef LCStringRef;
extern LCTypeRef LCTypeString;

LCStringRef LCStringCreate(LCContextRef context, char *string);
LCStringRef LCStringCreateFromHash(LCContextRef context, char hash[HASH_LENGTH]);
LCStringRef LCStringCreateFromChars(LCContextRef context, char* characters, size_t length);
LCStringRef LCStringCreateFromStrings(LCContextRef context, LCStringRef strings[], size_t count);
size_t LCStringLength(LCStringRef string);
bool LCStringEqual(LCStringRef string1, LCStringRef string2);
char* LCStringChars(LCStringRef string);

#endif
