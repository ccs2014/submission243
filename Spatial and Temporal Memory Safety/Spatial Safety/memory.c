#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Instance of CWE-122: Heap-based Buffer Overflow
// http://cwe.mitre.org/data/definitions/122.html

int 
main(void)
{
  //char buff[1] = malloc(
  char * buff = (char *) malloc(2 * sizeof(char));
  //buffer overflow
  strcpy(buff, "abcd");
  printf("%s\n", buff);
  return 0;
}
