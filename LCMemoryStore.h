
#ifndef LivelyC_LCMemoryStore_h
#define LivelyC_LCMemoryStore_h

#include "LCCore.h"

typedef LCObjectRef LCMemoryStoreRef;
extern LCTypeRef LCTypeMemoryStore;

LCMemoryStoreRef LCMemoryStoreCreate();
LCStoreRef LCMemoryStoreStoreObject(LCMemoryStoreRef store);

#endif
