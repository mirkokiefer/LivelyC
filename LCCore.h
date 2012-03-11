

#ifndef LivelyC_LCCore_h
#define LivelyC_LCCore_h

#include "ExternalLibs.h"

#define HASH_LENGTH 41

typedef int LCInteger;
typedef unsigned char LCByte;

typedef struct LCObject* LCObjectRef;
typedef struct LCType* LCTypeRef;
typedef struct LCStore*  LCStoreRef;
typedef struct LCContext* LCContextRef;

typedef FILE*(*writeData)(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);
typedef void(*deleteData)(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);
typedef FILE*(*readData)(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);

typedef enum {
  LCEqual,
  LCGreater,
  LCSmaller
} LCCompare;

struct LCType {
  void (*dealloc)(LCObjectRef object);
  LCCompare (*compare)(LCObjectRef object1, LCObjectRef object2);
  void (*serialize)(LCObjectRef object, FILE *fd);
  void (*deserialize)(LCObjectRef object, FILE *fd);
  void *meta;
};

LCObjectRef objectCreate(LCContextRef context, LCTypeRef type, char hash[HASH_LENGTH]);
void objectSetData(LCObjectRef object, void *data);
void* objectGetData(LCObjectRef object);
LCTypeRef objectGetType(LCObjectRef object);
LCObjectRef objectRetain(LCObjectRef object);
LCObjectRef objectRelease(LCObjectRef object);
LCInteger objectRetainCount(LCObjectRef object);
LCCompare objectCompare(LCObjectRef object1, LCObjectRef object2);
LCContextRef objectContext(LCObjectRef object);

void objectsSort(LCObjectRef objects[], size_t length);

LCStoreRef storeCreate(void *cookie, writeData writefn, deleteData deletefn, readData readfn);
LCContextRef contextCreate(LCStoreRef store);

void lcFree(void* object);

#endif
