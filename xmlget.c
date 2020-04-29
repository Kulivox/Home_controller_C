#include "basic_comm.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>




char * getXML() {
  int sockFD;

  struct hostent *host = gethostbyname("biker810.no-ip.org");

  createClientSocket(ip_int, (void *) host->h_addr_list[0], 80, &sockFD);

  const char *request = "GET /xml/ix.xml HTTP/1.1\r\n\r\n";

  ssize_t reqlen = strlen( request );
  if ( write( sockFD, request, reqlen ) != reqlen )
    return perror( "write" ), NULL;

  char * str = malloc(0);
  char buffer[2048] = {0};

  ssize_t totalReadBytes = 0;
  ssize_t readBytes = 0;
  while ((readBytes = read(sockFD, buffer, 2048)) > 0) {
    char * newptr = realloc(str, totalReadBytes + readBytes);

    if (newptr == NULL) {
      free(str);
      perror("Could not obtain more memory\n");
      return NULL;
    }

    str = newptr;

    memcpy(str + totalReadBytes, buffer, readBytes);
    totalReadBytes += readBytes;

  }
  char * newptr = realloc(str, totalReadBytes + 1);

  if (newptr == NULL) {
    free(str);
    perror("Could not obtain more memory\n");
    return NULL;
  }
  str = newptr;
  str[totalReadBytes] = 0;

  return str;
}


int extractValue(char * file, char * value, long * extractedVal) {
  char * needlePos = strstr(file, value);

  if (needlePos == NULL) {
    fprintf(stderr, "Could not find value\n");
    return 1;
  }

  char *valPos = needlePos + strlen(value) + 1;

  int range = 0;
  while (valPos[range] != '<') {
    range += 1;
  }

  char * buff = malloc(range + 1);
  memset(buff, 0, range + 1);
  memcpy(buff, valPos, range);

  *extractedVal =  strtol(buff, NULL, 10);

  free(file);
  free(buff);
  return 0;
}

