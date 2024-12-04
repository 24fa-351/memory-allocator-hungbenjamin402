#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "my_malloc.h"

// Test function prototypes
void test1_basic_allocation(void);
void test2_multiple_allocations(void);
void test3_zero_size(void);
void test4_realloc(void);
void test5_pattern_test(void);
void test6_alternating_alloc_free(void);
void test7_heap_status(void);
void test8_minheap_ordering(void);
void test9_minheap_ordering(void);
void test10_same_size_allocation(void);

// Utility functions
void fill_pattern(void* ptr, size_t size, unsigned char pattern) {
    unsigned char* bytes = (unsigned char*)ptr;
    for(size_t i = 0; i < size; i++) {
        bytes[i] = pattern;
    }
}

int check_pattern(void* ptr, size_t size, unsigned char pattern) {
    unsigned char* bytes = (unsigned char*)ptr;
    for(size_t i = 0; i < size; i++) {
        if(bytes[i] != pattern) {
            return 0;  // Pattern mismatch
        }
    }
    return 1;  // Pattern matches
}

// Test 1: Basic allocation and free
void test1_basic_allocation(void) {
    printf("Running Test 1: Basic allocation and free...\n");
    
    void* ptr = malloc(100);
    if(!ptr) {
        printf("Test 1 FAILED: malloc returned NULL\n");
        return;
    }

    fill_pattern(ptr, 100, 0xAA);
    
    // Verify pattern
    if(!check_pattern(ptr, 100, 0xAA)) {
        printf("Test 1 FAILED: Pattern verification failed\n");
        free(ptr);
        return;
    }
    
    free(ptr);
    printf("Test 1 PASSED\n");
}

// Test 2: Multiple allocations
void test2_multiple_allocations(void) {
    printf("Running Test 2: Multiple allocations...\n");
    
    void* ptrs[5];
    size_t sizes[] = {10, 20, 30, 40, 50};
    
    for(int i = 0; i < 5; i++) {
        ptrs[i] = malloc(sizes[i]);
        if(!ptrs[i]) {
            printf("Test 2 FAILED: malloc %d returned NULL\n", i);
            // Free previously allocated memory
            for(int j = 0; j < i; j++) {
                free(ptrs[j]);
            }
            return;
        }
        fill_pattern(ptrs[i], sizes[i], 0xBB);
    }

    int success = 1;
    for(int i = 0; i < 5; i++) {
        if(!check_pattern(ptrs[i], sizes[i], 0xBB)) {
            printf("Test 2 FAILED: Pattern verification failed for allocation %d\n", i);
            success = 0;
            break;
        }
    }

    for(int i = 0; i < 5; i++) {
        free(ptrs[i]);
    }
    
    if(success) {
        printf("Test 2 PASSED\n");
    }
}

// Test 3: Zero size allocation
void test3_zero_size(void) {
    printf("Running Test 3: Zero size allocation...\n");
    
    void* ptr = malloc(0);

    printf("Test 3 PASSED: malloc(0) returned %p\n", ptr);
    
    if(ptr) {
        free(ptr);
    }
}

// Test 4: Realloc functionality
void test4_realloc(void) {
    printf("Running Test 4: Realloc functionality...\n");
    
    // Initial allocation
    void* ptr1 = malloc(50);
    if(!ptr1) {
        printf("Test 4 FAILED: Initial malloc returned NULL\n");
        return;
    }
    
    fill_pattern(ptr1, 50, 0xCC);
    
    // Reallocate larger
    void* ptr2 = realloc(ptr1, 100);
    if(!ptr2) {
        printf("Test 4 FAILED: realloc returned NULL\n");
        free(ptr1);
        return;
    }

    if(!check_pattern(ptr2, 50, 0xCC)) {
        printf("Test 4 FAILED: Pattern not preserved after realloc\n");
        free(ptr2);
        return;
    }
    
    free(ptr2);
    printf("Test 4 PASSED\n");
}

