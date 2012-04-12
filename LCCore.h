

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

typedef LCObjectRef LCStringRef;

typedef LCTypeRef(*stringToType)(char *typeString);

typedef void(*writeStreamFun)(void *cookie, LCByte data[], size_t length);
typedef void(*closeStreamFun)(void *cookie);

typedef LCObjectRef(*LCCreateEachCb)(LCInteger i, void* info, LCObjectRef each);

typedef FILE*(*writeData)(void *cookie, LCTypeRef type, char *key);
typedef void(*deleteData)(void *cookie, LCTypeRef type, char *key);
typedef FILE*(*readData)(void *cookie, LCTypeRef type, char *key);

typedef void (*callback)(void *cookie);
typedef void(*childCallback) (void *cookie, char *key, LCObjectRef objects[], size_t length, bool composite);

typedef void (*walkChildren)(LCObjectRef object, void *cookie, childCallback cb);
typedef void (*storeChildren)(LCObjectRef object, char *key, LCObjectRef objects[], size_t length);

/*
 - a type either implements serialize/deserializeData or walk/storeChildren but never both.
 - serializationFormat decides whether an object can be rendered as a composite or not
 - initData should return any data the object needs to start deserialization
 - dealloc should always release all child objects and free the objects data if possible
*/
struct LCType {
  char* name;
  bool immutable;
  LCFormat serializationFormat;
  void (*dealloc)(LCObjectRef object);
  LCCompare (*compare)(LCObjectRef object1, LCObjectRef object2);
  int (*serializeDataBuffered)(LCObjectRef object, fpos_t offset, size_t bufferLength, FILE *fd);
  void (*serializeData)(LCObjectRef object, FILE *fd);
  void* (*deserializeData)(LCObjectRef object, FILE *fd);
  void* (*initData)(void);
  walkChildren walkChildren;
  storeChildren storeChildren;
  void *meta;
};

LCObjectRef objectCreate(LCTypeRef type, void* data);
LCObjectRef objectCreateFromContext(LCContextRef context, LCTypeRef type, char hash[HASH_LENGTH]);
LCObjectRef objectCreateFromFile(LCContextRef context, FILE *fd);
void* objectData(LCObjectRef object);
LCTypeRef objectType(LCObjectRef object);
bool objectImmutable(LCObjectRef object);
bool objectsImmutable(LCObjectRef objects[], size_t length);
LCObjectRef objectRetain(LCObjectRef object);
LCObjectRef objectRelease(LCObjectRef object);
void objectReleaseAlt(void *object);
LCInteger objectRetainCount(LCObjectRef object);
LCCompare objectCompare(LCObjectRef object1, LCObjectRef object2);
LCContextRef objectContext(LCObjectRef object);
void objectSerializeToLevels(LCObjectRef object, LCInteger levels, FILE *fpw);
void objectSerializeAsComposite(LCObjectRef object, FILE *fpw);
void objectSerialize(LCObjectRef object, FILE* fd);
void objectSerializeBinaryData(LCObjectRef object, FILE *fd);
void objectDeserializeBinaryData(LCObjectRef object, FILE *fd);
void objectDeserialize(LCObjectRef object, FILE* fd);
void objectStoreChildren(LCObjectRef object, char *key, LCObjectRef objects[], size_t length);
void objectHash(LCObjectRef object, char hashBuffer[HASH_LENGTH]);
LCStringRef objectCreateHashString(LCObjectRef object);
void objectStore(LCObjectRef object, LCContextRef context);
void objectStoreAsComposite(LCObjectRef object, LCContextRef context);
void objectsStore(LCObjectRef objects[], size_t length, LCContextRef context);
void objectCache(LCObjectRef object);
void objectDeleteCache(LCObjectRef object, LCContextRef context);
void objectsSort(LCObjectRef objects[], size_t length);
bool objectHashEqual(LCObjectRef object1, LCObjectRef object2);
char* typeName(LCTypeRef type);
bool typeImmutable(LCTypeRef type);
LCFormat typeSerializationFormat(LCTypeRef type);
bool typeBinarySerialized(LCTypeRef type);

LCStoreRef storeCreate(void *cookie, writeData writefn, deleteData deletefn, readData readfn);
bool storeFileExists(LCStoreRef store, LCTypeRef type, char hash[HASH_LENGTH]);
FILE* storeWriteData(LCStoreRef store, LCTypeRef type, char hash[HASH_LENGTH]);
void storeDeleteData(LCStoreRef store, LCTypeRef type, char hash[HASH_LENGTH]);
FILE* storeReadData(LCStoreRef store, LCTypeRef type, char hash[HASH_LENGTH]);

LCContextRef contextCreate(LCStoreRef store, stringToType translateFuns[], size_t length);
LCTypeRef contextStringToType(LCContextRef context, char* typeString);

LCTypeRef coreStringToType(char* typeString);

void lcFree(void* object);

#endif
