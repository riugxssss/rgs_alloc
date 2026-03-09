/* RGS Allocator - A custom allocator.
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
#include "rgs_support.h"
#include "rgs_freelist.h"
#include "rgs_functionality.h"

//Designated initializer
static rgs_fl_struct_t fl_refer = {
    .head = NULL,
    .tot_block = 0
};

/*A simple function that insert the block into the head
 * of the free list -> Computational time = O(1)
 *
 * In a sorted way so the freelist get adjacent blocks*/

void fl_insert(hblock_t *block){

    block->next = block->prev = NULL;

    if (fl_refer.head == NULL) {
        fl_refer.head = block;
        fl_refer.tot_block++;
        return;
    }

    hblock_t *curr = fl_refer.head;

    /* insert before head */
    if (block < curr) {
        block->next = curr;
        curr->prev = block;
        fl_refer.head = block;
        fl_refer.tot_block++;
        return;
    }

    while (curr->next && curr->next < block) {
        curr = curr->next;
    }

    block->next = curr->next;
    block->prev = curr;

    if (curr->next) curr->next->prev = block;

    curr->next = block;

    fl_refer.tot_block++;
}

/*A simple function that remove the block from the free list
 * Computational time -> Best-case -> O(1) - Worst-Case -> O(n)
 *
 * There are 3 cases to handle in the function head - middle - tail
 *
 * head -> where the target is the head so we set the head to NULL
 * and dec the tot block counter
 *
 * tail -> where the target is the head of the freelist so we set the prev->next to NULL
 * and the curr->prev to NULL to destroy the link
 *
 * middle -> where the target has a prev and a next 
 *
 *  1) n1 - n2 - n3 -> target n2, n1 next point to n3 and n3 prev point to n1
 *  2) n1 -> n3 and n2 prev and next reset to NULL
 *  3) n1 - n3 -> done
 *
 * */
int fl_remove(hblock_t *block){
    if (fl_refer.head == NULL){
        return RMV_FAILURE;
    }
    
    hblock_t *curr = fl_refer.head;

    //First case where the target is head
    if (curr == block){
        if (fl_refer.tot_block == 1){
            fl_refer.head = NULL;
            fl_refer.tot_block--;
            return RMV_SUCCESS;
        }
        fl_refer.head = curr->next;
        if (fl_refer.head){
            fl_refer.head->prev = NULL;
            fl_refer.tot_block--;
            return RMV_SUCCESS;
        }
    }

    while (curr){
        if (curr== block){
            //Tail case
            if (curr->next == NULL && curr->prev){
                curr->prev->next = NULL;
                curr->prev = NULL;
                fl_refer.tot_block--;
                return RMV_SUCCESS;
            //Last case
            }else if (curr->next && curr->prev){
                curr->prev->next = curr->next;
                curr->next->prev = curr->prev;
                curr->prev = curr->next = NULL;
                fl_refer.tot_block--;
                return RMV_SUCCESS;
            }
        }
        curr = curr->next;
    }
    return RMV_FAILURE;
}

/*A simple function that stamp the freelist in the stdout stream*/
void print_fl(void){
    if (fl_refer.head == NULL){
        printf("<NULL>\n");
        return;
    }

    //Reassigning the block to a variable for readability
    hblock_t *ptrhead = fl_refer.head;
    int c_node = 0;

    printf("-----PRINTING FREELIST-----\n");

    if (fl_refer.tot_block == 1){
        printf("[node: %d] addr=%p size=%zu\n", c_node, ptrhead, ptrhead->block_size);
        return;
    }

    while (ptrhead){
        printf("[node: %d] addr=%p size=%zu\n", c_node++, ptrhead, ptrhead->block_size);
        if (ptrhead->next == NULL){
            return;
        }
        ptrhead = ptrhead->next;
    }
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
 *
 *
 * IF NOT FOUND RETURN NO_FB_MATCH A VOID * MACRO
 *   */
#if ALLOC_STRATEGY == FIRST_FIT 
 hblock_t *fit_search(size_t size){
    
    hblock_t *curr = fl_refer.head;
    if (curr == NULL){
        return NO_FB_MATCH;
    }
    
    if (fl_refer.tot_block == 1){
        return curr;
    }
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
 *
 *
 *  IF NOT FOUND RETURN NO_FB_MATCH A VOID * MACRO
 *   */

 hblock_t *fit_search(size_t size){
    hblock_t *curr = fl_refer.head;
    hblock_t *best = NULL; //end up with the address of the best block 

    if (fl_refer.head == 1){
        if (curr->block_size >= size) return curr;
        return NO_FB_MATCH;
    }

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
 *  IF NOT FOUND RETURN NO_FB_MATCH A VOID * MACRO
 *
 *   */
#elif ALLOC_STRATEGY == WORST_FIT
 hblock_t *fit_search(size_t size){
    hblock_t *curr = fl_refer.head;
    hblock_t *worst = NULL;

    if (fl_refer.tot_block == 1){
        if (curr->block_size >= size) return curr;
        return NO_FB_MATCH;
    }

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

