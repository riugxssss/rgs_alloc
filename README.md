RGS Allocator
Overview

RGS Allocator is a custom memory allocator written in C, featuring:

Thread-safe allocation with a global mutex.

Free list management (rgs_freelist.c/.h) with insert/remove and print utilities.

Three fit strategies: best-fit, worst-fit, first-fit.

Core allocation functions (rgs_alloc.c/.h) including:

rgs_alloc, rgs_resize, rgs_free, rgs_alloczero

Internal abstractions: rgs_allocator, rgs_reallocator, rgs_freeall, rgs_callocator

Fundamental helper functions (rgs_functionality.c/.h) such as:

create_block, get_footer, get_heap_start, isvalid_ptr, etc.

Header-only macros library for modularity and code clarity.

RGS demonstrates full-featured memory management with coalescing, splitting, multiple allocation strategies, and basic thread-safety.

Repository Structure
rgs_alloc/
│
├─ include/                # Header files
│   ├─ rgs_freelist.h
│   ├─ rgs_alloc.h
│   ├─ rgs_functionality.h
│   └─ rgs_macros.h
│
├─ src/                    # Source files
│   ├─ rgs_freelist.c
│   ├─ rgs_alloc.c
│   └─ rgs_functionality.c
│
├─ tests/                  # Test file
│   └─ malloc_usage.c
│
├─ Makefile                # Build configuration
└─ README.md
Features

Free List Management (rgs_freelist):

fl_insert, fl_remove, print_fl

Maintains free blocks efficiently for fast allocation.

Allocation Strategies:

Best-fit, Worst-fit, First-fit selectable per allocation.

Thread Safety:

Uses a global pthread_mutex to make all allocation functions safe for multi-threaded use.

Core Functions (rgs_alloc):

rgs_alloc(size_t size) – allocate memory.

rgs_resize(void* ptr, size_t size) – resize allocated memory.

rgs_free(void* ptr) – free allocated memory.

rgs_alloczero(size_t nitem, size_t size) – allocate zero-initialized memory.

Low-Level Helpers (rgs_functionality):

Functions for block creation, footer/header access, heap start tracking, pointer validation, and coalescing.

Macros Library:

Provides constants, alignment helpers, and other utilities for modular code.

Installation & Build

Clone the repository:

git clone https://github.com/riugxssss/rgs_alloc.git
cd rgs_alloc

Build the allocator and the test using the Makefile:

make

This will compile the source files and generate the test executable.

Run the test program:

./malloc_usage

The Makefile includes -pthread to enable thread-safe operations.

Clean build files:

make clean
Learning References

The allocator was implemented based on the following study resources:

GeeksforGeeks: Memory Management in C

Linux man pages: malloc, free, realloc, calloc

General references on free lists, block splitting, and coalescing in dynamic memory allocation.

Usage Example
#include "rgs_alloc.h"

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
Notes

This allocator is mainly educational and demonstrates advanced memory management techniques.

Currently uses a single global mutex for thread safety; performance may degrade under heavy multi-threaded usage.

Designed for Linux environments; may require adjustments for other OSs.