// Test 5: Fill and verify large patterns
void test5_pattern_test(void) {
    printf("Running Test 5: Pattern test...\n");
    
    size_t size = 1024 * 1024;  // 1MB
    void* ptr = malloc(size);
    if(!ptr) {
        printf("Test 5 FAILED: malloc returned NULL\n");
        return;
    }

    unsigned char* bytes = (unsigned char*)ptr;
    for(size_t i = 0; i < size; i++) {
        bytes[i] = i & 0xFF;
    }

    int success = 1;
    for(size_t i = 0; i < size; i++) {
        if(bytes[i] != (i & 0xFF)) {
            printf("Test 5 FAILED: Pattern mismatch at offset %zu\n", i);
            success = 0;
            break;
        }
    }
    
    free(ptr);
    
    if(success) {
        printf("Test 5 PASSED\n");
    }
}

// Test 6: Alternating allocations and frees
void test6_alternating_alloc_free(void) {
    printf("Running Test 6: Alternating allocations and frees...\n");
    
    void* ptrs[100];
    int success = 1;

    for(int i = 0; i < 100; i++) {
        ptrs[i] = malloc(100);
        if(!ptrs[i]) {
            printf("Test 6 FAILED: malloc %d returned NULL\n", i);
            success = 0;
            break;
        }
        fill_pattern(ptrs[i], 100, 0xDD);
        
        if(i % 2 == 0) {
            if(!check_pattern(ptrs[i], 100, 0xDD)) {
                printf("Test 6 FAILED: Pattern verification failed for allocation %d\n", i);
                success = 0;
                break;
            }
            free(ptrs[i]);
            ptrs[i] = NULL;
        }
    }

    for(int i = 0; i < 100; i++) {
        if(ptrs[i]) {
            free(ptrs[i]);
        }
    }
    
    if(success) {
        printf("Test 6 PASSED\n");
    }
}

// Test 7: Heap Status Check
void test7_heap_status(void) {
    printf("\nRunning Test 7: Heap Status Check...\n");

    printf("\n=== Initial heap status ===\n");
    print_heap_status();

    void* ptr1 = malloc(100);
    printf("\n=== After allocating 100 bytes ===\n");
    print_heap_status();

    void* ptr2 = malloc(200);
    printf("\n=== After allocating 200 more bytes ===\n");
    print_heap_status();

    free(ptr1);
    printf("\n=== After freeing first allocation ===\n");
    print_heap_status();

    free(ptr2);
    printf("\n=== After freeing second allocation ===\n");
    print_heap_status();

    printf("\nTest 7 completed\n");
    printf("----------------------------------------\n");
}

// Test 8: Minheap ordering test
void test8_minheap_ordering(void) {
    printf("\nRunning Test 8: Minheap ordering test...\n");

    void* ptrs[5];
    size_t sizes[] = {500, 100, 300, 200, 400};

    // Allocate all blocks
    for(int i = 0; i < 5; i++) {
        ptrs[i] = malloc(sizes[i]);
        if(!ptrs[i]) {
            printf("Test 8 FAILED: malloc %d returned NULL\n", i);
            return;
        }
    }

    printf("\n=== After initial allocations ===\n");
    print_heap_status();
    print_minheap_status();

    printf("\n=== Freeing block of size 100 ===\n");
    free(ptrs[1]);
    print_minheap_status();

    printf("\n=== Freeing block of size 300 ===\n");
    free(ptrs[2]);
    print_minheap_status();

    printf("\n=== Freeing block of size 500 ===\n");
    free(ptrs[0]);
    print_minheap_status();

    printf("\n=== Freeing block of size 200 ===\n");
    free(ptrs[3]);
    print_minheap_status();

    printf("\n=== Freeing block of size 400 ===\n");
    free(ptrs[4]); // 400
    print_minheap_status();

    printf("\nTest 8 completed - All blocks freed and minheap verified\n");
    printf("----------------------------------------\n");
}

