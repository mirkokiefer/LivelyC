
#ifndef LivelyC_macbook_JsonSerialization_h
#define LivelyC_macbook_JsonSerialization_h

#include "LCCore.h"

void objectSerializeJson(LCObjectRef object, bool composite, FILE *fp);
void objectDeserializeJson(LCObjectRef object, json_value *json);

#endif
