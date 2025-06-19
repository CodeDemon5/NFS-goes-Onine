#include "ufs.h"
#include "udp.c"
#include "mfs.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#define BUFFER_SIZE (1000)

bool connected = false;

struct sockaddr_in addrSnd, addrRcv;
int sd;
int rc;

int sendMessage(mes_t * message){
  printf("client:: send message type [%d]\n", message->mes_type);
  rc = UDP_Write(sd, &addrSnd, (char *)message, sizeof(mes_t));
  if (rc < 0){
    printf("ERROR : CANNOT COMMMUNICATE TO SERVER\n");
  }else{
    printf("Message sent\n");
  }
  res_t * response;
  rc = UDP_Read(sd, &addrRcv, (char *)response, BUFFER_SIZE);
  if(response->code == 1){
    printf("Reponse received: {%s}",response->content);
  }else{
    printf("REQUEST ERROR: {%s}",response->content);
  }
  return 1;
}


int makeConnection(char *hostname, int port){
  struct sockaddr_in addrSnd, addrRcv;
  sd = UDP_Open(20000);
  rc = UDP_FillSockAddr(&addrSnd, hostname, port);
  assert(sd>0 && rc>=0);
  mes_t sendt;
  sendt.mes_type = __wave;
  sendMessage(&sendt);
  connected = true;
  printf("Made Connection\n");
  return 0;
}


int MFS_Init(char *hostname, int port){
  if (connected){
    return 1;
  }
  else{
    makeConnection(hostname, port);
    return 1;
  }
  return 0;
}
int MFS_Lookup(int pinum, char *name){
  mes_t message;
  message.type = __lookup;
  message.pinum = pinum;
  message.name = name;
  printf("On lookup now\n");
  sendMessage(&message);
  return 1;
};

int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int offset, int nbytes);
int MFS_Read(int inum, char *buffer, int offset, int nbytes);
int MFS_Creat(int pinum, int type, char *name);
int MFS_Unlink(int pinum, char *name);
int MFS_Shutdown(){
  mes_t * mes;
  mes->mes_type = __shut;
  exit(1);
};

// total blocks        36
// inodes            32 [size of each: 128]
// data blocks       32
// layout details
// inode bitmap address/len 1 [1]
// data bitmap address/len  2 [1]