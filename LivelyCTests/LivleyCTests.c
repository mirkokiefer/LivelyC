
#include "LivelyCTests.h"
#include "minuit.h"

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

static char* test_memory_stream() {
  LCMemoryStreamRef stream = LCMemoryStreamCreate(NULL);
  FILE* fd = LCMemoryStreamWriteFile(stream);
  fprintf(fd, "123");
  fprintf(fd, "456789");
  fflush(fd);
  
  size_t length = LCMemoryStreamLength(stream);
  char buffer[length+1];
  buffer[length] = '\0';
  readFromFile(LCMemoryStreamReadFile(stream), (LCByte*)buffer, length);
  
  mu_assert("LCMemoryStream read/write", strcmp("123456789", buffer)==0);
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
  mu_assert("LCStringCompare", (objectCompare(string1, string2) == LCGreater) &&
            (objectCompare(string1, string3) == LCSmaller) &&
            (objectCompare(string1, string1) == LCEqual) &&
            (objectCompare(string2, string3) == LCSmaller));
  
  LCStringRef strings[] = {string1, string2, string3};
  objectsSort(strings, 3);
  return 0;
}

static char* all_tests() {
  mu_run_test(test_retain_counting);
  mu_run_test(test_memory_stream);
  mu_run_test(test_string);
  
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
