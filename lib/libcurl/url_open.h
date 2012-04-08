

#ifndef LivelyStore_macbook_url_open_h
#define LivelyStore_macbook_url_open_h

#include <stdio.h>
#include <string.h>
#ifndef WIN32
#  include <sys/time.h>
#endif
#include <stdlib.h>
#include <errno.h>

#include <curl/curl.h>

enum fcurl_type_e {
  CFTYPE_NONE=0,
  CFTYPE_FILE=1,
  CFTYPE_CURL=2
};

typedef struct fcurl_data URL_FILE;
URL_FILE *url_fopen(const char *url,const char *operation);
int url_fclose(URL_FILE *file);
int url_feof(URL_FILE *file);
size_t url_fread(void *ptr, size_t size, size_t nmemb, URL_FILE *file);
char * url_fgets(char *ptr, size_t size, URL_FILE *file);
void url_rewind(URL_FILE *file);


#endif
