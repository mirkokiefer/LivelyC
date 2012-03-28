
#include "LivelyCTests.h"
#include "minuit.h"

void* arrayMap(LCInteger i, void* info, void* each);

int tests_run = 0;

static char* test_retain_counting() {
  LCStringRef test = LCStringCreate("abc");
  mu_assert("initial retain count=1", objectRetainCount(test)==1);
  objectRetain(test);
  mu_assert("retain increases retain count", objectRetainCount(test)==2);
  objectRelease(test);
  mu_assert("releasing decreases retain count", objectRetainCount(test)==1);
  objectRelease(test);
  return 0;
}

static char* test_pipe() {
  LCPipeRef stream = LCPipeCreate();
  FILE* fd = LCPipeWriteFile(stream);
  fprintf(fd, "123");
  fprintf(fd, "456789");
  fflush(fd);
  
  size_t length = LCPipeLength(stream);
  char buffer[length+1];
  buffer[length] = '\0';
  readFromFile(LCPipeReadFile(stream), (LCByte*)buffer, length);
  
  mu_assert("LCPipe read/write", strcmp("123456789", buffer)==0);
  return 0;
}

static char* test_memory_stream() {
  char *test = "abcdef";
  size_t testLength = strlen(test);
  FILE *read = createMemoryReadStream(NULL, (LCByte*)test, testLength, false, NULL);
  char readData[testLength];
  fscanf(read, "%s", readData);
  mu_assert("createMemoryReadStream", memcmp(test, readData, testLength)==0);
  return 0;
}


static char* test_string() {
  char* aCString = "abcd";
  char* anIdenticalCString = "abcd";
  char charArray[] = {'a', 'b', 'c', 'd'};
  LCStringRef aLCString = LCStringCreate(aCString);
  LCStringRef anIdenticalLCString = LCStringCreate(anIdenticalCString);
  LCStringRef stringFromCharArray = LCStringCreateFromChars(charArray, 4);
  
  mu_assert("LCStringEqual is correct", LCStringEqual(aLCString, anIdenticalLCString));
  mu_assert("LCStringCreateFromChars is correct", LCStringEqual(stringFromCharArray, aLCString));
  
  LCStringRef bothStrings = LCStringCreate("abcdabcd");
  LCStringRef stringArray[] = {aLCString, anIdenticalLCString};
  LCStringRef mergedString = LCStringCreateFromStrings(stringArray, 2);
  mu_assert("LCStringCreateFromStrings is correct", LCStringEqual(mergedString, bothStrings));
  
  LCStringRef tokenString = LCStringCreate("ab/cd/ef");
  LCArrayRef tokenArray = LCStringCreateTokens(tokenString, '/');
  LCStringRef* tokens = (LCStringRef*)LCArrayObjects(tokenArray);
  mu_assert("LCStringCreateTokens is correct", LCStringEqualCString(tokens[0], "ab") &&
            LCStringEqualCString(tokens[1], "cd") && LCStringEqualCString(tokens[2], "ef"));
    
  LCStringRef string1 = LCStringCreate("abcd");
  LCStringRef string2 = LCStringCreate("abcde");
  LCStringRef string3 = LCStringCreate("abd");
  mu_assert("LCStringCompare", (objectCompare(string1, string2) == LCSmaller) &&
            (objectCompare(string1, string3) == LCSmaller) &&
            (objectCompare(string1, string1) == LCEqual) &&
            (objectCompare(string2, string3) == LCSmaller));
  
  LCStringRef strings[] = {string2, string3, string1};
  objectsSort(strings, 3);
  mu_assert("string objectsSort", (strings[0]==string1) && (strings[1]==string2) && (strings[2]==string3));
  
  char *stringArr[] = {aCString, anIdenticalCString};
  LCStringRef concat = LCStringCreateFromStringArray(stringArr, 2);
  mu_assert("LCStringCreateFromStringArray", LCStringEqualCString(concat, "abcdabcd"));
  return 0;
}

