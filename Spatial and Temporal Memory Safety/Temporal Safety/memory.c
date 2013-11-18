#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Instance of CWE-416: Use After Free
// http://cwe.mitre.org/data/definitions/416.html

int
main(int argc, char *argv[])
{
    char * data;
    data = NULL;
    data = (char *)malloc(100*sizeof(char));
    memset(data, 'A', 100-1);
    data[100-1] = '\0';
    free(data);
    printf("%s\n", data);
}

/* comic of modified malloc
void* malloc(size_t bytes) {
  void *data = malloc(bytes);
  printf("Bytes: %d", bytes);
  printf("Data:  %08x\n", data);
  return data;
}
*/
