#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// CWE-78: Improper Neutralization of Special 
// Elements used in an OS Command ('OS Command Injection')
// http://cwe.mitre.org/data/definitions/78.html

# define FILENAME "./input.txt"
# define FULL_COMMAND "/bin/sh -c 'ls -la /tmp/' "

// Read data from a file to run against
void bad()
{
  char * data;
  char data_buf[100] = FULL_COMMAND;
  data = data_buf;
  {
    size_t data_len = strlen(data);
    FILE * pFile;
    if(100-data_len > 1)
    {
      pFile = fopen(FILENAME, "r");
      if (pFile != NULL)
      {
        if (fgets(data+data_len, (int)(100-data_len), pFile) == NULL)
        {
          printf("fgets() failed\n");
          data[data_len] = '\0';
        }
        fclose(pFile);
      }
    }
  }
  //system(data); -- cannot copy external image (execl) on gem5
  printf("%s \n", data);
}

int main(int argc, char * argv[])
{
  bad();
  return 0;
}