void* arrayMap(LCInteger i, void* info, void* each) {
  LCStringRef string = (LCStringRef)each;
  char hash[HASH_LENGTH];
  objectHash(string, hash);
  return LCStringCreate(hash);
}

static char* test_array() {
  LCStringRef string1 = LCStringCreate("abc");
  LCStringRef string2 = LCStringCreate("def");
  LCStringRef string3 = LCStringCreate("ghi");
  LCStringRef stringArray[] = {string1, string2, string3};
  LCArrayRef array = LCArrayCreate(stringArray, 3);
  mu_assert("LCArray stores elements correctly",
            (LCArrayObjectAtIndex(array, 0)==string1) && (LCArrayObjectAtIndex(array, 1)==string2));
  
  LCArrayRef subArray = LCArrayCreateSubArray(array, 1, -1);
  LCArrayRef subArray1 = LCArrayCreateSubArray(array, 1, 2);
  mu_assert("LCArrayCreateSubArray(array, start, -1) is correct",
            (LCArrayObjectAtIndex(subArray, 0)==string2) && (LCArrayObjectAtIndex(subArray, 1)==string3));
  mu_assert("LCArrayCreateSubArray is correct",
            (LCArrayObjectAtIndex(subArray1, 0)==string2) && (LCArrayObjectAtIndex(subArray1, 1)==string3));
  
  LCArrayRef arrayAppend = LCArrayCreateAppendingObjects(array, stringArray, 3);
  LCObjectRef* arrayAppendObjs = LCArrayObjects(arrayAppend);
  mu_assert("LCArrayCreateAppendingObjects", (arrayAppendObjs[3] == string1) && (arrayAppendObjs[4] == string2) &&
            (arrayAppendObjs[5] == string3));
  
  
  LCMutableArrayRef mArray = LCMutableArrayCreate(stringArray, 3);
  mu_assert("LCMutableArrayCreate",
            (LCMutableArrayObjectAtIndex(mArray, 0)==string1) &&
            (LCMutableArrayObjectAtIndex(mArray, 1)==string2) &&
            (LCMutableArrayObjectAtIndex(mArray, 2)==string3));
  
  LCStringRef string4 = LCStringCreate("jkl");
  LCMutableArrayAddObject(mArray, string4);
  mu_assert("LCMutableArrayAddObject", LCMutableArrayObjectAtIndex(mArray, 3) == string4);
  
  for (LCInteger i=0; i<50; i++) {
    LCMutableArrayAddObject(mArray, string4);
  }
  mu_assert("LCMutableArrayAddObject 50 times", (LCMutableArrayObjectAtIndex(mArray, 50) == string4) &&
            (LCMutableArrayObjectAtIndex(mArray, 1) == string2));
  
  LCMutableArrayRemoveIndex(mArray, 1);
  mu_assert("LCMutableArrayRemoveIndex1", (LCMutableArrayObjectAtIndex(mArray, 0)==string1) &&
            (LCMutableArrayObjectAtIndex(mArray, 1)==string3) &&
            (LCMutableArrayObjectAtIndex(mArray, 2)==string4));
  LCMutableArrayRemoveIndex(mArray, 0);
  mu_assert("LCMutableArrayRemoveIndex2", LCMutableArrayObjectAtIndex(mArray, 0)==string3);
  
  LCMutableArrayRemoveObject(mArray, string3);
  mu_assert("LCMutableArrayRemoveObject", LCMutableArrayObjectAtIndex(mArray, 0)==string4);

  LCStringRef sortStrings[] = {string2, string3, string1};
  LCMutableArrayRef sortArray = LCMutableArrayCreate(sortStrings, 3);
  LCMutableArraySort(sortArray);
  LCStringRef* sorted = LCMutableArrayObjects(sortArray);
  mu_assert("LCMutableArraySort", (sorted[0] == string1) && (sorted[1] == string2) && (sorted[2] == string3));
  
  LCArrayRef arrays[] = {array, array};
  LCArrayRef mergedArray = LCArrayCreateFromArrays(arrays, 2);
  mu_assert("LCArrayCreateFromArrays", LCArrayLength(mergedArray)==2*LCArrayLength(array));
  
  LCArrayRef mappedArray = LCArrayCreateArrayWithMap(array, NULL, arrayMap);
  char string1Hash[HASH_LENGTH];
  objectHash(string1, string1Hash);
  mu_assert("LCArrayCreateArrayWithMap",
            LCStringEqualCString(LCArrayObjectAtIndex(mappedArray, 0), string1Hash));
  
  return 0;
}

