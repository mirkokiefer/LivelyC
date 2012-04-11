
#ifndef LivelyC_macbook_JsonSerialization_h
#define LivelyC_macbook_JsonSerialization_h

#include "LCCore.h"

void objectSerializeJsonToLevels(LCObjectRef object, LCInteger levels, FILE *fp, walkChildren walkFun);
void objectSerializeJson(LCObjectRef object, bool composite, FILE *fp, walkChildren walkFun);
void objectDeserializeJson(LCObjectRef object, json_value *json, storeChildren storeFun);

#endif
