# RGS Allocator, an allocator made in C

RGS is a classical allocator with some features, the structure
is pretty similary to the standard C allocator - [Documentation](https://www.gnu.org/software/libc/manual/html_node/Memory-Allocation.html)

## What is an allocator?

*hint: if you already know in detail what it does skip this part*

**A memory allocator is a software** that can be useful for dynamic allocation, a dynamic allocation
is a (in C mainly) manual allocations where the user interact with the **Heap**..

*for those who dont know what is a heap - ([Heap](https://www.geeksforgeeks.org/what-is-a-memory-heap/))*

An allocator can do various manipulation of the heap, in my allocator (like in many others)
there are **Four basics manipulation**

But first for users that don't know i'll explain **how the heap work and the structure** of that memory.

The heap is a contiguos bytes field, like any other VMA (Virtual area memory), that **must have**
a start and an end. When a process is ready to the use that means the kernel allocated all of the necessary resources,
in GNU/Linux systems (like-Unix) every process follows a layout-model, **given by the ABI (Application Binary Interface)**
and in every process there is a VMA dedicated to the heap, where the allocators and allocate and free memory.

*Other links*
[ABI](https://en.wikipedia.org/wiki/Application_binary_interface)
[Allocator](https://sumofbytes.com/blog/whats-a-memory-allocator-anyway/)

## My Allocator - RGS

**The RGS (RiuGxS) Allocator** is an allocator made to learn, made in 5 days and it cover most of the basic features of 
allocator, the code can be used to comprehend something or how i did the allocator. It uses the sbrk() syscall, the 
sbrk syscall from the libc library allows to modify the program break of the VMA, **but what is the program break?**

**FANCY EXPLANATION**
The program break of the Heap VMA is the limit you can use of the heap and if you expand the program break you tell
the kernel to give you more memory. The sbrk() syscall return a void *ptr to the old limit of the heap.

**API**

1) RGS_ALLOC
2) RGS_ALLOCZERO
3) RGS_RESIZE
4) RGS_FREE


1. *rgs_alloc* allocate a block and return the ptr to the user data, **if the allocation failed errno setted and NULL is returned**
  
2. *rgs_alloczero* allocate a block and set every byte of the user data zone to 0, **if the allocation failed errno setted and NULL is returned**
   
3. *rgs_resize* resize a block size and based on the size you entered, it does optimization, **if fail errno setted and NULL is returned**
   
4. *rgs_free* the free function get many incomprehension, when we call the free function many people say *it deallocate memory* 
but that is not totally correct, **the heap VMA is organizated in blocks (or this is the common structure)** and when we call
the free function we only set that block to reusable not deallocate, that's also why when we use allocators they give
pointer containing randoms value, because some other program used that block already.

## Installation 

RGS Allocator – Installation & User Guide

This guide will help you install, build, and use the RGS Allocator.

1. Requirements

Before installing, make sure you have the following:
A C compiler (e.g., gcc or clang)

Make utility installed (make)

A Unix-like environment (Linux, macOS, WSL for Windows)

Optional but recommended:
git for cloning the repository
valgrind or similar tools for memory debugging

2. Clone the Repository

Open a terminal and run:
git clone https://github.com/riugxssss/rgs_alloc.git

cd rgs_alloc

**This will download all the source files into a local folder.**

## Technical Problems

My Allocator have some problems, but i wanted to keep it simple because **IT'S ONLY FOR LEARNING** 
1. The thread-safety is weak and sometimes can crash if the stress is too much
2. No use of some advanced techniques for freelist (like Segregated-list)
3. Possible bugs may occure (in my tests seems all okay)

## Off-topic resource

I made this section to say that is my very first "big" project so it's not perfect
but it was fun to do, i suggest this project to only people who programmed in C
for almost 1 - 2 years.
