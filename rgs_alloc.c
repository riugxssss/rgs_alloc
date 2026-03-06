/* RGS Allocator - A custom allocator with sbrk().
 *
 * Version 1.0 -- 2026
 *
 * Copyright (c) 2026, A Riugxss <riugxs@ gmail .com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h> //For sbrk() syscall 
#include "rgs_support.h"
#include "rgs_alloc.h"
#include <errno.h>

/*STANDARD ALIGNMENT*/
#define ALIGNMENT (1 << 4)
/*ALIGN SIZE*/
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

/*Metadata of the block header*/

typedef struct hblock {
    size_t block_size;
    struct hblock *prev;
    struct hblock *next;
    unsigned char free : 2; 
    #define SET_FREE (1 << 0) //00000001
    #define FREE     (1 << 0)
    #define SET_OCC  (1 << 1) //00000010
    #define OCC      (1 << 1)
}hblock_t;

/*Metadata of the block footer*/

typedef struct fblock {
    size_t block_size;
    unsigned char free : 2;
}fblock_t;

/*Free list global variable*/
hblock_t *freelist = NULL;

/*HELPER MACROS*/
#define ISFREE(block) ((block)->free & FREE) //IF the block is free return 1 otherwise OCC

/*HEADER SIZE*/
#define HEADER_SIZE sizeof(hblock_t)

/*FOOTER SIZE*/
#define FOOTER_SIZE sizeof(fblock_t)

/*OVERHEAD SIZE*/
#define OVERHEAD_SIZE (HEADER_SIZE + FOOTER_SIZE)

/*Creating block using SBRK syscall
 * aligning the size requested due to program break alignment
 * (4kb the size of a page)
 * */

static fblock_t *get_footer(hblock_t *block){
    if (block == NULL){
        return FOOTER_ERROR;
    }
    return (fblock_t *)((char *)block + (block->block_size - FOOTER_SIZE));
}

/*Creating block using SBRK syscall
 * aligning the size requested due to program break alignment
 * (4kb the size of a page) if the sbrk fail we return ALLOCFAILURE
 * (view rgs_support.h for macros) and we manipulate the header fields
 * setting the block to FREE (handling that in allocator func modifying)
 * */

static hblock_t *create_block(size_t req_size){
    size_t aligned_size = ALIGN(req_size) + OVERHEAD_SIZE;
    void *reqptr = sbrk(aligned_size);
    if (reqptr == SBRKRET){
        //fatal(int errnocode, const char *error)
        return ALLOCFAILURE;
    }
    hblock_t *newblock = (hblock_t *)reqptr;
    newblock->free |= SET_FREE;
    newblock->next = newblock->prev = NULL;
    newblock->block_size = aligned_size;
    return newblock;
}

/*A simple function that insert the block into the head
 * of the free list -> Computational time = O(1)*/

static void insert_blockfl(hblock_t *block){
    if (freelist == NULL){
        freelist = block;
        return;
    }

    /*Save address of old block head
     * and update the list, and moreover we set at the end
     * the head->prev to NULL*/
    hblock_t *tmp = freelist;
    freelist->prev = block;
    freelist= block;
    freelist->next = tmp;

    freelist->prev = NULL;
}

/*A simple function that remove the block from the free list
 * Computational time -> Best-case -> O(1) - Worst-Case -> O(n)*/
static int remove_blockfl(hblock_t *block){
    if (freelist == NULL){
        return -1;
    }
    
    hblock_t *curr = freelist;

    //First case where the target is head
    if (curr == block){
        if (curr->next){
            freelist = freelist->next;
            return 1;
        }else {
            freelist = NULL;
            return 1;
        }
    }

    while (curr){
        if (curr== block){
            //Tail case
            if (curr->next == NULL && curr->prev){
                curr->prev->next = NULL;
                curr->prev = NULL;
                return 1;
            }else if (curr->next && curr->prev){
                curr->prev->next = curr->next;
                curr->next->prev = curr->prev;
                curr->prev = NULL;
                curr->next = NULL;
                return 1;
            }
        }
        curr = curr->next;
    }
    return 0;
}

/*Implementing FIT-strategies searching blocks*/

