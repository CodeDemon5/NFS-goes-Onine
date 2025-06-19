#ifndef __ufs_h__
#define __ufs_h__

#define UFS_BLOCK_SIZE (4096)

#define DIRECT_PTRS (30)

typedef enum {
    UFS_DIRECTORY,
    UFS_REGULAR_FILE
} obj_type;

typedef enum {
    __lookup,
    __stat,
    __write,
    __read,
    __creat,
    __wave,
    __unlink,
    __shut,
} mes_type;


typedef struct {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    unsigned int direct[DIRECT_PTRS];
} inode_t;

typedef struct{
    unsigned int bits[UFS_BLOCK_SIZE / sizeof(unsigned int)]; //bitmaps of len 1
} bitmap_t;

typedef struct{
    mes_type mes_type;
    int code;
    char * content;
} res_t;

typedef struct {
    char name[28];  // up to 28 bytes of name in directory (including \0)
    int  inum;      // inode number of entry (-1 means entry not used)
} dir_ent_t;



// presumed: block 0 is the super block
typedef struct __super {
    int inode_bitmap_addr; // block address (in blocks)
    int inode_bitmap_len;  // in blocks
    int data_bitmap_addr;  // block address (in blocks)
    int data_bitmap_len;   // in blocks
    int inode_region_addr; // block address (in blocks)
    int inode_region_len;  // in blocks
    int data_region_addr;  // block address (in blocks)
    int data_region_len;   // in blocks
    int num_inodes;        // just the number of inodes
    int num_data;          // and data blocks...
} super_t;

typedef struct {
    int fd;
    super_t s;
    bitmap_t * inode_bm;
    bitmap_t * data_bm;
    inode_t * inodes;
} filesystem;

typedef struct {
    mes_type mes_type;
    obj_type type;
    int inum;
    char * buffer;
    int offset;
    int nbytes;
    int pinum;
    char * name;
} mes_t;


#endif // __ufs_h__
