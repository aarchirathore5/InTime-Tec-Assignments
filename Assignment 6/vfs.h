#ifndef VFS_H
#define VFS_H

#include <stddef.h>

#define BLOCK_SIZE 512
#define MAX_NAME_LEN 50

typedef struct FreeBlock {
    int blockIndex;
    struct FreeBlock* next;
    struct FreeBlock* prev;
} FreeBlock;

typedef struct FileNode {
    char name[MAX_NAME_LEN + 1];
    int isDirectory;
    struct FileNode* parent;
    struct FileNode* childHead;
    struct FileNode* next;
    struct FileNode* prev;
    int* blockPointers;
    size_t blockCount;
    size_t contentSize;
} FileNode;

void vfsInitialize(int numBlocks);
void vfsShutdown(void);

void cmdMkdir(const char* dirname);
void cmdCreate(const char* filename);
void cmdCd(const char* dirname);
void cmdLs(void);
void cmdPwd(void);
void cmdDf(void);
void cmdRmdir(const char* dirname);
void cmdDelete(const char* filename);
void cmdRead(const char* filename);
void cmdWrite(const char* filename, const char* content);

FileNode* vfsGetCwd(void);

void runCli(void);

#endif
