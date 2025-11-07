

#include "vfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int gTotalBlocks = 1024;   // default, can be changed at init
static char* gVirtualDisk = NULL;

static FreeBlock* gFreeHead = NULL;
static FreeBlock* gFreeTail = NULL;

static FileNode* gRootNode = NULL;
static FileNode* gCwdNode = NULL;


static void fatalError(const char* message) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}


static FileNode* allocateFileNode(const char* nameText, int isDirectoryFlag) {
    FileNode* node = (FileNode*) malloc(sizeof(FileNode));
    if (node == NULL) fatalError("Out of memory: allocateFileNode");
    strncpy(node->name, nameText, MAX_NAME_LEN);
    node->name[MAX_NAME_LEN] = '\0';
    node->isDirectory = isDirectoryFlag ? 1 : 0;
    node->parent = NULL;
    node->childHead = NULL;
    node->next = node->prev = NULL;
    node->blockPointers = NULL;
    node->blockCount = 0;
    node->contentSize = 0;
    return node;
}


static void pushFreeBlockTail(int blockIndexValue) {
    FreeBlock* node = (FreeBlock*) malloc(sizeof(FreeBlock));
    if (node == NULL) fatalError("Out of memory: pushFreeBlockTail");
    node->blockIndex = blockIndexValue;
    node->next = NULL;
    node->prev = gFreeTail;
    if (gFreeTail != NULL) gFreeTail->next = node;
    gFreeTail = node;
    if (gFreeHead == NULL) gFreeHead = node;
}


static size_t countFreeBlocks(void) {
    size_t count = 0;
    for (FreeBlock* walker = gFreeHead; walker != NULL; walker = walker->next) {
        ++count;
    }
    return count;
}


static int* popFreeBlocksHead(size_t requiredCount) {
    size_t available = countFreeBlocks();
    if (requiredCount > available) return NULL;
    int* allocation = (int*) malloc(requiredCount * sizeof(int));
    if (allocation == NULL) fatalError("Out of memory: popFreeBlocksHead");
    for (size_t allocateIndex = 0; allocateIndex < requiredCount; ++allocateIndex) {
        FreeBlock* node = gFreeHead;
        allocation[allocateIndex] = node->blockIndex;
        gFreeHead = node->next;
        if (gFreeHead != NULL) gFreeHead->prev = NULL;
        else gFreeTail = NULL;
        free(node);
    }
    return allocation;
}


static void returnBlocksToFreeList(const int* indices, size_t countToReturn) {
    for (size_t returnIndex = 0; returnIndex < countToReturn; ++returnIndex) {
        pushFreeBlockTail(indices[returnIndex]);
    }
}


static void insertChildNode(FileNode* parentDir, FileNode* childNode) {
    childNode->parent = parentDir;
    childNode->next = childNode->prev = NULL;
    if (parentDir->childHead == NULL) {
        parentDir->childHead = childNode;
        childNode->next = childNode->prev = childNode;
    } else {
        FileNode* head = parentDir->childHead;
        FileNode* tail = head->prev;
        tail->next = childNode;
        childNode->prev = tail;
        childNode->next = head;
        head->prev = childNode;
    }
}


static void unlinkChildNode(FileNode* parentDir, FileNode* childNode) {
    if (parentDir == NULL || childNode == NULL) return;
    FileNode* head = parentDir->childHead;
    if (head == NULL) return;
    if (head == childNode && childNode->next == childNode) {
        parentDir->childHead = NULL;
        childNode->next = childNode->prev = NULL;
        return;
    }
    FileNode* walker = head;
    int found = 0;
    do {
        if (walker == childNode) { found = 1; break; }
        walker = walker->next;
    } while (walker != head);
    if (!found) return;
    childNode->prev->next = childNode->next;
    childNode->next->prev = childNode->prev;
    if (parentDir->childHead == childNode) parentDir->childHead = childNode->next;
    childNode->next = childNode->prev = NULL;
}


static FileNode* findChildByName(FileNode* directoryNode, const char* nameToFind) {
    if (directoryNode == NULL || directoryNode->childHead == NULL) return NULL;
    FileNode* walker = directoryNode->childHead;
    do {
        if (strcmp(walker->name, nameToFind) == 0) return walker;
        walker = walker->next;
    } while (walker != directoryNode->childHead);
    return NULL;
}


