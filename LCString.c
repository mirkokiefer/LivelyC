
#include "LCString.h"

LCCompare stringCompare(LCStringRef object1, LCStringRef object2);
void stringSerialize(LCStringRef object, FILE *fd);
void stringDeserialize(LCStringRef object, FILE *fd);


struct LCType stringType = {
  .compare = stringCompare
};

LCTypeRef LCTypeString = &stringType;

LCStringRef LCStringCreate(LCContextRef context, char *string) {
  char* stringData = malloc(strlen(string)+1);
  if (stringData) {
    LCStringRef stringObject = objectCreate(context, LCTypeString, NULL);
    strcpy(stringData, string);
    objectSetData(stringObject, stringData);
    return stringObject;
  }
  return NULL;
}

LCStringRef LCStringCreateFromHash(LCContextRef context, char hash[HASH_LENGTH]) {
  return objectCreate(context, LCTypeString, hash);
}

LCStringRef LCStringCreateFromChars(LCContextRef context, char* characters, size_t length) {
  char string[length+1];
  string[length] = '\0';
  memcpy(string, characters, length*sizeof(char));
  return LCStringCreate(context, string);
}

LCStringRef LCStringCreateFromStrings(LCContextRef context, LCStringRef strings[], size_t count) {
  size_t totalLength = 1;
  for (LCInteger i=0; i<count; i++) {
    totalLength = totalLength + LCStringLength(strings[i])-1;
  }
  char buffer[totalLength];
  buffer[0]='\0';
  for (LCInteger i=0; i<count; i++) {
    strcat(buffer, LCStringChars(strings[i]));
  }
  return LCStringCreate(context, buffer);
}

char* LCStringChars(LCStringRef string) {
  return objectGetData(string);
}

size_t LCStringLength(LCStringRef string) {
  char* chars = objectGetData(string);
  return strlen(chars);
}

bool LCStringEqual(LCStringRef string1, LCStringRef string2) {
  return stringCompare(string1, string2) == LCEqual;
}

LCCompare stringCompare(LCStringRef object1, LCStringRef object2) {
  char* string1 = objectGetData(object1);
  char* string2 = objectGetData(object2);
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
    return LCGreater;
  } else {
    return LCSmaller;
  }
}

void stringSerialize(LCStringRef object, FILE *fd) {
  char* stringData = objectGetData(object);
  fprintf(fd, "%s", stringData);
}

void stringDeserialize(LCStringRef object, FILE *fd) {
  
}