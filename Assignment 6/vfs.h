#ifndef VFS_H
#define VFS_H

#include <stddef.h>

#define BLOCK_SIZE 512
#define MAX_NAME_LEN 50

// free block node, doubly linked
typedef struct FreeBlock {
    int blockIndex;
    struct FreeBlock* next;
    struct FreeBlock* prev;
} FreeBlock;

// file or directory node
typedef struct FileNode {
    char name[MAX_NAME_LEN + 1];
    int isDirectory;           // 1 => directory, 0 => file
    struct FileNode* parent;

    // for directories: circular doubly-linked children
    struct FileNode* childHead;
    struct FileNode* next;
    struct FileNode* prev;

    // for files: blocks and size
    int* blockPointers;
    size_t blockCount;
    size_t contentSize;
} FileNode;

// init/shutdown
void vfsInitialize(int numBlocks);   // To start vfs
void vfsShutdown(void);              // To clean everything up

// commands (used by CLI)
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

// To help for prompt
FileNode* vfsGetCwd(void);

#endif // VFS_H
