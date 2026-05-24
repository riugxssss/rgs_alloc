/* RGS Allocator - A custom allocator with sbrk().
 *
 * Version 1.1 -- 2026 Mar 9
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
 *   * Neither the name of RGS nor the names of its contributors may be used
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


/* ----  STD C API  ----*/

#include <stdio.h>
#include <string.h>
#include <unistd.h> //For sbrk() syscall 
#include <errno.h>
#include <pthread.h>

/* ----  CUSTOM API  ----*/ 
#include "../includes/rgs_support.h"
#include "../includes/rgs_alloc.h"
#include "../includes/rgs_freelist.h"
#include "../includes/rgs_functionality.h"

/*Global mutex init*/ 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


/*The RGS allocator, is a very simple allocator
 * for this function i used various technique, the ALIGN macro
 * align the size to a 16 BYTE mul, I implemented 3 of the fit
 * strategies (you can modify and test it on your own), the explanation of
 * all of them -> https://www.geeksforgeeks.org/operating-systems/partition-allocation-methods-in-memory-management
 * if the strategy don't find a good block in the free list (that matches the size)
 * it return NO_FB_MATCH and request more memory to the kernel with the sbrk() syscall
 * Documentation -> https://man7.org/linux/man-pages/man2/sbrk.2.html
 * if the allocation fail -> ALLOCFAILURE and return NULL
 * if the fit found a block we remove it from the free list and set the free flag BLOCK_USED
 * And for last we try to split the block to prevent the waste of memory*/
static void *rgs_allocator(size_t size){
    if (size == 0){
        return ALLOCFAILURE;
    }
    size_t total_size = ALIGN(size) + OVERHEAD_SIZE;
    hblock_t *ptrret = fit_search(total_size); 
    
    if (ptrret == NO_FB_MATCH){
        ptrret = create_block(total_size);
        if (ptrret == ALLOCFAILURE){
            return NULL;
        }
    }else {
        fl_remove(ptrret);
    }
    ptrret->free_h = SET_USED;

    split_block(ptrret, ALIGN(size));
    return (char *)ptrret + HEADER_SIZE; //Return to the payload (data space) 
}

/*The free function what really does (and not what people usually say)
 * manage the blocks, take as param a ptr allocated in the heap
 * and set the flag (hblock_t struct gives us the free field to specifies avaibility of the block)
 * free to FREE with the macro -> FREE, but our function does not effectively remove that block, that data
 * but only set a block free that means we can reuse that block (filled with old data)
 *
 *  After, calls the coalescing function, the coalescing function merge adiacent blocks that are
 *  free into one big block and put the block in the freelist*/
static void rgs_freeall(void *ptr){
    if (ptr == NULL) return; //Safe to call NULL with the free 
    hblock_t * block =  (hblock_t *)((char *)ptr - HEADER_SIZE);

    if (ISFREE(block)){
        return; //No use-after free
    }

    block->free_h = SET_FREE;
    fblock_t *bfooter = get_footer(block);
    bfooter->block_size = block->block_size;
    bfooter->free_f = SET_FREE;
    //Try to coalesce blocks
    block = blocks_coalescing(block);
    
    fl_insert(block); //after coalescing we insert the new block in the freelist
}

/*Allocation of the block with setting the byte to 0
 * the function use a block allocated with n * m size
 * by the main allocator the return a ptr to the start
 * of the payload of the block filled with random values (due to some old usage)
 * and with the memset function we set n byte to 0x0 (0)
 * MEMSET DOCUMENTATION -> https://en.cppreference.com/w/c/string/byte/memset 
 * */
static void *rgs_callocator(size_t nitem, size_t size){
    if (nitem == 0){
        errno = EINVAL;
        return NULL; //Mul size * nitem = 0 if one is 0
    }


    //Secure check for size *nitem, after year 2002 ISSUES
    if (size != 0 && nitem > SIZE_MAX / size){
        errno = EINVAL;
        return NULL;
    }
    size_t total = size * nitem; //n * m

    void *block_allocated = rgs_allocator(total);
    if (block_allocated == NULL){
        errno = ENOMEM;
        return NULL;
    }
    memset(block_allocated, 0x0, total); //we initialize the byte to 0
    return block_allocated;
}

/*The reallocator function is a way to modify the block size
 * in the examples below we can see that the error handling is 
 * similar to the original realloc (standard C)
 * We modify the errno value too with ENOMEM in the alloc*/
static void *rgs_reallocator(void *ptr, size_t size){


    if (ptr == NULL && size > 0){
        return rgs_allocator(size); //normal allocation
    }
    if (size == 0 && ptr){
        rgs_freeall(ptr);
        return NULL;
    }
    if (!isvalid_ptr(ptr)){
        return NULL;
    }

    //To the start of the block (header fields)
    hblock_t *blockstart = (hblock_t *) ((char *)ptr - HEADER_SIZE);
    size_t bsize = blockstart->block_size;
    //First case 
    if (size < bsize){
        split_block(blockstart, size);
        return ptr;
    }
    //Second case
    if (size > bsize){
        void *newptr = rgs_allocator(size);
        if (newptr == NULL){
            return NULL;
        }
        memcpy(newptr, ptr, bsize - OVERHEAD_SIZE);
        rgs_freeall(ptr);
        return newptr;
    }
    return NULL;
}

/*THREAD-SAFETY FUNCTIONS*/

void *rgs_alloc(size_t size){
    pthread_mutex_lock(&mutex);
    void *ptr = rgs_allocator(size);
    if (ptr == NULL){
        errno = ENOMEM;
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    pthread_mutex_unlock(&mutex);
    return ptr;
}

void *rgs_resize(void *ptr, size_t size){
    pthread_mutex_lock(&mutex);
    void *ptrret = rgs_reallocator(ptr, size);
    if (ptrret == NULL){
        errno = ENOMEM;
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    pthread_mutex_unlock(&mutex);
    return ptrret;
}
void *rgs_alloczero(size_t nitem, size_t size){
    pthread_mutex_lock(&mutex);
    void *ptr = rgs_callocator(nitem, size);
    if (ptr == NULL){
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    pthread_mutex_unlock(&mutex);
    return ptr;
}
void rgs_free(void *ptr){
    pthread_mutex_lock(&mutex);
    rgs_freeall(ptr);
    pthread_mutex_unlock(&mutex);
}
