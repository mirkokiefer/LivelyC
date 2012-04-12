
#ifndef LivelyC_macbook_JsonSerialization_h
#define LivelyC_macbook_JsonSerialization_h

#include "LCCore.h"

void objectSerializeTextToJson(LCObjectRef object, FILE *fpw);
void objectSerializeJsonToLevels(LCObjectRef object, LCInteger levels, FILE *fp, walkChildren walkFun);
void objectSerializeJson(LCObjectRef object, bool composite, FILE *fp, walkChildren walkFun);
LCObjectRef objectCreateFromJsonFile(FILE *fd, LCContextRef context);
void objectDeserializeJsonFile(LCObjectRef object, FILE *fd);

#endif
