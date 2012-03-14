
#ifndef LivelyStore_LCSHA_h
#define LivelyStore_LCSHA_h

#include "LCCore.h"
#include "LCUtils.h"
#include "LCString.h"
#include "LCMemoryStream.h"

void LCCreateSHAString(LCByte data[], size_t length, char buffer[HASH_LENGTH]);
void* createHashContext(LCMemoryStreamRef stream);
void updateHashContext(void* context);
void finalizeHashContext(void* context, char buffer[HASH_LENGTH]);
#endif
