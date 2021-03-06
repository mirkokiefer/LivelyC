
#include "LCString.h"
#include "LCUtils.h"
#include "LCMutableData.h"
#include "LCMemoryStream.h"

LCCompare stringCompare(LCStringRef object1, LCStringRef object2);
void stringSerialize(LCObjectRef object, FILE *fd);
void* stringDeserialize(LCObjectRef object, FILE* fd);


struct LCType stringType = {
  .name = "LCString",
  .immutable = true,
  .serializationFormat = LCText,
  .compare = stringCompare,
  .serializeData = stringSerialize,
  .deserializeData = stringDeserialize
};

LCTypeRef LCTypeString = &stringType;

LCStringRef LCStringCreate(char *string) {
  char* stringData = malloc(strlen(string)+1);
  if (stringData) {
    strcpy(stringData, string);
    LCStringRef stringObject = objectCreate(LCTypeString, stringData);
    return stringObject;
  }
  return NULL;
}

LCStringRef LCStringCreateFromHash(LCContextRef context, char hash[HASH_LENGTH]) {
  return objectCreateFromContext(context, LCTypeString, hash);
}

LCStringRef LCStringCreateFromChars(char* characters, size_t length) {
  char string[length+1];
  string[length] = '\0';
  memcpy(string, characters, length*sizeof(char));
  return LCStringCreate(string);
}

LCStringRef LCStringCreateFromStringsWithDelim(LCStringRef strings[], size_t length, char *delimiter) {
  char *stringArray[length];
  for (LCInteger i=0; i<length; i++) {
    stringArray[i] = LCStringChars(strings[i]);
  }
  return LCStringCreateFromStringArrayWithDelim(stringArray, length, delimiter);
}

LCStringRef LCStringCreateFromStrings(LCStringRef strings[], size_t length) {
  return LCStringCreateFromStringsWithDelim(strings, length, NULL);
}

LCStringRef LCStringCreateFromStringArrayWithDelim(char* strings[], size_t length, char *delimiter) {
  size_t totalLength = 1;
  for (LCInteger i=0; i<length; i++) {
    totalLength = totalLength + strlen(strings[i]);
  }
  if (delimiter) {
    totalLength = totalLength + (length-1)*strlen(delimiter);
  }
  char buffer[totalLength];
  buffer[0]='\0';
  for (LCInteger i=0; i<length; i++) {
    if (i>0 && delimiter) {
      strcat(buffer, delimiter);
    }
    strcat(buffer, strings[i]);
  }
  return LCStringCreate(buffer);
}

LCStringRef LCStringCreateFromStringArray(char* strings[], size_t length) {
  return LCStringCreateFromStringArrayWithDelim(strings, length, NULL);
}

LCStringRef LCStringCreateFromData(LCDataRef data) {
  return LCStringCreateFromChars((char*)LCDataDataRef(data), LCDataLength(data));
}

char* LCStringChars(LCStringRef string) {
  return objectData(string);
}

size_t LCStringLength(LCStringRef string) {
  char* chars = objectData(string);
  return strlen(chars);
}

bool LCStringEqual(LCStringRef string1, LCStringRef string2) {
  return stringCompare(string1, string2) == LCEqual;
}

bool LCStringEqualCString(LCStringRef string, char* cString) {
  char* chars = objectData(string);
  return strcmp(chars, cString) == 0;
}

LCArrayRef LCStringCreateTokens(LCStringRef string, char delimiter) {
  char* cString = LCStringChars(string);
  LCStringRef substrings[strlen(cString)/2];
  LCInteger tokenStart = 0;
  LCInteger substringCount = 0;
  for (LCInteger i=0; i<strlen(cString); i++) {
    if(cString[i] == delimiter) {
      substrings[substringCount] = LCStringCreateFromChars(&cString[tokenStart], i-tokenStart);
      substringCount = substringCount + 1;
      tokenStart = i+1;
    }
  }
  substrings[substringCount] = LCStringCreate(&cString[tokenStart]);
  substringCount = substringCount + 1;
  LCArrayRef array = LCArrayCreate(substrings, substringCount);
  for(LCInteger i=0; i<substringCount; i++) {
    objectRelease(substrings[i]);
  }
  return array;
}

LCCompare stringCompare(LCStringRef object1, LCStringRef object2) {
  char* string1 = objectData(object1);
  char* string2 = objectData(object2);
  LCInteger string1Length = strlen(string1);
  LCInteger string2Length = strlen(string2);
  LCInteger maxCompareLength = string1Length;
  if (maxCompareLength > string2Length) {
    maxCompareLength = string2Length;
  }
  
  for (LCInteger i=0; i<maxCompareLength; i++) {
    if (string1[i] > string2[i]) {
      return LCGreater;
    }
    if (string1[i] < string2[i]) {
      return LCSmaller;
    }
  }
  if (string1Length == string2Length) {
    return LCEqual;
  }
  if (string1Length < string2Length) {
    return LCSmaller;
  } else {
    return LCGreater;
  }
}

void stringSerialize(LCObjectRef object, FILE *fp) {
  fprintf(fp, "%s", LCStringChars(object));
}

void* stringDeserialize(LCStringRef object, FILE *fp) {
  LCMutableDataRef data = LCMutableDataCreate(NULL, 0);
  LCMutableDataAppendFromFile(data, fp, fileLength(fp));
  char* serializedString = (char*)LCMutableDataDataRef(data);
  char* buffer = malloc(sizeof(char)*strlen(serializedString));
  if (buffer) {
    strcpy(buffer, serializedString);
    objectRelease(data);
    return buffer;
  } else {
    return NULL;
  }
}
