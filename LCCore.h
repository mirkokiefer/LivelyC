

#ifndef LivelyC_LCCore_h
#define LivelyC_LCCore_h

#include "ExternalLibs.h"

#define HASH_LENGTH 41
#define LC_HASH_BYTE_LENGTH 20

//Errors
#define ErrorObjectImmutable "can't add mutable objects to immutable object"

typedef int LCInteger;
typedef unsigned char LCByte;

typedef struct LCObject* LCObjectRef;
typedef struct LCType* LCTypeRef;
typedef struct LCStore*  LCStoreRef;
typedef struct LCContext* LCContextRef;

typedef void*(*LCCreateEachCb)(LCInteger i, void* info, void* each);

typedef FILE*(*writeData)(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);
typedef void(*deleteData)(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);
typedef FILE*(*readData)(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);

typedef void (*callback)(void *cookie);

typedef enum {
  LCEqual,
  LCGreater,
  LCSmaller
} LCCompare;

struct LCType {
  bool immutable;
  void (*dealloc)(LCObjectRef object);
  LCCompare (*compare)(LCObjectRef object1, LCObjectRef object2);
  void (*serialize)(LCObjectRef object, void* cookie, callback flushFunct, FILE *fd);
  void* (*deserialize)(LCObjectRef object, FILE *fd);
  void *meta;
};

LCObjectRef objectCreate(LCTypeRef type, void* data);
LCObjectRef objectCreateFromContext(LCContextRef context, LCTypeRef type, char hash[HASH_LENGTH]);
void* objectData(LCObjectRef object);
LCTypeRef objectGetType(LCObjectRef object);
bool objectImmutable(LCObjectRef object);
bool objectsImmutable(LCObjectRef objects[], size_t length);
LCObjectRef objectRetain(LCObjectRef object);
LCObjectRef objectRelease(LCObjectRef object);
LCInteger objectRetainCount(LCObjectRef object);
LCCompare objectCompare(LCObjectRef object1, LCObjectRef object2);
LCContextRef objectContext(LCObjectRef object);
void objectSetContext(LCObjectRef object, LCContextRef context);
void objectSerialize(LCObjectRef object, FILE* fd);
void objectStore(LCObjectRef object, LCContextRef context);
void objectsStore(LCObjectRef objects[], size_t length, LCContextRef context);
void objectCache(LCObjectRef object);
void objectDeleteCache(LCObjectRef object);
char* objectHash(LCObjectRef object);

void objectsSort(LCObjectRef objects[], size_t length);

LCStoreRef storeCreate(void *cookie, writeData writefn, deleteData deletefn, readData readfn);
LCContextRef contextCreate(LCStoreRef store);

void lcFree(void* object);

#endif