static size_t computeUsedBlocks(void) {
    size_t used = 0;
    size_t stackCapacity = 1024;
    FileNode** stack = (FileNode**) malloc(sizeof(FileNode*) * stackCapacity);
    if (stack == NULL) fatalError("Out of memory: computeUsedBlocks");
    size_t top = 0;
    stack[top++] = gRootNode;
    while (top > 0) {
        FileNode* node = stack[--top];
        if (node->isDirectory) {
            if (node->childHead != NULL) {
                FileNode* childWalker = node->childHead;
                do {
                    if (top >= stackCapacity) {
                        size_t newCap = stackCapacity + 1024;
                        FileNode** reallocated = (FileNode**) realloc(stack, sizeof(FileNode*) * newCap);
                        if (reallocated == NULL) fatalError("Out of memory: computeUsedBlocks expand");
                        stack = reallocated;
                        stackCapacity = newCap;
                    }
                    stack[top++] = childWalker;
                    childWalker = childWalker->next;
                } while (childWalker != node->childHead);
            }
        } else {
            used += node->blockCount;
        }
    }
    free(stack);
    return used;
}


static void printFullPath(FileNode* directory) {
    const size_t maxDepth = 1024;
    const char* pathStack[maxDepth];
    size_t depth = 0;
    FileNode* walker = directory;
    while (walker != NULL && walker->parent != NULL) {
        pathStack[depth++] = walker->name;
        walker = walker->parent;
        if (depth >= maxDepth) break;
    }
    if (depth == 0) { printf("/\n"); return; }
    putchar('/');
    for (ssize_t idx = (ssize_t) depth - 1; idx >= 0; --idx) {
        printf("%s", pathStack[idx]);
        if (idx > 0) putchar('/');
    }
    putchar('\n');
}


static void destroySingleNode(FileNode* node) {
    if (node == NULL) return;
    if (node->blockPointers != NULL) free(node->blockPointers);
    free(node);
}


static char* unescapeString(const char* source) {
    size_t length = strlen(source);
    char* out = (char*) malloc(length + 1); // worst-case
    if (out == NULL) fatalError("Out of memory: unescapeString");
    size_t readPos = 0;
    size_t writePos = 0;
    while (readPos < length) {
        char ch = source[readPos++];
        if (ch == '\\' && readPos < length) {
            char esc = source[readPos++];
            switch (esc) {
                case 'n': out[writePos++] = '\n'; break;
                case 't': out[writePos++] = '\t'; break;
                case '\\': out[writePos++] = '\\'; break;
                case '"': out[writePos++] = '\"'; break;
                case 'r': out[writePos++] = '\r'; break;
                default:
                    // unknown escape: keep both chars
                    out[writePos++] = '\\';
                    out[writePos++] = esc;
                    break;
            }
        } else {
            out[writePos++] = ch;
        }
    }
    out[writePos] = '\0';
    char* shrunk = (char*) realloc(out, writePos + 1);
    if (shrunk != NULL) out = shrunk;
    return out;
}


void vfsInitialize(int numBlocks) {
    if (numBlocks > 0) gTotalBlocks = numBlocks;
    else gTotalBlocks = 1024;

    gVirtualDisk = (char*) malloc((size_t) gTotalBlocks * BLOCK_SIZE);
    if (gVirtualDisk == NULL) fatalError("Failed to allocate gVirtualDisk");

    gFreeHead = gFreeTail = NULL;
    for (int blockIdx = 0; blockIdx < gTotalBlocks; ++blockIdx) pushFreeBlockTail(blockIdx);

    gRootNode = allocateFileNode("/", 1);
    gRootNode->parent = NULL;
    gRootNode->childHead = NULL;
    gCwdNode = gRootNode;
}

