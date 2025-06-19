#include <stdio.h>
#include "udp.h"
#include "libmfs.c"

#define BUFFER_SIZE (1000)

// client code
int main(int argc, char *argv[]){
  MFS_Init("localhost",10000);
  // MFS_Lookup(0,".");
}
