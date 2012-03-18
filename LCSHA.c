
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
  serializationCookieRef cookie = malloc(sizeof(struct serializationCookie));
  if (cookie) {
    cookie->stream = stream;
    SHA1_Init(&(cookie->context));
  }
  return cookie;
}

void updateHashContext(void* context) {
  serializationCookieRef cookie = (serializationCookieRef)context;
  fflush(LCPipeWriteFile(cookie->stream));
  size_t length = LCPipeLength(cookie->stream);
  LCByte buffer[length];
  LCPipeData(cookie->stream, buffer, length);
  SHA1_Update(&(cookie->context), buffer, length);
}

void finalizeHashContext(void* context, char buffer[HASH_LENGTH]) {
  serializationCookieRef cookie = (serializationCookieRef)context;
  updateHashContext(context);
  LCByte byteBuffer[LC_HASH_BYTE_LENGTH];
  SHA1_Final(byteBuffer, &(cookie->context));
  createHexString(byteBuffer, LC_HASH_BYTE_LENGTH, buffer);
}

void computeSHA1(unsigned char data[], size_t length, LCByte buffer[]) {
  SHA_CTX context;
  SHA1_Init(&context);
  SHA1_Update(&context, data, length);
  SHA1_Final(buffer, &context);
}
#pragma GCC diagnostic warning "-Wdeprecated-declarations"