static char* test_dictionary() {
  LCStringRef string1 = LCStringCreate("abc");
  LCStringRef string2 = LCStringCreate("def");
  LCStringRef string3 = LCStringCreate("ghi");
  LCStringRef string1c = LCStringCreate("abc");
  LCStringRef string2c = LCStringCreate("def");
  LCKeyValueRef kv1 = LCKeyValueCreate(string1, string2);
  LCKeyValueRef kv2 = LCKeyValueCreate(string2, string3);
  LCKeyValueRef kv3 = LCKeyValueCreate(string3, string1);
  LCKeyValueRef keyValues[] = {kv1, kv2, kv3};
  LCMutableDictionaryRef dict = LCMutableDictionaryCreate(keyValues, 3);

  mu_assert("LCMutableDictionaryCreate, LCMutableDictionaryValueForKey", (LCMutableDictionaryValueForKey(dict, string1c) == string2) &&
            (LCMutableDictionaryValueForKey(dict, string2c) == string3) &&
            (LCMutableDictionaryValueForKey(dict, string3) == string1));
  
  LCMutableDictionaryDeleteKey(dict, string2c);
  mu_assert("LCMutableDictionaryDeleteKey", LCMutableDictionaryValueForKey(dict, string2c) == NULL);
  
  LCMutableDictionarySetValueForKey(dict, string1c, string1);
  mu_assert("LCMutableDictionarySetValueForKey", LCMutableDictionaryValueForKey(dict, string1) == string1);
  return 0;
}

static char* test_sha1() {
  char* testData1 = "compute sha1";
  char* realHash = "eefbec885d1042d22ea36fd1690d94dec9029680";
  
  char computedHash[HASH_LENGTH];
  createSHAString((LCByte*)testData1, strlen(testData1), computedHash);
  
  mu_assert("SHA1 from testData1 is correct", strcmp(realHash, computedHash) == 0);
  return 0;
}

static char* test_data() {
  char* aCString = "123456";
  
  LCDataRef aData = LCDataCreate((LCByte*)aCString, strlen(aCString)+1);
  LCByte* dataFromLCData = LCDataDataRef(aData);
  
  mu_assert("LCData stores data correctly", strcmp(aCString, (char*)dataFromLCData)==0);
  
  LCMutableDataRef mData = LCMutableDataCreate((LCByte*)aCString, strlen(aCString));
  char *string2 = "78910";
  LCMutableDataAppend(mData, (LCByte*)string2, strlen(string2));
  LCMutableDataAppend(mData, (LCByte*)"\0", 1);
  
  char *string3 = "12345678910";
  mu_assert("LCMutableDataAppend", strcmp((char*)LCMutableDataDataRef(mData), string3)==0);
  
  LCMutableDataRef mData2 = LCMutableDataCreate(NULL, 0);
  FILE *fp = createMemoryReadStream(NULL, (LCByte*)string3, strlen(string3)+1, false, NULL);
  LCMutableDataAppendFromFile(mData2, fp, -1);
  LCMutableDataAppend(mData2, (LCByte*)"\0", 1);
  mu_assert("LCMutableDataAppendFromFile", strcmp((char*)LCMutableDataDataRef(mData2), string3)==0);
  return 0;
}

