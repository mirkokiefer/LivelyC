
#include "LCSHA.h"

void computeSHA1(unsigned char data[], size_t length, LCByte buffer[]);

struct serializationCookie {
  LCPipeRef stream;
  SHA_CTX context;
};

typedef struct serializationCookie* serializationCookieRef;

void createSHAString(LCByte data[], size_t length, char buffer[HASH_LENGTH]) {
  LCByte sha[LC_HASH_BYTE_LENGTH];
  computeSHA1(data, length, sha);
  createHexString(sha, LC_HASH_BYTE_LENGTH, buffer);
}

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
void* createHashContext(LCPipeRef stream) {
  SHA_CTX *context = malloc(sizeof(SHA_CTX));
  if (context) {
    SHA1_Init(context);
  }
  return context;
}

void updateHashContext(void* cookie, LCByte data[], size_t length) {
  SHA_CTX *context = (SHA_CTX*)cookie;
  SHA1_Update(context, data, length);
}

void finalizeHashContext(void* cookie, char buffer[HASH_LENGTH]) {
  SHA_CTX *context = (SHA_CTX*)cookie;
  LCByte byteBuffer[LC_HASH_BYTE_LENGTH];
  SHA1_Final(byteBuffer, context);
  createHexString(byteBuffer, LC_HASH_BYTE_LENGTH, buffer);
}

void computeSHA1(unsigned char data[], size_t length, LCByte buffer[]) {
  SHA_CTX context;
  SHA1_Init(&context);
  SHA1_Update(&context, data, length);
  SHA1_Final(buffer, &context);
}
#pragma GCC diagnostic warning "-Wdeprecated-declarations"

