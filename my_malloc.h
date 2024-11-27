#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stddef.h>
#include <sys/types.h>

// Block metadata structure
typedef struct block_meta {
    size_t size;           // Size of the block
    size_t requested_size; // Original requested size by user
    int is_free;          // 1 if block is free, 0 if allocated
    struct block_meta *next;  // Pointer to the next block
} block_meta_t;

// Minheap structure for free blocks
#define MAX_FREE_BLOCKS 1024

typedef struct {
    block_meta_t* blocks[MAX_FREE_BLOCKS];
    int size;
} free_block_heap_t;

// Core memory management functions
void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);

// Helper function to get initial memory block
void* get_me_blocks(ssize_t how_much);

// Debug functions
void print_heap_status(void);
void print_minheap_status(void);

#endif // MY_MALLOC_H