static char* test_object_persistence_with_store(LCStoreRef store, char *storeType) {
  LCContextRef context = contextCreate(store, NULL, 0);
  
  mu_assert("contextStringToType", contextStringToType(context, "LCString") == LCTypeString);
  
  char* string = "abcdef";
  LCStringRef test = LCStringCreate(string);
  objectStore(test, context);
  objectDeleteCache(test);
  mu_assert("string persistence", LCStringEqualCString(test, string));
  
  LCStringRef string1 = LCStringCreate("abc");
  LCStringRef string2 = LCStringCreate("def");
  LCStringRef string3 = LCStringCreate("ghi");
  LCStringRef stringArray[] = {string1, string2, string3};
  LCArrayRef array = LCArrayCreate(stringArray, 3);
  objectStore(array, context);
  objectDeleteCache(array);
  LCStringRef *strings = LCArrayObjects(array);
  mu_assert("array persistence", LCStringEqual(string1, strings[0]) && LCStringEqual(string2, strings[1]) &&
            LCStringEqual(string3, strings[2]));
  
  LCKeyValueRef keyValue = LCKeyValueCreate(string1, array);
  objectStore(keyValue, context);
  objectDeleteCache(keyValue);
  objectCache(keyValue);
  mu_assert("keyValue persistence", LCStringEqual(LCKeyValueKey(keyValue), string1));
  
  LCArrayRef mArray = LCMutableArrayCreate(stringArray, 3);
  objectStore(mArray, context);
  objectDeleteCache(mArray);
  LCMutableArrayAddObject(mArray, string1);
  objectStore(mArray, context);
  objectDeleteCache(mArray);
  objectCache(mArray);
  LCStringRef *strings1 = LCMutableArrayObjects(mArray);
  mu_assert("mutable array persistence", LCStringEqual(string1, strings1[0]) && LCStringEqual(string2, strings1[1]) &&
            LCStringEqual(string3, strings1[2]) && LCStringEqual(string1, strings1[3]));
  
  LCMutableArrayAddObject(mArray, LCStringCreate("test1"));
  objectStoreAsComposite(mArray, context);
  objectDeleteCache(mArray);
  LCStringRef *strings2 = LCMutableArrayObjects(mArray);
  mu_assert("composite persistence", LCStringEqual(string1, strings2[0]) && LCStringEqual(string2, strings2[1]) &&
            LCStringEqual(string3, strings2[2]) && LCStringEqual(string1, strings2[3]));

  return 0;
}

static char* test_object_persistence() {
  LCMemoryStoreRef store = LCMemoryStoreCreate();
  char *memoryTest = test_object_persistence_with_store(LCMemoryStoreStoreObject(store), "memory");
  if (memoryTest) {
    return memoryTest;
  }
  
  LCStringRef homeFolder = getHomeFolder();
  char *strings[] = {LCStringChars(homeFolder), "/testing/"};
  LCStringRef testPath = LCStringCreateFromStringArray(strings, 2);
  LCFileStoreRef fileStore = LCFileStoreCreate(LCStringChars(testPath));
  
  char *fileTest = test_object_persistence_with_store(LCFileStoreStoreObject(fileStore), "file");
  deleteDirectory(LCStringChars(testPath));
  return fileTest;
}

static char* all_tests() {
  mu_run_test(test_retain_counting);
  mu_run_test(test_pipe);
  mu_run_test(test_memory_stream);
  mu_run_test(test_string);
  mu_run_test(test_array);
  mu_run_test(test_dictionary);
  mu_run_test(test_sha1);
  mu_run_test(test_data);
  mu_run_test(test_object_persistence);
  return 0;
}

bool testsRun() {
  char *result = all_tests();
  if (result != 0) {
    printf("%s\n", result);
  }
  else {
    printf("ALL TESTS PASSED\n");
  }
  printf("Tests run: %d\n", tests_run);
  
  return result == 0;
}