/*First FIT
 *
 * - Takes the first block that soddisfy the size request*
 *
 *  Example -> FL = 20 - 32 - 48
 *  size request -> 25
 *  
 *  20 < 25 no match -> go next
 *  32 > 25 match -> return that free block
 *
 *  Complexity time -> O(n)
 *
 *   */
#if ALLOC_STRATEGY == FIRST_FIT 
static hblock_t *fit_search(size_t size ){
    hblock_t *curr = freelist;
    while (curr){
        if (curr->block_size >= size){
            return curr;
        }
        curr = curr->next;
    }
    return NO_FB_MATCH;
}
#elif ALLOC_STRATEGY == BEST_FIT
/*Best FIT
 *
 * - Takes the block with the closest size to the size request
 *
 *  Example -> FL = 20 - 32 - 35 - 31
 *  size request = 23
 *  20 < 23 no match
 *  32 > 23 ok first match go on
 *  35 > 32 no, best still 32
 *  32 > 31 final match 31 -> ret 31
 *  
 *  Complexity time -> O(n)
 *
 *   */

static hblock_t *fit_search(size_t size ){
    hblock_t *curr = freelist;
    hblock_t *best = NULL; //end up with the address of the best block 
    while (curr){
        if (curr->block_size >= size){
            if (best == NULL || curr->block_size < best->block_size){
                best = curr;
            }
        }
        curr = curr->next;
    }
    if (best == NULL){
        return NO_FB_MATCH;
    }
    return best;
}

/*Worst FIT
 *
 * - Takes the block with the highest size
 *
 *  Example -> 23 - 45 - 30 
 *  size request = 21
 *  21 < 23 ok first match -> worst = 23
 *  23 < 45 ok second match -> worst = 45
 *  45 > 30 no match -> final worst = 45
 *  
 *  Complexity time -> O(n)
 *
 *   */
#elif ALLOC_STRATEGY == WORST_FIT
static hblock_t *fit_search(size_t size){
    hblock_t *curr = freelist;
    hblock_t *worst = NULL;
    while (curr){
        if (curr->block_size >= size){
            if (worst == NULL || curr->block_size > worst->block_size){
                worst = curr;
            }
        }
        curr = curr->next;
    }
    if (worst == NULL){
        return NO_FB_MATCH;
    }
    return worst;
}
#else
#error "INVALID MACRO VALUE"
#endif /*ALLOC STRATEGY CHOICES*/

/*------------------------------------------------------*/

/*Splitting blocks
 *
 *  The splitting blocks tecnhique is used to reduce 
 *  the internal fragmentation due to allocation
 *  and splitting the blocks means to reuse the extra-byte allocated in our block
 *  Example:
 *      allocated payload -> 88 (+ OVERHEAD_SIZE)
 *      used_size 22
 *      88 - 22 = 66 unused a big waste!
 *      unused < MIN_B_SIZE (32) + HEADER_SIZE (24) = (56) -> NO we can split.
 *      base address of that block + block->size - (66 + footer_size) -> it leads to the newb address
 *      Example with numbers: base: 1000 + size(88 + OVERHEAD_SIZE) - (66 + 8) = 1120 - 74 = 1046 - 24 = 1022 start of the payload
 * */ 

static int split_block(hblock_t *block, size_t used_size ){
    size_t block_size = block->block_size;
    size_t payload_size = block_size - OVERHEAD_SIZE;
    size_t unused_size = payload_size - used_size;
    if (unused_size < MIN_B_SIZE + HEADER_SIZE){
        return SB_FAILURE;
    }

    hblock_t *newb = (hblock_t *)(((char *)block + block->block_size) - (unused_size + FOOTER_SIZE));
    newb->free |= SET_FREE;
    newb->block_size = unused_size + OVERHEAD_SIZE;
    fblock_t *newf = get_footer(newb);
    newf->block_size = newb->block_size;
    newf->free = newb->free;
    insert_blockfl(newb);
    return SB_SUCCESS;
}

/*Coalescing of blocks
 * 
 * 
 * The coalescing of blocks is a technique used to 
 * merge adjacent blocks (only if they are free) into one big block*/

