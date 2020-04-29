#include "basic_comm.h"
#include "xmlget.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
  long extVal;
  char * str = getXML();
  extractValue(str, "inp1", &extVal);

  printf("%ld\n", extVal);





  return 0;
}
