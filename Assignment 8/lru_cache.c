#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lru_cache.h"

#define TABLE_SIZE 10000

LRUNode* toCreateNode(int key, char *value) {
    LRUNode *node = malloc(sizeof(LRUNode));
    node->key = key;
    strcpy(node->value, value);
    node->prev = node->next = NULL;
    return node;
}

void moveNodeToFront(LRUCache *cache, LRUNode *node) {
    if (cache->mostRecent == node) return;

    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;

    if (cache->leastRecent == node) cache->leastRecent = node->prev;

    node->next = cache->mostRecent;
    node->prev = NULL;
    cache->mostRecent->prev = node;
    cache->mostRecent = node;
}

void deleteTheLeastUsedNode(LRUCache *cache) {
    LRUNode *node = cache->leastRecent;

    cache->lookupTable[node->key] = NULL;

    if (node->prev) node->prev->next = NULL;
    cache->leastRecent = node->prev;

    free(node);
    cache->size--;
}

LRUCache* toInitializeTheNode(int capacity) {
    LRUCache *cache = malloc(sizeof(LRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->mostRecent = NULL;
    cache->leastRecent = NULL;
    cache->lookupTable = calloc(TABLE_SIZE, sizeof(LRUNode*));
    return cache;
}

char* getCacheValue(LRUCache *cache, int key) {
    LRUNode *node = cache->lookupTable[key];
    if (!node) return NULL;
    moveNodeToFront(cache, node);
    return node->value;
}

void putCacheValue(LRUCache *cache, int key, char *value) {
    LRUNode *node = cache->lookupTable[key];

    if (node) {
        strcpy(node->value, value);
        moveNodeToFront(cache, node);
        return;
    }

    if (cache->size == cache->capacity) {
        deleteTheLeastUsedNode(cache);
    }

    LRUNode *newNode = toCreateNode(key, value);
    newNode->next = cache->mostRecent;

    if (cache->mostRecent)
        cache->mostRecent->prev = newNode;

    cache->mostRecent = newNode;

    if (!cache->leastRecent)
        cache->leastRecent = newNode;

    cache->lookupTable[key] = newNode;
    cache->size++;
}

void intitalizingLRUprogram() {
    char command[32];
    LRUCache *cache = NULL;

    char outputLines[500][200];
    int outputCount = 0;

    int firstInput = 1;

    while (scanf("%s", command) == 1) {

        if (firstInput) {
            printf("Input:\n");
            firstInput = 0;
        }

        if (strcmp(command, "createCache") == 0) {
            int capacity;
            scanf("%d", &capacity);
            printf("createCache %d\n", capacity);
            cache = toInitializeTheNode(capacity);
        }

        else if (strcmp(command, "put") == 0) {
            int key;
            char value[100];
            scanf("%d %s", &key, value);
            printf("put %d %s\n", key, value);
            putCacheValue(cache, key, value);
        }

        else if (strcmp(command, "get") == 0) {
            int key;
            scanf("%d", &key);
            printf("get %d\n", key);

            char *result = getCacheValue(cache, key);
            if (result)
                sprintf(outputLines[outputCount++], "%s", result);
            else
                sprintf(outputLines[outputCount++], "NULL");
        }

        else if (strcmp(command, "exit") == 0) {
            printf("exit\n");
            break;
        }
    }

    printf("\nOutput:\n");
    for (int i = 0; i < outputCount; i++) {
        printf("%s\n", outputLines[i]);
    }
}
