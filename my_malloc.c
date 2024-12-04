#include "my_malloc.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define META_SIZE sizeof(block_meta_t)
#define HEAP_SIZE (1024 * 1024 + 4096) // 1MB + 4KB for metadata

static void* heap_start = NULL;      // Start of our heap
static free_block_heap_t free_heap;  // Our minheap of free blocks
static int is_initialized = 0;       // Flag to track initialization

static block_meta_t* get_block_meta(void* ptr) {
    return (block_meta_t*)((char*)ptr - META_SIZE);
}

static void swap_blocks(int i, int j) {
    block_meta_t* temp = free_heap.blocks[i];
    free_heap.blocks[i] = free_heap.blocks[j];
    free_heap.blocks[j] = temp;
}

static void heapify_up(int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (free_heap.blocks[parent]->size > free_heap.blocks[index]->size) {
            swap_blocks(parent, index);
            index = parent;
        } else {
            break;
        }
    }
}

static void heapify_down(int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < free_heap.size &&
        free_heap.blocks[left]->size < free_heap.blocks[smallest]->size) {
        smallest = left;
    }

    if (right < free_heap.size &&
        free_heap.blocks[right]->size < free_heap.blocks[smallest]->size) {
        smallest = right;
    }

    if (smallest != index) {
        swap_blocks(index, smallest);
        heapify_down(smallest);
    }
}

static void insert_free_block(block_meta_t* block) {
    if (free_heap.size >= MAX_FREE_BLOCKS) {
        fprintf(stderr, "Free block heap is full\n");
        return;
    }

    free_heap.blocks[free_heap.size] = block;
    heapify_up(free_heap.size);
    free_heap.size++;
}

static block_meta_t* find_free_block(size_t size) {
    for (int i = 0; i < free_heap.size; i++) {
        if (free_heap.blocks[i]->size >= size) {
            block_meta_t* block = free_heap.blocks[i];
            free_heap.blocks[i] = free_heap.blocks[free_heap.size - 1];
            free_heap.size--;
            heapify_down(i);
            return block;
        }
    }
    return NULL;
}

static void remove_free_block(block_meta_t* block) {
    for (int i = 0; i < free_heap.size; i++) {
        if (free_heap.blocks[i] == block) {
            free_heap.blocks[i] = free_heap.blocks[free_heap.size - 1];
            free_heap.size--;
            heapify_down(i);
            return;
        }
    }
}

static void init_heap(void) {
    if (!is_initialized) {
        // Get initial memory block
        heap_start = get_me_blocks(HEAP_SIZE);
        if (heap_start == (void*)-1) {
            fprintf(stderr, "Failed to initialize heap\n");
            return;
        }

        block_meta_t* initial_block = (block_meta_t*)heap_start;
        initial_block->size = HEAP_SIZE - META_SIZE;
        initial_block->requested_size = 0;  // Initially no request
        initial_block->is_free = 1;
        initial_block->next = NULL;

        free_heap.size = 0;
        insert_free_block(initial_block);

        is_initialized = 1;

        #ifdef DEBUG
        fprintf(stderr, "Heap initialized with size: %d bytes at address: %p\n",
                HEAP_SIZE, heap_start);
        #endif
    }
}

void* get_me_blocks(ssize_t how_much) {
    void* ptr = sbrk(0);
    void* request = sbrk(how_much);
    if (request == (void*)-1) {
        return NULL;
    }
    return ptr;
}

static void split_block(block_meta_t* block, size_t size) {
    if (block->size >= size + META_SIZE + 16) {
        block_meta_t* new_block = (block_meta_t*)((char*)block + META_SIZE + size);
        new_block->size = block->size - size - META_SIZE;
        new_block->requested_size = 0;  // New block is free
        new_block->is_free = 1;
        new_block->next = block->next;

        block->size = size;
        block->next = new_block;

        insert_free_block(new_block);
    }
}

