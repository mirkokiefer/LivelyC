
#ifndef LivelyStore_LCSHA_h
#define LivelyStore_LCSHA_h

#include "LCCore.h"
#include "LCUtils.h"
#include "LCString.h"
#include "LCPipe.h"

void createSHAString(LCByte data[], size_t length, char buffer[HASH_LENGTH]);
void* createHashContext(LCPipeRef stream);
void updateHashContext(void* context);
void finalizeHashContext(void* context, char buffer[HASH_LENGTH]);
#endif
