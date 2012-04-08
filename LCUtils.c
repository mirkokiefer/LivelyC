
#include "LCUtils.h"
#include "LCPipe.h"
#include "url_open.h"

#define READ_BUFFER_SIZE 1024

void LCPrintf(LCObjectRef object) {
  objectSerialize(object, stdout);
}

char hexDigitToASCIChar(char hexDigit) {
  if(hexDigit > 9) {
    hexDigit = hexDigit - 10 + 97; //97 is A
  } else {
    hexDigit = hexDigit + 48; //48 is 0
  }
  return hexDigit;
}

char asciCharToHexDigit(char asciChar) {
  if (asciChar >= 97) { //A = 97
    return asciChar - 97 + 10; // A -> 10, B -> 11...
  } else {
    return asciChar - 48; // if a number char (48=char 0)
  }
}

void byteToHexDigits(LCByte input, char* buffer) {
  buffer[0] = hexDigitToASCIChar(input/16);
  buffer[1] = hexDigitToASCIChar(input%16);
}

LCByte hexDigitsToByte(char* hexDigits) {
  return hexDigits[0]*16 + hexDigits[1];
}

void createHexString(LCByte data[], size_t length, char buffer[]) {
  for(LCInteger i=0; i<length; i++) {
    byteToHexDigits(data[i], &buffer[i*2]);
  }
  buffer[length*2] = '\0';
}

LCDataRef createDataFromHexString(LCStringRef hexString) {
  LCByte buffer[LC_HASH_BYTE_LENGTH];
  for (LCInteger i=0; i<LC_HASH_BYTE_LENGTH; i++) {
    char digit1 = LCStringChars(hexString)[i*2];
    char digit2 = LCStringChars(hexString)[(i*2)+1];
    char hexDigits[] = {asciCharToHexDigit(digit1), asciCharToHexDigit(digit2)};
    buffer[i] = hexDigitsToByte(hexDigits);
  }
  return LCDataCreate(buffer, LC_HASH_BYTE_LENGTH);
}

LCArrayRef createPathArray(LCStringRef path) {
  return LCStringCreateTokens(path, '/');
}

void writeToFile(LCByte data[], size_t length, char* filePath) {
  FILE *fp = fopen(filePath, "w");
  fwrite(data, sizeof(unsigned char), length, fp);
  fclose(fp);
}

size_t fileLength(FILE *fp) {
  struct stat stat;
  int err = fstat(fileno(fp), &stat);
  if (err) {
    size_t currentPos = ftell(fp);
    if (currentPos == -1) {
      return -1;
    } else {
      if (fseek(fp, 0, SEEK_END) == -1) {
        return -1;
      } else {
        size_t endPos = ftell(fp);
        fseek(fp, currentPos, SEEK_SET);
        return endPos-currentPos;
      }
    }
  } else {
    return stat.st_size / sizeof(LCByte);
  }
}

void readFromFile(FILE *fp, LCByte buffer[], size_t length) {
  fread(buffer, sizeof(LCByte), length, fp);
}

int makeDirectory(char* path) {
  struct stat sb;
  if (stat(path, &sb) != 0) {
    return mkdir(path, S_IRWXU);
  } else {
    return 0;
  }
}

static int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  int rv = remove(fpath);
  
  if (rv)
    perror(fpath);
  
  return rv;
}

int deleteDirectory(char *path) {
  return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

LCStringRef getHomeFolder() {
  struct passwd *passwdEnt = getpwuid(getuid());
  return LCStringCreate(passwdEnt->pw_dir);
}

int pipeFiles(FILE *read, FILE *write, size_t bufferLength) {
  LCByte buffer[bufferLength];
  while (!feof(read) && !ferror(read)) {
    size_t lengthRead = fread(buffer, sizeof(LCByte), bufferLength, read);
    fwrite(buffer, sizeof(LCByte), lengthRead, write);
  }
  return 0;
}

int pipeFileToFunction(void *cookie, FILE *read, writeStreamFun writeFun, size_t bufferLength) {
  LCByte buffer[bufferLength];
  while (!feof(read) && !ferror(read)) {
    size_t lengthRead = fread(buffer, sizeof(LCByte), bufferLength, read);
    writeFun(cookie, buffer, lengthRead);
  }
  return 0; 
}

int pipeURLToFile(URL_FILE *read, FILE *write, size_t bufferLength) {
  LCByte buffer[bufferLength];
  while (!url_feof(read)) {
    size_t lengthRead = url_fread(buffer, sizeof(LCByte), bufferLength, read);
    fwrite(buffer, sizeof(LCByte), lengthRead, write);
  }
  return 0;
}
