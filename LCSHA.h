
#ifndef LivelyStore_LCSHA_h
#define LivelyStore_LCSHA_h

#include "LCCore.h"
#include "LCUtils.h"
#include "LCString.h"
#include "LCPipe.h"

void createSHAString(LCByte data[], size_t length, char buffer[HASH_LENGTH]);
void* createHashContext(void);
void updateHashContext(void* context, LCByte data[], size_t length);
void finalizeHashContext(void* context, char buffer[HASH_LENGTH]);
#endif
