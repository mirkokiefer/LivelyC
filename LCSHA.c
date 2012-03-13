
#include "LCSHA.h"

void computeSHA1(unsigned char data[], size_t length, LCByte buffer[]);

LCStringRef LCCreateSHAString(LCByte data[], size_t length) {
  LCByte sha[LC_HASH_BYTE_LENGTH];
  computeSHA1(data, length, sha);
  return createHexString(sha, LC_HASH_BYTE_LENGTH);
}

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
void computeSHA1(unsigned char data[], size_t length, LCByte buffer[]) {
  SHA_CTX context;
  SHA1_Init(&context);
  SHA1_Update(&context, data, length);
  SHA1_Final(buffer, &context);
}
#pragma GCC diagnostic warning "-Wdeprecated-declarations"

