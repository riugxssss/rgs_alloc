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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include "rgs_alloc.h"
#include "rgs_support.h"
#include "rgs_functionality.h"
#include "rgs_freelist.h"

/*A simple function that get the footer of a block doing simple math
 *
 * block = 1000
 * + (block_size - FOOTER_SIZE) 100 - 8 = 92
 * footer -> 1000 + 92 = 1092*/

fblock_t *get_footer(hblock_t *block){
    return (fblock_t *)((char *)block + (block->block_size - FOOTER_SIZE));
}

/*Creating block using SBRK syscall
 * aligning the size requested due to program break alignment
 * (4kb the size of a page) if the sbrk fail we return ALLOCFAILURE
 * (view rgs_support.h for macros) and we manipulate the header fields
 * setting the block to FREE (handling that in allocator func modifying)
 * */

hblock_t *create_block(size_t aligned_size){
    void *reqptr = sbrk(aligned_size);
    if (reqptr == SBRKRET){
        return ALLOCFAILURE;
    }
    hblock_t *newblock = (hblock_t *)reqptr;
    newblock->next = newblock->prev = NULL;
    newblock->free_h = SET_FREE;
    newblock->block_size = aligned_size;

    fblock_t *footer = get_footer(newblock);
    footer->block_size = newblock->block_size;
    footer->free_f = newblock->free_h;
    return newblock;
}

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

int split_block(hblock_t *block, size_t used_size ){
    size_t block_size = block->block_size;
    size_t payload_size = block_size - OVERHEAD_SIZE;
    size_t unused_size; 
    if (used_size < payload_size){
        unused_size = payload_size - used_size;
    }else {
        return SB_FAILURE;
    }
    if (unused_size < MIN_B_SIZE + OVERHEAD_SIZE){ //16 + 32 = 48
        return SB_FAILURE;
    }

    hblock_t *newb = (hblock_t *)((char *)block + (HEADER_SIZE + used_size));
    block->block_size = used_size + OVERHEAD_SIZE; //update the new size
    
    //We update the new field of the struct and get a footer and set the field too
    newb->free_h = SET_FREE;
    newb->block_size = unused_size + OVERHEAD_SIZE;
    fblock_t *newf = get_footer(newb);
    newf->block_size = newb->block_size;
    newf->free_f = newb->free_h;

    fl_insert(newb);
    return SB_SUCCESS;
}


/*Getter addr of the heap start
 * open the proc/self/maps where are stored
 * the address of every VMA and we search for the heap
 * Explanation:
 * The ABI System V realese 5 gives the layout model
 * for every process, a process is a dynamic entity that a program
 * can live, the kernel handle the process with pages (the pages are the memory unit of the kernel)
 * that are fixed-size blocks of memory and they are virtual, but they also have a physic frame 
 * allocated in the RAM (only if the PTE (Page table entry) has a bit called present ON (to 1)) that rappresent the page
 * Every pages of a process is stored in the page table (a table of pages with entries) and every page
 * get descripted by a PTE, thus the kernel knows how to handle everything (Like page fault due MMU request to VA -> PA)
 * know every policy, access modificator and gestional bit of the page. But all of that of explanation to say
 * that the kernel use a struct to describe a process called TASK_STRUCT where contain PID, bit access, bit privilege
 * and so on, but also the mm_struct a struct used to allow better memory management, and so on in the inside of that
 * we find the vma_struct, (for those who don't know what is a VMA that stand for Virtual area memory, is a data structure used by the kernel
 * to rappresent an address interval that have a start-end like the field contained in the struct -> vma_start and vma_end, you can see the vma
 * as a large buffer with limits to allow the use of others VMA) and in that file we opened you can see everything in a process that we talked about.
 *
 *
 * I wish that explanation it was comprhendable (i'm really really sorry i'm practicing my technical english)*/
void *get_heap_start(){
    FILE *fptr = fopen("/proc/self/maps" ,"r");
    if (fptr == NULL){
        return NULL;
    }
    char buf[256];
    void *heap_addr;

    while (fgets(buf, sizeof(buf), fptr)){
        if (strstr(buf, "[heap]")){
            unsigned long start;
            if (sscanf(buf, "%lx", &start) == 1){
                heap_addr = (void *)start;
            }
            break;
        }
    }
    fclose(fptr);
    return heap_addr;
}

/*Function used in reallocator that check if the field
 * exist -> if not return 0
 * to verify that the block is in the range of the heap
 * allowing to add an extra-layer of security
 * 
 * */
int isvalid_ptr(void *ptr){
    if (ptr == NULL){
        return 0;
    }
    hblock_t *to_header = (hblock_t *) ((char *) ptr - HEADER_SIZE);

    //We are in the heap -> checking the fields struct
    if (to_header){
        if (to_header->block_size == 0)     return 0;
        if ((void *)to_header > sbrk(0))    return 0;
    }else {
        return 0;
    }

    return 1;
}
/*Coalescing of blocks
 * 
 * 
 * The coalescing of blocks is a technique used to 
 * merge adjacent blocks (only if they are free) into one big block
 * allowing to reduce the external fragmentation to blocks.
 * There are two cases to check, if the next block is free and 
 * if the prev block is free
 *
 *   prev block - curr block - next block
 * if the prev block is free merge into one 
 *
 *   merge block - next block
 * if the next block is free merge into one
 *   merge block 
 *
 * The prev and the next block were in the freelist so we remove them
 * and add the merged block*/

hblock_t *blocks_coalescing(hblock_t *block){
    if (block == NULL){
        return NULL;
    } 

    //HEAP start-end
    void *heap_start    = get_heap_start();
    void *heap_end      = sbrk(0);

    size_t startb_size = block->block_size;
    hblock_t *next = (hblock_t *)((char *)block + startb_size);    

    //Checking if the pointer is still in the VMA Heap otherwise -> SEGVFAULT
    if ((void *)next < heap_end && ISFREE(next)){
        fl_remove(next);
        block->block_size += next->block_size;

        fblock_t *footer = get_footer(block);
        footer->block_size = block->block_size;
        footer->free_f = block->free_h;
        
        next->prev = next->next = NULL;
    }
    
    //Check if the block is in the heap
    if ((char *)block > (char *)heap_start){

        fblock_t *prev_footer = (fblock_t *) ((char *)block - FOOTER_SIZE);
        hblock_t *prev = (hblock_t *) ((char *)block - prev_footer->block_size);

        if ((char *)prev >= (char *)heap_start && ISFREE(prev)){

            fl_remove(prev);
            prev->block_size += block->block_size;
            fblock_t * footer = get_footer(prev);
            footer->block_size = prev->block_size;
            footer->free_f = prev->free_h;
            
            block->prev = block->next = NULL;

            block = prev;
        }
    }
    
    return block;
}