# RGS Allocator

A custom memory allocator written in C. The project implements allocator behavior using a heap, free-list management, and the `sbrk()` system call. It is designed to help understand how dynamic memory management works under the hood and how block metadata is maintained in user-space allocators.

## Overview

Memory allocators are responsible for managing a process’s heap and deciding how to allocate and free memory blocks. This project explores allocator internals at a practical level:

- allocate a block of requested size
- track block metadata using headers and footers
- manage free blocks using a freelist
- split blocks to reduce internal fragmentation
- coalesce adjacent free blocks
- resize blocks when possible
- protect the public allocator API with mutexes

## Features

The project includes:

- custom allocator API
- `sbrk()`-based heap growth
- boundary-tag block metadata
- free-list insertion and removal
- first-fit, best-fit, and worst-fit strategy support
- block splitting and coalescing
- zero-initialized allocation
- resize support
- basic thread-safety wrappers around the public API

## Technical implementation

The project demonstrates several important systems-programming concepts:

- process heap layout
- program-break behavior
- VMA and heap growth semantics
- block metadata and boundary-tag design
- free-list organization
- strategy-based allocation
- memory safety awareness and validation logic

The implementation is split across several files:

- `src/rgs_alloc.c` — allocator entry points and public API
- `src/rgs_functionality.c` — block creation, splitting, coalescing, and helper routines
- `src/rgs_freelist.c` — free-list operations and fit selection
- `includes/rgs_alloc.h` — public allocator interface
- `includes/rgs_functionality.h` and `includes/rgs_support.h` — internal support definitions

## Repository structure

- `src/` — allocator implementation
- `includes/` — internal/public headers
- `test/` — usage examples and validation code
- `Makefile` — build automation
- `READme.md` — project documentation

## Prerequisites

- Linux
- GCC or Clang
- Make
- standard Unix libc

## Build

From the repository root:

```bash
make
```

This builds the binary named `main` in the project root.

## Usage

The public API exposes functions such as:

```c
void *rgs_alloc(size_t size);
void *rgs_resize(void *ptr, size_t size);
void *rgs_free(void *ptr);
void *rgs_alloczero(size_t nitem, size_t size);
```

Example:

```c
char *buf = rgs_alloc(128);
char *buf2 = rgs_alloczero(8, sizeof(int));
char *buf3 = rgs_resize(buf, 256);
rgs_free(buf);
```

## Known limitations

- this is a learning and experimentation project, not a production allocator
- it does not cover the full complexity of a mature allocator implementation
- block metadata and coalescing logic are intentionally simplified
- there is no full stress-testing or verification suite
- thread safety is a conceptual layer rather than a full allocator hardening model

## Future improvements

Possible improvements include:

- more advanced allocator strategies
- segregated free lists
- better fragmentation analysis
- improved correctness validation
- concurrency-aware allocator design
- benchmarking and stress tests

## License

The repository includes a license file. Verify the repository metadata before relying on it as a legal statement.

## Related concepts

- memory allocation
- heap management
- operating systems
- C memory safety
- allocator design