static hblock_t *blocks_coalescing(hblock_t *block){
    
    size_t startb_size = block->block_size;
    //Check if the next is free
    if (block->next){
        hblock_t *next = (hblock_t *)((char *)block + startb_size);
    
        //Checking if the pointer is still in the VMA Heap otherwise -> SEGVFAULT
        if ((void *)next < sbrk(0) && ISFREE(next)){
            size_t next_size = next->block_size;
            size_t newsize = startb_size + next_size;
            block->block_size = newsize;
            block->next = next->next;
            if (next->next){
                next->next->prev = block;
            }
        }
    }

    if (block->prev){
        //We are in the prev block header now
        hblock_t *prev_footer = (hblock_t *) ((char *)block - FOOTER_SIZE);
        hblock_t *prev_header = (hblock_t *) ((char *)block - prev_footer->block_size);
        if (ISFREE(prev_header)){
        size_t prev_size = prev_header->block_size;
        size_t newsize = startb_size + prev_size;
        block->block_size = newsize;
        block->prev = prev_header->prev;
        if (prev_header->prev){
            prev_header->prev->next = block;
            }
        }
    }
    return block;
}

/*The RGS allocator, is a very simple allocator, infact
 * for this function i used various technique, the ALIGN macro
 * align the size to a 16 BYTE mul, I implemented 3 of the fit
 * strategies (you can modify and test it on your own), the explanation of
 * all of them -> https://www.geeksforgeeks.org/operating-systems/partition-allocation-methods-in-memory-management
 * if the strategy don't find a good block in the free list (that matches the size)
 * it return NO_FB_MATCH and request more memory to the kernel with the sbrk() syscall
 * Documentation -> https://man7.org/linux/man-pages/man2/sbrk.2.html
 * if the allocation fail -> ALLOCFAILURE and return NULL
 * if the fit found a block we remove it from the free list and set the free flag OCC
 * And for last we try to split the block to prevent the waste of memory*/

void *rgs_allocator(size_t size){
    if (size <= 0){
        return ALLOCFAILURE;
    }
    size_t sizeoh = ALIGN(size) + OVERHEAD_SIZE;
    hblock_t *ptrret = fit_search(sizeoh); 

    if (ptrret == NO_FB_MATCH){
        ptrret = create_block(size);
        if (ptrret == ALLOCFAILURE){
            return NULL;
        }
    }else {
        remove_blockfl(ptrret);
    }
    ptrret->free |= SET_OCC;
    split_block(ptrret, size);
    return (char *)ptrret + HEADER_SIZE;
}

/*The free function what really does (and not what people usually say)
 * manage the blocks, take as param a ptr allocated in the heap
 * and set the flag (hblock_t struct gives us the free field to specifies avaibility of the block)
 * free to FREE with the macro -> FREE, but our function does not effectively remove that block, that data
 * but only set a block free that means we can reuse that block (filled with old data)
 * -------------
 *  After, calls the coalescing function, the coalescing function merge adiacent blocks that are
 *  free into one big block and put the block in the freelist*/
void rgs_free(void *ptr){
    if (ptr == NULL) return; //Safe to call NULL with the free  
    hblock_t * block =  (hblock_t *)((char *)ptr - HEADER_SIZE);
    block->free |= SET_FREE;
    fblock_t *bfooter = get_footer(block);
    bfooter->block_size = block->block_size;

    //Try to coalesce blocks
    
    hblock_t *merged = blocks_coalescing(block);
    insert_blockfl(merged);
}

/*Allocation of the block with setting the byte to 0
 * the function use a block allocated with n * m size
 * by the main allocator the return a ptr to the start
 * of the payload of the block filled with random values (due to some old usage)
 * and with the memset function we set n byte to 0x0 (0)
 * MEMSET DOCUMENTATION -> https://en.cppreference.com/w/c/string/byte/memset */

void *rgs_callocator    (size_t nitem, size_t size){
    if (size == 0 || nitem == 0){
        return NULL; //Mul size * nitem = 0 if one is 0
    }
    size_t total = size * nitem;
    hblock_t *block_allocated = rgs_allocator(total);
    memset(block_allocated, 0x0, total);
    return block_allocated;
}

//TODO: reallocator
void *rgs_reallocator   (void *ptr, size_t size);

