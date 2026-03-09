RGS Allocator

Overview

RGS Allocator is a custom memory allocator written in C. It provides:

Thread-safe allocation using a global mutex.

Free list management (rgs_freelist.c/.h) with fl_insert, fl_remove, and print_fl.

Three fit strategies: best-fit, worst-fit, first-fit.

Core allocation functions (rgs_alloc.c/.h):

rgs_alloc, rgs_resize, rgs_free, rgs_alloczero

Internal abstractions: rgs_allocator, rgs_reallocator, rgs_freeall, rgs_callocator

Helper functions (rgs_functionality.c/.h):

create_block, get_footer, get_heap_start, isvalid_ptr, etc.

Header-only macros library for constants, alignment, and utilities.

RGS demonstrates advanced memory management techniques including coalescing, splitting, and multiple allocation strategies in a thread-safe environment.

Repository Structure
Directory / File	Description
include/rgs_freelist.h	Free list functions and definitions
include/rgs_alloc.h	Public allocation API
include/rgs_functionality.h	Low-level helper functions
include/rgs_macros.h	Header-only macros
src/rgs_freelist.c	Free list implementation
src/rgs_alloc.c	Core allocator implementation
src/rgs_functionality.c	Helper function implementation
tests/malloc_usage.c	Example usage and test
Makefile	Build configuration
Features

Free List Management: Efficient block tracking with fl_insert, fl_remove, and print_fl.

Allocation Strategies: Best-fit, Worst-fit, First-fit selectable per allocation.

Thread Safety: All allocator functions are protected with a global pthread_mutex.

Core Functions:

rgs_alloc(size_t size) – Allocate memory.

rgs_resize(void* ptr, size_t size) – Resize allocated memory.

rgs_free(void* ptr) – Free allocated memory.

rgs_alloczero(size_t nitem, size_t size) – Allocate zero-initialized memory.

Low-Level Helpers: Block creation, footer/header access, heap start tracking, pointer validation, and coalescing.

Installation & Build

Clone the repository:

git clone https://github.com/riugxssss/rgs_alloc.git
cd rgs_alloc

Build the allocator and test executable using the Makefile:

make

Run the test program:

./malloc_usage

Clean build files:

make clean

The -pthread flag is included in the Makefile for thread-safe operations.

Usage Example
#include "rgs_alloc.h"
#include <stdio.h>

int main() {
    // Allocate 256 bytes
    void* ptr = rgs_alloc(256);
    if (!ptr) {
        perror("Allocation failed");
        return 1;
    }

    // Resize to 512 bytes
    ptr = rgs_resize(ptr, 512);

    // Allocate zero-initialized array
    int* arr = rgs_alloczero(10, sizeof(int));

    // Free memory
    rgs_free(ptr);
    rgs_free(arr);

    return 0;
}
References

This allocator was developed based on:

GeeksforGeeks: Memory Management in C

Linux man pages: malloc, free, realloc, calloc

General literature on free lists, block splitting, and coalescing in dynamic memory allocation.

Notes

Mainly educational, showcasing dynamic memory allocation concepts.

Uses a single global mutex for thread safety; may not scale for high-performance multi-threaded workloads.

Designed for Linux environments; may require adjustments for other OSs.
