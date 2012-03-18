

#ifndef LivelyC_LCCore_h
#define LivelyC_LCCore_h

#include "ExternalLibs.h"

#define HASH_LENGTH 41
#define LC_HASH_BYTE_LENGTH 20

extern char *LCUnnamedObject;
//Errors
#define ErrorObjectImmutable "can't add mutable objects to immutable object"

typedef int LCInteger;
typedef unsigned char LCByte;

typedef enum {
  LCEqual,
  LCGreater,
  LCSmaller
} LCCompare;

typedef enum {
  LCBinary,
  LCText
} LCFormat;

typedef struct LCObject* LCObjectRef;
typedef struct LCType* LCTypeRef;
typedef struct LCStore*  LCStoreRef;
typedef struct LCContext* LCContextRef;

typedef LCTypeRef(*stringToType)(char *typeString);

typedef void*(*LCCreateEachCb)(LCInteger i, void* info, void* each);

typedef FILE*(*writeData)(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);
typedef void(*deleteData)(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);
typedef FILE*(*readData)(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);

typedef void (*callback)(void *cookie);
typedef void(*childCallback) (void *cookie, char *key, LCObjectRef objects[], size_t length, LCInteger depth);

struct LCType {
  char* name;
  bool immutable;
  LCFormat serializationFormat;
  void (*dealloc)(LCObjectRef object);
  LCCompare (*compare)(LCObjectRef object1, LCObjectRef object2);
  void (*serializeData)(LCObjectRef object, void *cookie, callback flushFunct, FILE *fd);
  void* (*deserializeData)(LCObjectRef object, FILE *fd);
  void* (*initData)();
  void (*walkChildren)(LCObjectRef object, void *cookie, childCallback cb);
  void (*storeChildren)(LCObjectRef object, char *key, LCObjectRef objects[], size_t length);
  void *meta;
};

LCObjectRef objectCreate(LCTypeRef type, void* data);
LCObjectRef objectCreateFromContext(LCContextRef context, LCTypeRef type, char hash[HASH_LENGTH]);
void* objectData(LCObjectRef object);
LCTypeRef objectType(LCObjectRef object);
bool objectImmutable(LCObjectRef object);
bool objectsImmutable(LCObjectRef objects[], size_t length);
LCObjectRef objectRetain(LCObjectRef object);
LCObjectRef objectRelease(LCObjectRef object);
LCInteger objectRetainCount(LCObjectRef object);
LCCompare objectCompare(LCObjectRef object1, LCObjectRef object2);
LCContextRef objectContext(LCObjectRef object);
void objectSetContext(LCObjectRef object, LCContextRef context);
void objectSerialize(LCObjectRef object, FILE* fd);
void objectDeserialize(LCObjectRef object, FILE* fd);
char* objectHash(LCObjectRef object);
void objectStore(LCObjectRef object, LCContextRef context);
void objectsStore(LCObjectRef objects[], size_t length, LCContextRef context);
void objectCache(LCObjectRef object);
void objectDeleteCache(LCObjectRef object);
void objectsSort(LCObjectRef objects[], size_t length);

char* typeName(LCTypeRef type);
bool typeImmutable(LCTypeRef type);
LCFormat typeSerializationFormat(LCTypeRef type);

LCStoreRef storeCreate(void *cookie, writeData writefn, deleteData deletefn, readData readfn);
LCContextRef contextCreate(LCStoreRef store, stringToType translateFuns[], size_t length);
LCTypeRef contextStringToType(LCContextRef context, char* typeString);

LCTypeRef coreStringToType(char* typeString);

void lcFree(void* object);

#endif