void* malloc(size_t size) {
    if (size == 0) return NULL;

    if (!is_initialized) {
        init_heap();
        if (!is_initialized) return NULL;
    }

    size_t original_size = size;

    size = (size + 7) & ~7;  // 8-byte alignment

    block_meta_t* block = find_free_block(size);
    if (block) {
        // Split block if it's too large
        split_block(block, size);
        block->is_free = 0;
        block->requested_size = original_size;

        // Zero out the allocated memory
        memset(ptr, 0, size);
        void* ptr = (char*)block + META_SIZE;
        return ptr;
    }

    return NULL;
}

static void coalesce(block_meta_t* block) {
    block_meta_t* current = (block_meta_t*)heap_start;

    while (current && current->next) {
        if (current->is_free && current->next->is_free) {
            if ((char*)current + META_SIZE + current->size == (char*)current->next) {
                remove_free_block(current);
                remove_free_block(current->next);

                current->size += META_SIZE + current->next->size;
                current->requested_size = 0;  // Merged block has no request
                current->next = current->next->next;

                insert_free_block(current);
                continue;
            }
        }
        current = current->next;
    }
}

void free(void* ptr) {
    if (!ptr) return;

    block_meta_t* block = get_block_meta(ptr);

    if (block < (block_meta_t*)heap_start ||
        block >= (block_meta_t*)((char*)heap_start + HEAP_SIZE)) {
        fprintf(stderr, "Invalid free: pointer outside heap range\n");
        return;
    }

    block->is_free = 1;
    block->requested_size = 0;
    insert_free_block(block);

    coalesce(block);
}

void* realloc(void* ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    block_meta_t* block = get_block_meta(ptr);
    if (block < (block_meta_t*)heap_start ||
        block >= (block_meta_t*)((char*)heap_start + HEAP_SIZE)) {
        return NULL;
    }

    if (block->size >= size) {
        block->requested_size = size;  // Update requested size
        split_block(block, size);
        return ptr;
    }

    void* new_ptr = malloc(size);
    if (!new_ptr) return NULL;

    memcpy(new_ptr, ptr, block->size);
    free(ptr);

    return new_ptr;
}

void print_heap_status(void) {
    block_meta_t* current = (block_meta_t*)heap_start;
    fprintf(stderr, "Heap Status:\n");
    fprintf(stderr, "Heap Start: %p\n", heap_start);

    int block_count = 0;
    size_t total_free = 0;
    size_t total_allocated = 0;

    while (current) {
        fprintf(stderr, "Block %d: addr=%p size=%zu (%zu requested) is_free=%d\n",
                block_count++, current, current->size,
                current->is_free ? 0 : current->requested_size,
                current->is_free);
        if (current->is_free) {
            total_free += current->size;
        } else {
            total_allocated += current->size;
        }
        current = current->next;
    }

    fprintf(stderr, "Total free memory: %zu bytes\n", total_free);
    fprintf(stderr, "Total allocated memory: %zu bytes\n", total_allocated);
    fprintf(stderr, "Total heap size: %d bytes\n", HEAP_SIZE);
    fprintf(stderr, "Free blocks in minheap: %d\n\n", free_heap.size);
}

// Debug function to print minheap status
void print_minheap_status(void) {
    fprintf(stderr, "Minheap Status:\n");
    fprintf(stderr, "Number of free blocks: %d\n", free_heap.size);

    fprintf(stderr, "Minheap contents (sorted by size):\n");
    for (int i = 0; i < free_heap.size; i++) {
        fprintf(stderr, "  Index %d: addr=%p size=%zu\n",
                i, free_heap.blocks[i], free_heap.blocks[i]->size);
    }

    // Verify heap property
    int violations = 0;
    for (int i = 0; i < free_heap.size; i++) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < free_heap.size &&
            free_heap.blocks[left]->size < free_heap.blocks[i]->size) {
            fprintf(stderr, "WARNING: Heap property violation at index %d with left child\n", i);
            violations++;
        }
        if (right < free_heap.size &&
            free_heap.blocks[right]->size < free_heap.blocks[i]->size) {
            fprintf(stderr, "WARNING: Heap property violation at index %d with right child\n", i);
            violations++;
        }
    }

    if (violations == 0) {
        fprintf(stderr, "Heap property verified: OK\n");
    } else {
        fprintf(stderr, "WARNING: Found %d heap property violations!\n", violations);
    }
    fprintf(stderr, "\n");
}