#include <stdio.h>
#include <stdlib.h>

// Return Oriented Attack

int 
bad(){
    printf("\nBad things happen\n\n");
    // call exit() only because return will get into bad()
    exit(0); 
  return 0;
}

int
innocuous(){
  printf("innocuous\n");
  // load the address of bad function 
  // into the stack pointer
  asm("lda $5,0x12000034c");
  asm("stq $5,0($sp)"); 

  return 1;
}

int
main(int argc, char * argv[]) {
  innocuous();

  printf("exiting!");
  return;
}

