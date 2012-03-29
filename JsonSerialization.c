
#include "JsonSerialization.h"
#include "LCMemoryStream.h"

struct LCSerializationCookie {
  FILE *fp;
  LCObjectRef object;
  bool first;
  bool composite;
};

static void serializeChildCallback(void *cookie, char *key, LCObjectRef objects[], size_t length, bool composite) {
  struct LCSerializationCookie *info = (struct LCSerializationCookie*)cookie;
  if (info->first) {
    info->first = false;
  } else {
    fprintf(info->fp, ",\n");
  }
  fprintf(info->fp, "\"%s\": [", key);
  for (LCInteger i=0; i<length; i++) {
    if (objects[i] == NULL) {
      continue;
    }
    if (i>0) {
      fprintf(info->fp, ",");
    }
    fprintf(info->fp, "\{\"type\": \"%s\", ", typeName(objectType(objects[i])));
    if (info->composite) {
      fprintf(info->fp, "\"object\": ");
      objectSerializeAsComposite(objects[i], info->fp);
    } else {
      char hash[HASH_LENGTH];
      objectHash(objects[i], hash);
      fprintf(info->fp, "\"hash\": \"%s\"", hash);
    }
    fprintf(info->fp, "}");
  }
  fprintf(info->fp, "]");
}

void objectSerializeJson(LCObjectRef object, bool composite, FILE *fpw, walkChildren walkFun) {
  struct LCSerializationCookie cookie = {
    .fp = fpw,
    .object = object,
    .first = true,
    .composite = composite
  };
  fprintf(fpw, "{");
  walkFun(object, &cookie, serializeChildCallback);
  fprintf(fpw, "}");
}

void objectDeserializeJson(LCObjectRef object, json_value *json, storeChildren storeFun) {
  LCContextRef context = objectContext(object);
  for (LCInteger i=0; i<json->u.object.length; i++) {
    char *key = json->u.object.values[i].name;
    json_value *value = json->u.object.values[i].value;
    json_value **objectsInfo = value->u.array.values;
    
    size_t objectsLength = value->u.array.length;
    LCObjectRef objects[objectsLength];
    
    for (LCInteger j=0; j<objectsLength; j++) {
      json_value *objectInfo = objectsInfo[j];
      char *typeString;
      char *hash = NULL;
      json_value *embeddedObject = NULL;
      for (LCInteger k=0; k<objectInfo->u.object.length; k++) {
        char* objectInfoKey = objectInfo->u.object.values[k].name;
        json_value *objectInfoValue = objectInfo->u.object.values[k].value;
        if (strcmp(objectInfoKey, "type")==0) {
          typeString = objectInfoValue->u.string.ptr;
        } else if(strcmp(objectInfoKey, "hash")==0) {
          hash = objectInfoValue->u.string.ptr;
        } else if (strcmp(objectInfoKey, "object")==0) {
          embeddedObject = objectInfoValue;
        }
      }
      if (hash) {
        objects[j] = objectCreateFromContext(context, contextStringToType(context, typeString), hash);
      } else if(embeddedObject) {
        objects[j] = objectCreate(contextStringToType(context, typeString), NULL);
        if (typeBinarySerialized(objectType(objects[j]))) {
          char *dataString = embeddedObject->u.string.ptr;
          size_t dataStringLength = embeddedObject->u.string.length;
          char dataStringJson[dataStringLength+2];
          sprintf(dataStringJson, "\"%s\"", dataString);
          objectDeserialize(objects[j], createMemoryReadStream(NULL, (LCByte*)dataStringJson, dataStringLength+2, false, NULL));
        } else {
          objectDeserializeJson(objects[j], embeddedObject, storeFun);
        }
      }
    }
    storeFun(object, key, objects, objectsLength);
    for (LCInteger j=0; j<objectsLength; j++) {
      objectRelease(objects[j]);
    }
  }
}
