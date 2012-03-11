
#include "LivelyCTests.h"
#include "minuit.h"

int tests_run = 0;

static char* test_retain_counting() {
  LCStringRef test = LCStringCreate(NULL, "abc");
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
  FILE* fd = LCMemoryStreamFile(stream);
  fprintf(fd, "123");
  fprintf(fd, "456789");
  fclose(fd);
  FILE* fd2 = LCMemoryStreamFile(stream);
  char buffer[10];
  fscanf(fd2, "%s", buffer);
  mu_assert("LCMemoryStream read/write", strcmp("123456789", buffer)==0);
  
  fseek(fd2, -4, SEEK_END);
  char buffer1[5];
  fscanf(fd2, "%s", buffer1);
  fclose(fd2);
  mu_assert("LCMemoryStream seek", strcmp("6789", buffer1)==0);

  return 0;
}

static char* test_string() {
  char* aCString = "abcd";
  char* anIdenticalCString = "abcd";
  char charArray[] = {'a', 'b', 'c', 'd'};
  LCStringRef aLCString = LCStringCreate(NULL, aCString);
  LCStringRef anIdenticalLCString = LCStringCreate(NULL, anIdenticalCString);
  LCStringRef stringFromCharArray = LCStringCreateFromChars(NULL, charArray, 4);
  
  mu_assert("LCStringEqual is correct", LCStringEqual(aLCString, anIdenticalLCString));
  mu_assert("LCStringCreateFromChars is correct", LCStringEqual(stringFromCharArray, aLCString));
  
  LCStringRef bothStrings = LCStringCreate(NULL, "abcdabcd");
  LCStringRef stringArray[] = {aLCString, anIdenticalLCString};
  LCStringRef mergedString = LCStringCreateFromStrings(NULL, stringArray, 2);
  mu_assert("LCStringCreateFromStrings is correct", LCStringEqual(mergedString, bothStrings));
  
  /*LCStringRef tokenString = LCStringCreate(NULL, "ab/cd/ef");
  LCArrayRef tokenArray = LCStringCreateTokens(tokenString, '/');
  LCStringRef* tokens = (LCStringRef*)LCArrayObjects(tokenArray);
  mu_assert("LCStringCreateTokens is correct", LCStringEqualCString(tokens[0], "ab") &&
            LCStringEqualCString(tokens[1], "cd") && LCStringEqualCString(tokens[2], "ef"));*/
    
  LCStringRef string1 = LCStringCreate(NULL, "abcd");
  LCStringRef string2 = LCStringCreate(NULL, "abcde");
  LCStringRef string3 = LCStringCreate(NULL, "abd");
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
