
#ifndef LivelyC_LCFileStore_h
#define LivelyC_LCFileStore_h

#include "LCCore.h"

typedef LCObjectRef LCFileStoreRef;
extern LCTypeRef LCTypeFileStore;

LCFileStoreRef LCFileStoreCreate(char *location);
LCStoreRef LCFileStoreStoreObject(LCFileStoreRef store);

#endif
