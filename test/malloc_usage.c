#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define ALLOC_STRATEGY 2
#include "../includes/rgs_alloc.h"

#define SMALL_BLOCKS 100
#define RANDOM_BLOCKS 200

int main(void)
{
    printf("Allocator: %s | Strategy: %d\n\n", FIT_NAME, ALLOC_STRATEGY);

    srand(time(NULL));

    // =========================
    // TEST 1 - basic allocation
    // =========================
    printf("=== TEST 1: basic allocation ===\n");

    char *p1 = rgs_alloc(32);
    if (!p1) {
        printf("Allocation error for p1\n");
        return 1;
    }

    strcpy(p1, "hello custom allocator");
    printf("p1: %s\n", p1);


    // =========================
    // TEST 2 - calloc
    // =========================
    printf("\n=== TEST 2: calloc ===\n");

    int *p2 = rgs_alloczero(10, sizeof(int));
    if (!p2) {
        printf("Allocation error for p2\n");
        return 1;
    }

    for (int i = 0; i < 10; i++)
        printf("p2[%d] = %d\n", i, p2[i]);



    // =========================
    // TEST 3 - realloc growth
    // =========================
    printf("\n=== TEST 3: realloc growth ===\n");

    int *p3 = rgs_alloc(sizeof(int) * 4);
    if (!p3) {
        printf("Allocation error for p3\n");
        return 1;
    }

    for (int i = 0; i < 4; i++)
        p3[i] = i + 1;

    p3 = rgs_resize(p3, sizeof(int) * 10);
    if (p3 == NULL){
        printf("Error during realloc\n");
    }

    for (int i = 4; i < 10; i++)
        p3[i] = i + 1;

    for (int i = 0; i < 10; i++)
        printf("%d ", p3[i]);

    printf("\n");



    // =========================
    // TEST 4 - realloc shrink
    // =========================
    printf("\n=== TEST 4: realloc shrink ===\n");

    p3 = rgs_resize(p3, sizeof(int) * 3);

    for (int i = 0; i < 3; i++)
        printf("%d ", p3[i]);

    printf("\n");



    // =========================
    // TEST 5 - many allocations
    // =========================
    
    printf("\n=== TEST 5: many allocations ===\n");

    void *blocks[SMALL_BLOCKS];

    for (int i = 0; i < SMALL_BLOCKS; i++)
    {
        blocks[i] = rgs_alloc(16);

        if (!blocks[i]) {
            printf("Allocation failed at index %d\n", i);
            return 1;
        }

        memset(blocks[i], 0x0, 16);
    }


    
    // =========================
    // TEST 6 - fragmentation
    // =========================
    printf("\n=== TEST 6: fragmentation ===\n");

    for (int i = 0; i < SMALL_BLOCKS; i += 2)
        rgs_free(blocks[i]);

    

    // =========================
    // TEST 7 - memory reuse
    // =========================
    printf("\n=== TEST 7: block reuse ===\n");

    for (int i = 0; i < SMALL_BLOCKS / 2; i++)
    {
        void *tmp = rgs_alloc(8);
        if (!tmp)
            printf("Reuse allocation failed\n");
    }



    // =========================
    // TEST 8 - random stress
    // =========================
    printf("\n=== TEST 8: random stress test ===\n");

    void *rand_blocks[RANDOM_BLOCKS];

    for (int i = 0; i < RANDOM_BLOCKS; i++)
    {
        size_t size = rand() % 128 + 1;
        rand_blocks[i] = rgs_alloc(size);

        if (rand_blocks[i])
            memset(rand_blocks[i], 0x0, size);
    }

    for (int i = 0; i < RANDOM_BLOCKS; i++)
    {
        if (rand() % 2 && rand_blocks[i])
            rgs_free(rand_blocks[i]);
    }



    // =========================
    // TEST 9 - double free
    // =========================
    printf("\n=== TEST 9: double free ===\n");

    char *df = rgs_alloc(32);
    rgs_free(df);

    printf("Second free (should not crash)\n");
    rgs_free(df);



    // =========================
    // CLEANUP
    // =========================
    printf("\n=== CLEANUP ===\n");

    rgs_free(p1);
    rgs_free(p2);
    rgs_free(p3);

    for (int i = 0; i < SMALL_BLOCKS; i++)
        rgs_free(blocks[i]);

    for (int i = 0; i < RANDOM_BLOCKS; i++)
        rgs_free(rand_blocks[i]);


    printf("\nAll stress tests completed successfully!\n");

    return 0;
}