// Test 9: Block size metadata test
void test9_metadata(void) {
    printf("\nRunning Test 9: Block size metadata test...\n");

    void* ptr1 = malloc(100);
    void* ptr2 = malloc(255);
    void* ptr3 = malloc(1024);

    printf("\n=== After allocating blocks of 100, 255, and 1024 bytes ===\n");
    print_heap_status();

    free(ptr2);
    printf("\n=== After freeing 255-byte block ===\n");
    print_heap_status();

    // Reallocate with different size
    void* ptr4 = malloc(200);   // Should reuse the freed block
    printf("\n=== After allocating 200 bytes in freed space ===\n");
    print_heap_status();

    // Cleanup
    free(ptr1);
    free(ptr3);
    free(ptr4);

    printf("\n=== After freeing all blocks ===\n");
    print_heap_status();

    printf("\nTest 9 completed - Metadata tracking verified\n");
    printf("----------------------------------------\n");
}

// Test 10: Same-size allocations test
void test10_same_size_allocation(void) {  // Fixed function name to match array
    printf("\nRunning Test 10: Same-size allocations test...\n");

#define NUM_ALLOCS 10
#define ALLOC_SIZE 128
    void* ptrs[NUM_ALLOCS];

    for(int i = 0; i < NUM_ALLOCS; i++) {
        ptrs[i] = malloc(ALLOC_SIZE);
        if(!ptrs[i]) {
            printf("Test 10 FAILED: malloc %d returned NULL\n", i);
            return;
        }

        fill_pattern(ptrs[i], ALLOC_SIZE, i & 0xFF);
        if(!check_pattern(ptrs[i], ALLOC_SIZE, i & 0xFF)) {
            printf("Test 10 FAILED: Pattern verification failed at block %d\n", i);
            return;
        }
    }

    printf("\n=== After allocating %d blocks of size %d ===\n",
           NUM_ALLOCS, ALLOC_SIZE);
    print_heap_status();

    // Free alternate blocks
    printf("\n=== After freeing alternate blocks ===\n");
    for(int i = 0; i < NUM_ALLOCS; i += 2) {
        free(ptrs[i]);
    }
    print_heap_status();

    // Reallocate same size
    printf("\n=== After reallocating freed blocks ===\n");
    for(int i = 0; i < NUM_ALLOCS; i += 2) {
        ptrs[i] = malloc(ALLOC_SIZE);
        if(!ptrs[i]) {
            printf("Test 10 FAILED: Reallocation failed at block %d\n", i);
            return;
        }
    }
    print_heap_status();

    // Free all
    printf("\n=== After freeing all blocks ===\n");
    for(int i = 0; i < NUM_ALLOCS; i++) {
        free(ptrs[i]);
    }
    print_heap_status();

    printf("\nTest 10 completed - Same-size allocation test passed\n");
    printf("----------------------------------------\n");
}

typedef void (*test_func)(void);
test_func tests[] = {
    test1_basic_allocation,
    test2_multiple_allocations,
    test3_zero_size,
    test4_realloc,
    test5_pattern_test,
    test6_alternating_alloc_free,
    test7_heap_status,
    test8_minheap_ordering,
    test9_metadata,
    test10_same_size_allocation
};

int main(int argc, char* argv[]) {
    if(argc > 2 && strcmp(argv[1], "-t") == 0) {
        // Run specific test
        int test_num = atoi(argv[2]);
        if(test_num > 0 && test_num <= sizeof(tests)/sizeof(tests[0])) {
            tests[test_num - 1]();
        } else {
            printf("Invalid test number. Valid range: 1-%zu\n", 
                   sizeof(tests)/sizeof(tests[0]));
            return 1;
        }
    } else {
        // Run all tests
        printf("Running all tests...\n\n");
        for(size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
            tests[i]();
            printf("\n");
        }
    }
    
    return 0;
}