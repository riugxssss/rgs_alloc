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

 #ifndef RGS_FUNCTIONALITY_H
 #define RGS_FUNCTIONALITY_H

#include <stdio.h>
#include "rgs_support.h"

/*Metadata of the block header*/

typedef struct hblock {
    size_t block_size; //block size field
    struct hblock *prev; //point to the previous block
    struct hblock *next; //point to the next block
    ui8 free_h; //free flag header
}hblock_t;

/*Metadata of the block footer*/

typedef struct fblock {
    size_t block_size; //block size field
    ui8 free_f; //free flag footer
}fblock_t;

/*API OF THE LIBRARY*/

fblock_t *get_footer(hblock_t *block);

int split_block(hblock_t *block, size_t used_size );
int isvalid_ptr(void *ptr);

hblock_t *blocks_coalescing(hblock_t *block);
hblock_t *create_block(size_t aligned_size);

void *get_heap_start();

/*MACRO -> USAGE = BLOCK ALLOCATION*/

#define SET_FREE        (1 << 0) //00000001
#define BLOCK_FREE      (1 << 0)
#define SET_USED        (1 << 1) //00000010
#define BLOCK_USED      (1 << 1)


 #endif