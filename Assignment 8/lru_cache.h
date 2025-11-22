#ifndef LRU_CACHE_H
#define LRU_CACHE_H

typedef struct LRUNode {
    int key;
    char value[100];
    struct LRUNode *prev, *next;
} LRUNode;

typedef struct {
    int capacity;
    int size;
    LRUNode *mostRecent;
    LRUNode *leastRecent;
    LRUNode **lookupTable;
} LRUCache;

LRUCache* toInitializeTheNode(int capacity);
char* getCacheValue(LRUCache *cache, int key);
void putCacheValue(LRUCache *cache, int key, char *value);
void intitalizingLRUprogram();

#endif
