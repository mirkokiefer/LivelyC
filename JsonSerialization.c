
#include "JsonSerialization.h"
#include "LCMemoryStream.h"
#include "LCMutableData.h"
#include "LCUtils.h"

LCObjectRef objectCreateFromJson(json_value *json, LCContextRef context);

struct LCSerializationCookie {
  FILE *fp;
  LCObjectRef object;
  bool first;
  LCInteger levels;
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
    bool serializedAsText = objectType(objects[i])->serializationFormat == LCText;
    if ((info->levels != 0) && serializedAsText) {
      if (info->levels == -1) {
        objectSerializeAsComposite(objects[i], info->fp);
      } else {
        objectSerializeToLevels(objects[i], info->levels-1, info->fp);
      }
    } else {
      char hash[HASH_LENGTH];
      objectHash(objects[i], hash);
      fprintf(info->fp, "\{\"type\": \"%s\", \"hash\": \"%s\"}", typeName(objectType(objects[i])), hash);
    }
  }
  fprintf(info->fp, "]");
}

void objectSerializeTextToJson(LCObjectRef object, FILE *fpw) {
  fprintf(fpw, "\{\"type\": \"%s\", \"data\": \"", typeName(objectType(object)));
  objectSerializeBinaryData(object, fpw);
  fprintf(fpw, "\"}");
}

void objectSerializeJsonToLevels(LCObjectRef object, LCInteger levels, FILE *fpw, walkChildren walkFun) {
  struct LCSerializationCookie cookie = {
    .fp = fpw,
    .object = object,
    .first = true,
    .levels = levels
  };
  fprintf(fpw, "\{\"type\": \"%s\", \"data\": {", typeName(objectType(object)));
  walkFun(object, &cookie, serializeChildCallback);
  fprintf(fpw, "}}");
}

void objectSerializeJson(LCObjectRef object, bool composite, FILE *fpw, walkChildren walkFun) {
  if (composite) {
    objectSerializeJsonToLevels(object, -1, fpw, walkFun);
  } else {
    objectSerializeJsonToLevels(object, 0, fpw, walkFun);
  }
}

static void objectDeserializeBinaryDataFromJson(LCObjectRef object, json_value *children) {
  char *dataStringJson = children->u.string.ptr;
  objectDeserializeBinaryData(object, createMemoryReadStream(NULL, (LCByte*)dataStringJson, strlen(dataStringJson)+1, false, NULL));
}

static void objectDeserializeObjectDataFromJson(LCObjectRef object, json_value *json) {
  LCContextRef context = objectContext(object);
  for (LCInteger i=0; i<json->u.object.length; i++) {
    char *key = json->u.object.values[i].name;
    json_value *value = json->u.object.values[i].value;
    json_value **objectsInfo = value->u.array.values;
    
    size_t objectsLength = value->u.array.length;
    LCObjectRef objects[objectsLength];
    
    for (LCInteger j=0; j<objectsLength; j++) {
      json_value *objectInfo = objectsInfo[j];
      objects[j] = objectCreateFromJson(objectInfo, context);
    }
    
    objectStoreChildren(object, key, objects, objectsLength);
    for (LCInteger j=0; j<objectsLength; j++) {
      objectRelease(objects[j]);
    }
  }
}

static void objectDeserializeDataFromJson(LCObjectRef object, json_value *json) {
  if (typeBinarySerialized(objectType(object))) {
    objectDeserializeBinaryDataFromJson(object, json);
  } else {
    objectDeserializeObjectDataFromJson(object, json);
  }
}

static json_value* fileToJson(FILE *fd) {
  LCMutableDataRef data = LCMutableDataCreate(NULL, 0);
  LCMutableDataAppendFromFile(data, fd, fileLength(fd));
  LCMutableDataAppend(data, (LCByte*)"\0", 1);
  char *jsonString = (char*)LCMutableDataDataRef(data);
  json_value *json = json_parse(jsonString);
  objectRelease(data);
  return json;
}

LCObjectRef objectCreateFromJsonFile(FILE *fd, LCContextRef context) {
  json_value *json = fileToJson(fd);
  LCObjectRef object = objectCreateFromJson(json, context);
  json_value_free(json);
  return object;
}

LCObjectRef objectCreateFromJson(json_value *json, LCContextRef context) {
  char *typeString;
  char *hash = NULL;
  json_value *children = NULL;
  for (LCInteger k=0; k<json->u.object.length; k++) {
    char* objectInfoKey = json->u.object.values[k].name;
    json_value *objectInfoValue = json->u.object.values[k].value;
    if (strcmp(objectInfoKey, "type")==0) {
      typeString = objectInfoValue->u.string.ptr;
    } else if(strcmp(objectInfoKey, "hash")==0) {
      hash = objectInfoValue->u.string.ptr;
    } else if (strcmp(objectInfoKey, "data")==0) {
      children = objectInfoValue;
    }
  }
  LCObjectRef object;
  if (hash) {
    object = objectCreateFromContext(context, contextStringToType(context, typeString), hash);
  } else if(children) {
    object = objectCreate(contextStringToType(context, typeString), NULL);
    objectDeserializeDataFromJson(object, children);
  }
  return object;
}

void objectDeserializeJsonFile(LCObjectRef object, FILE *fd) {
  json_value *json = fileToJson(fd);
  json_value *children = NULL;
  for (LCInteger k=0; k<json->u.object.length; k++) {
    char* objectInfoKey = json->u.object.values[k].name;
    json_value *objectInfoValue = json->u.object.values[k].value;
    if (strcmp(objectInfoKey, "data")==0) {
      children = objectInfoValue;
    }
  }
  objectDeserializeDataFromJson(object, children);
  json_value_free(json);
}