void vfsShutdown(void) {
    if (gRootNode != NULL) {
        if (gRootNode->childHead != NULL) {
            FileNode* topWalker = gRootNode->childHead;
            size_t topCount = 0;
            FileNode* counter = topWalker;
            do { ++topCount; counter = counter->next; } while (counter != topWalker);

            FileNode** topChildren = (FileNode**) malloc(sizeof(FileNode*) * topCount);
            if (topChildren == NULL) fatalError("Out of memory: vfsShutdown");
            size_t fill = 0;
            do { topChildren[fill++] = topWalker; topWalker = topWalker->next; } while (topWalker != gRootNode->childHead);

            for (size_t childIndex = 0; childIndex < topCount; ++childIndex) {
                FileNode* subtreeRoot = topChildren[childIndex];
                // collect subtree nodes then free
                size_t stackCap = 1024;
                FileNode** stack = (FileNode**) malloc(sizeof(FileNode*) * stackCap);
                if (stack == NULL) fatalError("Out of memory: vfsShutdown stack");
                size_t stackTop = 0;
                stack[stackTop++] = subtreeRoot;

                FileNode** collected = NULL;
                size_t collectedCap = 0;
                size_t collectedSize = 0;

                while (stackTop > 0) {
                    FileNode* cur = stack[--stackTop];
                    if (collectedSize >= collectedCap) {
                        size_t newCap = collectedCap + 1024;
                        FileNode** reallocated = (FileNode**) realloc(collected, sizeof(FileNode*) * newCap);
                        if (reallocated == NULL) fatalError("Out of memory: collected");
                        collected = reallocated;
                        collectedCap = newCap;
                    }
                    collected[collectedSize++] = cur;

                    if (cur->isDirectory && cur->childHead != NULL) {
                        FileNode* childWalker = cur->childHead;
                        do {
                            if (stackTop >= stackCap) {
                                size_t newStackCap = stackCap + 1024;
                                FileNode** reallocatedStack = (FileNode**) realloc(stack, sizeof(FileNode*) * newStackCap);
                                if (reallocatedStack == NULL) fatalError("Out of memory: stack expand");
                                stack = reallocatedStack;
                                stackCap = newStackCap;
                            }
                            stack[stackTop++] = childWalker;
                            childWalker = childWalker->next;
                        } while (childWalker != cur->childHead);
                    }
                }

                for (size_t idx = 0; idx < collectedSize; ++idx) {
                    FileNode* toFree = collected[idx];
                    if (!toFree->isDirectory && toFree->blockCount > 0 && toFree->blockPointers != NULL) {
                        returnBlocksToFreeList(toFree->blockPointers, toFree->blockCount);
                    }
                    if (toFree->blockPointers != NULL) free(toFree->blockPointers);
                    free(toFree);
                }
                free(collected);
                free(stack);
            }
            free(topChildren);
        }
        free(gRootNode);
        gRootNode = NULL;
        gCwdNode = NULL;
    }


    FreeBlock* fbWalker = gFreeHead;
    while (fbWalker != NULL) {
        FreeBlock* nextFb = fbWalker->next;
        free(fbWalker);
        fbWalker = nextFb;
    }
    gFreeHead = gFreeTail = NULL;

    if (gVirtualDisk != NULL) {
        free(gVirtualDisk);
        gVirtualDisk = NULL;
    }
}


void cmdMkdir(const char* dirname) {
    if (dirname == NULL || dirname[0] == '\0') { printf("Invalid directory name.\n"); return; }
    if (findChildByName(gCwdNode, dirname) != NULL) { printf("Name already exists in current directory.\n"); return; }
    FileNode* newDir = allocateFileNode(dirname, 1);
    insertChildNode(gCwdNode, newDir);
    printf("Directory '%s' created successfully.\n", dirname);
}

void cmdCreate(const char* filename) {
    if (filename == NULL || filename[0] == '\0') { printf("Invalid file name.\n"); return; }
    if (findChildByName(gCwdNode, filename) != NULL) { printf("Name already exists in current directory.\n"); return; }
    FileNode* newFile = allocateFileNode(filename, 0);
    insertChildNode(gCwdNode, newFile);
    printf("File '%s' created successfully.\n", filename);
}

void cmdCd(const char* dirname) {
    if (dirname == NULL) { printf("Invalid directory.\n"); return; }
    if (strcmp(dirname, "..") == 0) {
        if (gCwdNode->parent != NULL) {
            gCwdNode = gCwdNode->parent;
            if (gCwdNode == gRootNode) printf("Moved to /\n");
            else { printf("Moved to "); printFullPath(gCwdNode); }
        } else {
            printf("Already at root.\n");
        }
        return;
    }
    if (strcmp(dirname, "/") == 0) {
        gCwdNode = gRootNode;
        printf("Moved to /\n");
        return;
    }
    FileNode* target = findChildByName(gCwdNode, dirname);
    if (target == NULL || !target->isDirectory) {
        printf("Directory not found.\n");
        return;
    }
    gCwdNode = target;
    printf("Moved to ");
    printFullPath(gCwdNode);
}

void cmdLs(void) {
    if (gCwdNode->childHead == NULL) {
        printf("(empty)\n");
        return;
    }
    FileNode* walker = gCwdNode->childHead;
    do {
        if (walker->isDirectory) printf("%s/\n", walker->name);
        else printf("%s\n", walker->name);
        walker = walker->next;
    } while (walker != gCwdNode->childHead);
}

void cmdPwd(void) {
    printFullPath(gCwdNode);
}

void cmdDf(void) {
    size_t used = computeUsedBlocks();
    size_t freeBlocks = (size_t) gTotalBlocks - used;
    double percent = (gTotalBlocks == 0) ? 0.0 : ((double) used * 100.0) / (double) gTotalBlocks;
    printf("Total Blocks: %d\n", gTotalBlocks);
    printf("Used Blocks: %zu\n\n", used);
    printf("Free Blocks: %zu\n\n", freeBlocks);
    printf("Disk Usage: %.2f%%\n", percent);
}

