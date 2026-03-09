/* RGS Allocator - A custom allocator.
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


#ifndef RGS_SUPPORT_H
#define RGS_SUPPORT_H

#include <stdint.h>


#define ui8 uint8_t //for flag field

/*Errors macro*/
#define SBRKRET         (void *) -1
#define ALLOCFAILURE    (void *) -2
#define NO_FB_MATCH     (void *) -3

/*Custom return*/
#define SB_SUCCESS      1 //Split block success
#define RMV_SUCCESS     1 //Remove FL success

#define SB_FAILURE      1 //Split block failure
#define RMV_FAILURE     0 //Remove FL failure

/*Block Sizes*/
#define MIN_B_SIZE (1 << 4) //16 byte

/*STANDARD ALIGNMENT*/
#define ALIGNMENT (1 << 4)
/*ALIGN SIZE*/
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

/*HELPER MACROS*/
#define ISFREE(block) ((block)->free_h & BLOCK_FREE) //IF the block is free return 1 otherwise block used

/*HEADER SIZE*/
#define HEADER_SIZE sizeof(hblock_t)

/*FOOTER SIZE*/
#define FOOTER_SIZE sizeof(fblock_t)

/*OVERHEAD SIZE*/
#define OVERHEAD_SIZE (HEADER_SIZE + FOOTER_SIZE)

#endif /*RGS_SUPPORT_H*/
