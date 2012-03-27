
#include "LCFileStore.h"
#include "LCString.h"
#include "LCUtils.h"

FILE* fileStoreWrite(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);
void fileStoreDelete(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);
FILE* fileStoreRead(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]);
void fileStoreDealloc(LCObjectRef store);
void* fileStoreInitData();
LCStringRef createDirectoryPath(LCFileStoreRef store, LCTypeRef type);
LCStringRef createFilePath(LCFileStoreRef store, LCTypeRef type, char *hash);

typedef struct fileStoreData* fileStoreDataRef;

struct fileStoreData {
  LCStringRef location;
};

struct LCType typeFileStore = {
  .initData = fileStoreInitData,
  .dealloc = fileStoreDealloc
};

LCTypeRef LCTypeFileStore = &typeFileStore;

void* fileStoreInitData() {
  fileStoreDataRef data = malloc(sizeof(struct fileStoreData));
  if (data) {
    data->location = NULL;
  }
  return data;
}

LCFileStoreRef LCFileStoreCreate(char *location) {
  fileStoreDataRef data = fileStoreInitData();
  data->location = LCStringCreate(location);
  makeDirectory(location);
  return objectCreate(LCTypeFileStore, data);
}

static char* fileStoreLocation(LCFileStoreRef store) {
  fileStoreDataRef data = objectData(store);
  return LCStringChars(data->location);
}

LCStoreRef LCFileStoreStoreObject(LCFileStoreRef store) {
  return storeCreate(store, fileStoreWrite, fileStoreDelete, fileStoreRead);
}

FILE* fileStoreWrite(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]) {
  LCStringRef directoryPath = createDirectoryPath(cookie, type);
  makeDirectory(LCStringChars(directoryPath));
  LCStringRef filePath = createFilePath(cookie, type, hash);
  FILE *fp = fopen(LCStringChars(filePath), "w");
  objectRelease(filePath);
  return fp;
}

void fileStoreDelete(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]) {
  // TODO
}

FILE* fileStoreRead(void *cookie, LCTypeRef type, char hash[HASH_LENGTH]) {
  LCStringRef filePath = createFilePath(cookie, type, hash);
  FILE *fp = fopen(LCStringChars(filePath), "r");
  objectRelease(filePath);
  return fp;
}

void fileStoreDealloc(LCObjectRef store) {
  fileStoreDataRef data = objectData(store);
  objectRelease(data->location);
  lcFree(data);
}

LCStringRef createDirectoryPath(LCFileStoreRef store, LCTypeRef type) {
  char *storeLocation = fileStoreLocation(store);
  char *typeString = typeName(type);
  char *strings[] = {storeLocation, typeString, "/"};
  return LCStringCreateFromStringArray(strings, 3);
}

LCStringRef createFilePath(LCFileStoreRef store, LCTypeRef type, char *hash) {
  LCStringRef directory = createDirectoryPath(store, type);
  char *strings[] = {LCStringChars(directory), hash, ".txt"};
  LCStringRef path = LCStringCreateFromStringArray(strings, 3);
  objectRelease(directory);
  return path;
}
