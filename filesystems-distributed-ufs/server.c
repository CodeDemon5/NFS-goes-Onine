#include "udp.c"
#include "ufs.h"
#include "mfs.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <signal.h>

#define BUFFER_SIZE (1000)

bool EvalRequest(mes_t* message, int* sd);
struct sockaddr_in addr;
filesystem files;
super_t* s ;
void * image;
int fd;
// server code
int main(int argc, char* argv[]) {
    // int sd = UDP_Open(atoi(argv[1]));
    fd = open(argv[1], O_RDWR);
    // strcpy(flat_file,argv[1]);
    mes_t * message;
    message->mes_type = __lookup;
    message->pinum = 0;
    message->name = ".";
    int * sd = 0;
    EvalRequest(message,sd);
    // printf("Opened filesystem {%s} on port {%s}\n",argv[2],argv[1]);
    // assert(sd > -1);
    // while (1){
    //     char * m1;
    //     printf("server:: waiting...\n");
    //     rc = UDP_Read(sd, &addr, m1, sizeof(mes_t)/sizeof(char));
    //     printf("%s",m1);
    //     mes_t * message = (mes_t *)m1;
    //     printf("%s\n",message->name);
    //     printf("server:: read message [size:%d contents:(%d), name: (%s)]\n", rc, message->mes_type,message->name);
    //     if (rc > 0 && !SendResponse(message,&sd)){
    //         res_t* response;
    //         response->code = 0;
    //         response->content = "Internal Server Error :?";
    //         rc = UDP_Write(sd, &addr, (char *) &response, sizeof(res_t));
    //         printf("sent internal error\n");
    //     };
        // if (rc > 0){
        //     char reply[BUFFER_SIZE];
        //     sprintf(reply, "goodbye world");
        //     rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
        //     // printf("server:: reply\n");
        // }
        // }
    return 0;
    }

bool EvalRequest(mes_t* message, int* sd) {   
    
    res_t response;
    assert(fd > -1);
    struct stat sbuf;
    int rc = fstat(fd, &sbuf);
    assert(rc > -1);
    int image_size = (int)sbuf.st_size;
    image = mmap(NULL, image_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    assert(image != MAP_FAILED);
    s = (super_t*)image;
    
    files.data_bm = (bitmap_t*)(image + s->data_bitmap_addr * UFS_BLOCK_SIZE);
    files.inode_bm = (bitmap_t*)(image + s->inode_bitmap_addr * UFS_BLOCK_SIZE);
    files.inodes = (inode_t*)(image + s->inode_region_addr * UFS_BLOCK_SIZE);
    
    switch (message->mes_type){
        case __lookup: ;//weirdest workaround ever
            printf("Got to lookup\n");
            inode_t * inode = files.inodes + message->pinum;
            for(int i = 0; i < inode->size; i ++){
                dir_ent_t * obj =  image + (inode->direct[i] * UFS_BLOCK_SIZE);
                if(strcmp(obj->name,message->name) == 0){
                    printf("Found!\n");
                    // response.code = 1;
                    // sprintf(response.content,"{%d}",obj->inum);
                    // rc = UDP_Write(*sd, &addr, (char *) &response, sizeof(res_t));
                    return 1;
                }
            }
            return 0;
        case __stat: ;
            printf("At stat!\n");
            inode_t * inode = files.inodes + message->inum;
            MFS_Stat_t stat;
            stat.size = inode->size;
            stat.type = inode->type;
            break;
        case __read:; //int inum, char *buffer, int offset, int nbytes
            printf("At read!\n");
            inode_t * inode = files.inodes + message->inum;
            if(message->inum >= UFS_BLOCK_SIZE/sizeof(inode_t) || message->inum < 0){
                return;
            }else if(!files.inode_bm->bits[message->inum]){
                return;
            }else if(message->offset >= inode->size){
                return;
            }else if(message->nbytes > inode->size - message->offset){
                return;
            }
            int a = message->offset/UFS_BLOCK_SIZE;
            int b = message->offset%UFS_BLOCK_SIZE;
            char * file = image + UFS_BLOCK_SIZE*inode->direct[a];
            char * answer = malloc(message->nbytes);
            for(int i = 0; i < message->nbytes; i ++){
                if(b + i == UFS_BLOCK_SIZE){
                    file = image + UFS_BLOCK_SIZE*inode->direct[a+1];
                }
                if(b + i >= UFS_BLOCK_SIZE){
                    answer[i] = file[b + i - UFS_BLOCK_SIZE];
                }else{
                    answer[i] = file[b + i];
                }
            }
            puts(answer);
            break;
            
        case __write:
            printf("At write!");
            inode_t * inode = files.inodes + message->inum;
            if(inode->type == UFS_DIRECTORY){
                return;
            }else if(message->inum >= UFS_BLOCK_SIZE/sizeof(inode_t) || message->inum < 0){
                return;
            }else if(!files.inode_bm->bits[message->inum]){
                return;
            }else if(message->offset < 0 || message->offset >= inode->size){
                return;
            }else if(message->nbytes > inode->size - message->offset){
                return;
            }
            int extra = (message->offset + message->nbytes) > inode->size ? message->offset + message->nbytes - inode->size : 0;
            int a = message->offset/UFS_BLOCK_SIZE;
            int b = message->offset%UFS_BLOCK_SIZE;
            char * file = image + UFS_BLOCK_SIZE*inode->direct[a];
            int i = 0;
            memcpy(file, message->buffer , strlen(message->nbytes) - extra);
            
            if(i == message->nbytes){
                ;
            }else if(!extra){
                file = image + UFS_BLOCK_SIZE*inode->direct[a+1];
                memcpy(file, message->buffer + message->nbytes - extra, extra);
            }else{
                int j = 0;
                while(j < s->data_bitmap_len*1024){
                    if(files.data_bm->bits[j] == 1) break;
                    j++;
                }
                inode->direct[a+1] = s->data_region_addr + j;
                file = image + UFS_BLOCK_SIZE*inode->direct[a+1];
                memcpy(file, message->buffer + message->nbytes - extra, extra);
            }                
            rc = msync(image, UFS_BLOCK_SIZE*36 ,MS_SYNC);
            assert(rc > -1);
            break;
        
        default:
            break;
    
    }
    printf("Here?\n");
};