void cmdRmdir(const char* dirname) {
    if (dirname == NULL || dirname[0] == '\0') { printf("Invalid directory name.\n"); return; }
    FileNode* target = findChildByName(gCwdNode, dirname);
    if (target == NULL || !target->isDirectory) { printf("Directory not found.\n"); return; }
    if (target->childHead != NULL) { printf("Directory not empty. Remove files first.\n"); return; }
    unlinkChildNode(gCwdNode, target);
    destroySingleNode(target);
    printf("Directory removed successfully.\n");
}

void cmdDelete(const char* filename) {
    if (filename == NULL || filename[0] == '\0') { printf("Invalid file name.\n"); return; }
    FileNode* target = findChildByName(gCwdNode, filename);
    if (target == NULL || target->isDirectory) { printf("File not found.\n"); return; }
    if (target->blockCount > 0 && target->blockPointers != NULL) {
        returnBlocksToFreeList(target->blockPointers, target->blockCount);
    }
    unlinkChildNode(gCwdNode, target);
    destroySingleNode(target);
    printf("File deleted successfully.\n");
}

void cmdRead(const char* filename) {
    if (filename == NULL || filename[0] == '\0') { printf("Invalid file name.\n"); return; }
    FileNode* target = findChildByName(gCwdNode, filename);
    if (target == NULL || target->isDirectory) { printf("File not found.\n"); return; }
    if (target->contentSize == 0) { printf("(empty)\n"); return; }
    size_t remaining = target->contentSize;
    size_t pointerIdx = 0;
    while (remaining > 0 && pointerIdx < target->blockCount) {
        int blockNo = target->blockPointers[pointerIdx];
        size_t diskOffset = (size_t) blockNo * BLOCK_SIZE;
        size_t toPrint = (remaining > BLOCK_SIZE) ? BLOCK_SIZE : remaining;
        fwrite(gVirtualDisk + diskOffset, 1, toPrint, stdout);
        remaining -= toPrint;
        ++pointerIdx;
    }
    putchar('\n');
}

void cmdWrite(const char* filename, const char* content) {
    if (filename == NULL || filename[0] == '\0') { printf("Invalid file name.\n"); return; }
    FileNode* target = findChildByName(gCwdNode, filename);
    if (target == NULL || target->isDirectory) { printf("File not found.\n"); return; }

    // unescape content so sequences like \n become actual newline
    char* unescaped = unescapeString(content);
    size_t contentLen = strlen(unescaped);
    size_t requiredBlocks = (contentLen == 0) ? 0 : (contentLen + BLOCK_SIZE - 1) / BLOCK_SIZE;

    int* allocation = NULL;
    if (requiredBlocks > 0) {
        allocation = popFreeBlocksHead(requiredBlocks);
        if (allocation == NULL) {
            printf("Disk full. Write operation failed.\n");
            free(unescaped);
            return;
        }
    }


    if (target->blockCount > 0 && target->blockPointers != NULL) {
        returnBlocksToFreeList(target->blockPointers, target->blockCount);
        free(target->blockPointers);
        target->blockPointers = NULL;
        target->blockCount = 0;
        target->contentSize = 0;
    }

    if (requiredBlocks > 0) {
        target->blockPointers = allocation;
        target->blockCount = requiredBlocks;
        target->contentSize = contentLen;
        size_t bytesLeft = contentLen;
        size_t writtenSoFar = 0;
        for (size_t blockIndexVar = 0; blockIndexVar < requiredBlocks; ++blockIndexVar) {
            int blockNo = allocation[blockIndexVar];
            size_t diskOffset = (size_t) blockNo * BLOCK_SIZE;
            size_t toCopy = (bytesLeft > BLOCK_SIZE) ? BLOCK_SIZE : bytesLeft;
            memcpy(gVirtualDisk + diskOffset, unescaped + writtenSoFar, toCopy);
            if (toCopy < BLOCK_SIZE) memset(gVirtualDisk + diskOffset + toCopy, 0, BLOCK_SIZE - toCopy);
            writtenSoFar += toCopy;
            bytesLeft -= toCopy;
        }
    } else {
        target->blockPointers = NULL;
        target->blockCount = 0;
        target->contentSize = 0;
    }

    printf("Data written successfully (size=%zu bytes).\n", contentLen);
    free(unescaped);
}


FileNode* vfsGetCwd(void) {
    return gCwdNode;
}
