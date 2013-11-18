#include <stdio.h>

// Instance of CWE-843: Access of Resource Using Incompatible Type
// ('Type Confusion')
// http://cwe.mitre.org/data/definitions/843.html

int
main(int argc, char * argv[]) {
  int f2 = 42;
  void (*ptr)() = (void (*)()) f2;
  ptr();
}

