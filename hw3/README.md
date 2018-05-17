# Homework 3 Dynamic Memory Allocator - CSE 320 - Spring 2018
#### Professor Eugene Stark

### **Due Date: Friday 3/23/2018 @ 11:59pm**


We **HIGHLY** suggest that you read this entire document, the book chapter,
and examine the base code prior to beginning. If you do not read the entire
document before beginning, you may find yourself doing extra work.

> :scream: Start early so that you have an adequate amount of time to test
your program!

> :scream: The functions `malloc`, `free`, `realloc`, `memalign`, `calloc`,
> etc., are **NOT ALLOWED** in your implementation. If any of these functions,
> or any other function with similar functionality is found in your program,
> you **will receive a <font color="red">ZERO</font>**.

# Introduction

You must read **Chapter 9.9 Dynamic Memory Allocation** before
starting this assignment. This chapter contains all the theoretical
information needed to complete this assignment. Since the textbook has
sufficient information about the different design strategies and
implementation details of an allocator, this document will not cover this
information. Instead, it will refer you to the necessary sections and pages in
the textbook.

## Takeaways

After completing this assignment, you will have a better understanding of:
* The inner workings of a dynamic memory allocator
* Memory padding and alignment
* Structs and linked lists in C
* [errno](https://linux.die.net/man/3/errno) numbers in C
* Unit testing in C

# Overview

You will create a segregated free list allocator, adopting the "Buddy System"
as a memory management policy.

> The allocation policy of this assignment is similar to that described in this
> [wikipedia page](https://en.wikipedia.org/wiki/Buddy_memory_allocation).
> You might find the step-by-step explanation there to be helpful.

In a buddy system, all blocks have sizes that are powers of two,
and each free list holds all the free blocks of one particular size.
A request for a block of memory of a particular size is satisfied by obtaining
a block from the free list that holds blocks just big enough to accomodate the
requested size (plus overhead for a block header).
If a block of a particular size is required, but there are currently no free
blocks of that size, then a free block of the next larger size is obtained,
it is split in half, and the two halves are inserted into the free list.
This process continues recursively, up to the largest supported block size.
If there are no free blocks of the largest size, then the operating system
is requested to increase the size of the heap to create a new free block.

When a block is freed under buddy memory allocation, an immediate attempt
at coalescing is made.  Coalescing of a newly freed block is possible if and
only if the "buddy" of the block is already free.  The buddy of a block is
an immediately adjacent block of the same size, whose address bears a special
relationship to that of its neighbor.  In particular, the buddy of a block
having address `A` and size `S` is the block whose address is obtained as the
bitwise exclusive-OR `A^S` of the address `A` and size `S`.  As the size
`S` is a power of two, this exclusive-OR amounts to toggling one bit of
the address `A`.  For example, the buddy of the block with address `0xaa0200`
and size 512 (i.e. `0x200`) is the block with address `0xaa0000`, and vice versa.
These two blocks can be combined to yield a single block with address
`0xaa0000` and size 1024.
A block formed by coalescing two blocks is itself subjected to a coalescing
attempt.  This continues until a block is obtained that cannot be coalesced
because its buddy is not currently free.  The resulting block is added to
the free list appropriate for its size.

Using these basic ideas, you are to implement your own versions of the **malloc**,
**realloc**, and **free** functions.  To avoid conflict with the system-defined
functions of the same names, the functions you will implement will be called
`bud_malloc`, `bud_realloc`, and `bud_free`.  These functions are to satisfy
specifications given in more detail below.

To help debug and test your implementation, you can use some Criterion unit
tests that we have supplied.  You will also write additional tests of your own
to verify that your implementation functions correctly.

# Detailed Description

As outlined above, in the buddy memory allocation technique, block sizes are
always powers of two.  Because of this, we can describe the size of a block by
its **order**; that is, the logarithm to the base 2 of its size.
For example, a block of size 32 has order 5, a block of size 64 has order 6,
and so on.

The file `budmm.h` provided to you as part of the base code for this assignment
defines various constants pertaining to the allocator you are to implement.

```c
#define ORDER_MIN 5  /* Min block size 32 */
#define ORDER_MAX 15
#define ORDER_TO_BLOCK_SIZE(ord) (1 << (ord))
#define MIN_BLOCK_SIZE (ORDER_TO_BLOCK_SIZE(ORDER_MIN))
#define MAX_BLOCK_SIZE (ORDER_TO_BLOCK_SIZE(ORDER_MAX-1))
#define MAX_HEAP_SIZE (4 * ORDER_TO_BLOCK_SIZE(ORDER_MAX-1))
#define NUM_FREE_LIST (ORDER_MAX - ORDER_MIN)
```

The constant `ORDER_MIN` defines the order of the smallest block your allocator
should handle.
The constant `ORDER_MAX` has a value that is one greater than the order of the
largest block that your allocator should handle.
The difference `ORDER_MAX - ORDER_MIN` is the number of different block size
classes that your allocator will support.  Since each block class has its own
free list, this is also the number of free lists, and a constant `NUM_FREE_LIST`
has been defined for that.
The macro `ORDER_TO_BLOCK_SIZE` maps an order to the corresponding block size.
The constant `MAX_HEAP_SIZE` defines the maximum size to which your heap can grow.
Note that you are not to make any changes to the `budmm.h` header file or the
`budutil.c` source file.  In addition, your code should not make any assumptions
about the specific values of the constants defined in `budmm.h`.
We may decide, for example, to change the values of `ORDER_MIN`, `ORDER_MAX`,
and `MAX_HEAP_SIZE` when testing your code.

The header file `budmm.h` also defines two structures that are used for maintaining
information about memory blocks: `bud_header` and `bud_free_block`.
The `bud_header` structure is defined as follows:

```c
/* Struct for a block header (64 bits) */
typedef struct {
    uint64_t allocated : 1;
    uint64_t     order : 4;
    uint64_t    padded : 1;
    uint64_t   unused1 : 10;
    uint64_t     rsize : 16;
    uint64_t   unused2 : 32;
} __attribute__((packed)) bud_header;

/*
                                 Format of an allocated memory block
    +---------------------------------------------------------------------------------------+
    |                                      64-bits wide                                     |
    +---------------------------------------------------------------------------------------+

    +-------------------------------------------+----------------+-------+------+-----+-----+ <- header
    |                unused                     | requested_size |unused |padded|order|alloc|
    |                                           |                |       |  1   |  4  |  1  |
    |                32 bits                    |    16 bits     |10 bits| bits | bits| bit |
    +-------------------------------------------+----------------+-------+------+-----+-----+
    |                                                                                       |
    |                                   Payload and Padding                                 |
    |                                                                                       |
    +---------------------------------------------------------------------------------------+
*/
```

The fields of this structure mostly record information about a block of memory
that is needed whether or not the block is allocated or free.
The `allocated` field records the allocation status (1 = allocated, 0 = unallocated)
of the block.  The `order` field records the order of the block.
The `padded` field records whether the block has been padded; i.e. whether
the requested size plus the size of the header is less than the actual
size of the block.
For an allocated block, the `rsize` field holds the requested size
(i.e. the size of the payload) for that block.

The `bud_free_block` structure is defined as follows:
```c
/* Struct for a free block */
typedef struct bud_free_block {
    bud_header header;
    struct bud_free_block *next;
    struct bud_free_block *prev;
} bud_free_block;

/*
                                      Format of a free block
    +---------------------------------------------------------------------------------------+
    |                                      64-bits wide                                     |
    +---------------------------------------------------------------------------------------+

    +-------------------------------------------+----------------+-------+------+-----+-----+ <- header
    |                unused                     | requested_size |unused |padded|order|alloc|
    |                                           |                |       |  1   |  4  |  1  |
    |                32 bits                    |    16 bits     |10 bits| bits | bits| bit |
    +-------------------------------------------+----------------+-------+------+-----+-----+
    |                                      next pointer                                     |
    |                                                                                       |
    |                                         64 bits                                       |
    +---------------------------------------------------------------------------------------+
    |                                      prev pointer                                     |
    |                                                                                       |
    |                                         64 bits                                       |
    +---------------------------------------------------------------------------------------+
    |                                                                                       |
    |                                        FREE AREA                                      |
    |                                                                                       |
    +---------------------------------------------------------------------------------------+
*/
```

This structure defines information about a block that is currently free.
Besides the header, there are `next` and `prev` pointer fields that are
used to link the block into a doubly linked free list, as described in
more detail below.

Note that the `bud_header` or `bud_free_block` structure will always be
stored at the beginning of a block.  A pointer to the beginning of a block
can be cast either to type `bud_header *` or `bud_free_block *` to allow
access to the fields of either structure, depending on whether the block
is currently allocated or free.

The array `free_list_heads` contains the heads of the free lists for
the allocator.  There are `NUM_FREE_LIST` entries in this array, one
for each order in the range `[ORDER_MIN, ORDER_MAX)`.
Each free list should contain only blocks of one particular order;
e.g. the list headed by `free_list_heads[0]` will contain only free
blocks of order `ORDER_MIN` (32 bytes), and the list headed by
`free_list_heads[3]` will contain only free blocks of order
`ORDER_MIN+3` (256 bytees).

For maintaining the free lists, you are to use *circular, doubly linked lists*,
with the entries of the `free_list_heads` array serving as *sentinels*
(google "circular doubly linked lists with sentinel" if you need an introduction
to this data structure).
The sentinel is a special node that serves to mark  both the beginning
and the end of the list.  It is never removed.
An empty list contains only of the sentinel node, whose `next` and `prev`
pointers refer back to itself.
The function `bud_mem_init` in `bud_util.c` initializes the free lists
in this condition.
Inserting into and deleting from a circular doubly linked list is done
in the usual way, except that, owing to the use of the sentinel, there
are no edge cases for inserting or removing at the beginning or the end
of the list.

> :scream: You **MUST** use the entries of the `free_list_heads` array
> as the sentinel nodes for the free lists, and you **MUST** implement
> these lists as circular, doubly linked lists.
> The helper functions discussed later, as well as the unit tests,
> assume that you have done this when accessing your free lists.

For our purposes, all insertions and deletions from a free list will be
done at the beginning of the list (resulting in a LIFO discipline).
So inserting into a list would set the `next` field of the sentinel to be
the inserted node, and removing from a list would likewise modify the
`next` field of the sentinel.

The header `budmm.h` also contains function prototypes for several
utility functions that have been provided to you in `budutil.c`.

> :scream: <font color="red"> You must not modify anything in `budutil.c`.</font>
> However, it is encouraged to read through the functions in this file
> to understand how the heap gets extended.

The provided utility functions are:

```c
/*
 * Any program using the budmm library must call this function ONCE
 * before issuing any allocation requests. This function DOES NOT
 * allocate any space to your allocator.
 */
void bud_mem_init();
```

You must call `bud_mem_init` before doing anything else.
This function initializes the free lists and some internal state
variables, but it does not allocate any space to your heap.
In order to get space in the heap, you will need to use the
`bud_sbrk` function described below.

```c
/*
 * Any program using the budmm library must call this function ONCE
 * after all allocation requests are complete. If implemented cleanly,
 * your program should have no memory leaks in valgrind after this function
 * is called.
 */
void bud_mem_fini();
```

As the comment indicates, you must call this function once before your
program terminates.

```c
/*
 * This function changes the position of your program's break.
 * Each time it is called, the break pointer is increased by BLOCK_SIZE_MAX.
 *
 * @return On success, this function returns the previous program break.
 * If the break was increased, this value is a pointer to the start of the newly
 * allocated memory. On error, (void *)-1 is returned and errno is set to ENOMEM.
 */
void *bud_sbrk();
```

The `bud_sbrk` function is used to increase the size of the heap by moving the
heap break pointer.  For this assignment, your implementation **MUST ONLY** use
`bud_sbrk` to do this.  *DO NOT* attempt to use any system calls such as **brk**
or **sbrk** to move the break pointer.

> :smile: A real allocator may use the **brk**/**sbrk** syscalls for small
> memory allocations and **mmap**/**munmap** syscalls for large allocations. To
> allow your program to use other functions provided by glibc, that rely on
> glibc's allocator, we prove a safe wrapper around **sbrk**. This makes it so
> that the heap breakpoint does not get altered by glibc's allocator and
> destroy the memory managed by your allocator.

Each time `bud_sbrk` is called, the break pointer is advanced by `MAX_BLOCK_SIZE`,
which is the maximum size of a block that your allocator can handle,
until the total size of the heap has reached `MAX_HEAP_SIZE`, at which point
further calls to `bud_sbrk` will fail and return `(void *)-1`.
A successful call to `bud_sbrk` returns the old break pointer, which is a pointer
to the newly created block.

The `bud_sbrk` function keeps track of the starting and ending addresses of
the heap for you. You can get these addresses through the `get_heap_start` and
`get_heap_end` functions:

```c
/*
 * @return The starting address of the heap for your allocator.
 */
void *get_heap_start();

/*
 * @return The ending address of the heap for your allocator.
 */
void *get_heap_end();
```

Note that, although the address returned by `get_heap_start` is a valid
address in the heap, namely the first address in the heap, the address returned
by `get_heap_end` is the the first address *beyond the end* of the current heap,
which is not a valid address in the heap.  This way of defining a memory region
is usually the most convenient way to do it.

The remaining function prototypes in `budmm.h` are for the functions you are to write.
You will implement the following three functions in the file `src/budmm.c`.
The file `include/budmm.h` contains the prototypes and documentation found here.

> :scream: <font color="red">Make sure these functions have these exact names
> and arguments. They must also appear in the correct file. If you do not name
> the functions correctly with the correct arguments, your program will not
> compile when we test it. **YOU WILL GET A ZERO**</font>

```c
/*
 * This is your implementation of bud_malloc. It acquires uninitialized memory that
 * is aligned and padded properly for the underlying system.
 *
 * @param size The number of bytes requested to be allocated.
 *
 * @return If successful, a pointer to a valid region of memory of the
 * requested size is returned, else NULL is returned and errno as follows:
 *
 * If the size passed is invalid, errno is set to EINVAL.
 * If the request cannot be satisfied, errno is set to ENOMEM.
 */
void *bud_malloc(uint32_t size);
```

## Notes on `bud_malloc`

When implementing your `bud_malloc` function, first determine if the request size
is valid.  The request size is invalid if it is 0 or it is greater than
`MAX_BLOCK_SIZE - sizeof(bud_header)`.  In the case of an invalid size,
set `errno` to `EINVAL` and return `NULL`.  For a valid size, you will return either
a valid pointer in case of success, or `NULL` in case the request could not be satified.
In the latter case, `errno` should be set to `ENOMEM`.

Once the request parameters have been validated, you should attempt to satisfy
the request with the smallest block size that accommodates the request
(taking into account the header overhead).
For example, a request of size 200 should be satisfied by a block of size 256.
A request of size 256 will have to be satisfied by a block of size 512, in order
to have room for the header.  Note that there will be a unique matching block size
for any valid request size.  If there do not currently exist any free blocks of
the appropriate size, then the request should be satisfied by obtaining a free block
of the next higher size, splitting it in half, placing the half with the higher
address in the free list and using the half with the lower address to satisfy the
request.  Note that if there are no free blocks of the next higher size, then
blocks of the next higher size after that should be considered, and so on,
up to blocks of size MAX_BLOCK_SIZE.  Each time you use a larger block
to satisfy a smaller request, the larger block will be split in half and the
unused half will be left in the appropriate free list.
If there are no blocks in any free list up to size `MAX_BLOCK_SIZE`, then you should
call `bud_sbrk` to attempt to extend the heap and obtain a new block of size
`MAX_BLOCK_SIZE`.
If this fails, then you must set `errno` to `ENOMEM` and return `NULL`.

The pointer returned by `bud_malloc` is to the *payload area* of the
newly allocated block, which starts immediately after the header.

```
/*
 * Marks a dynamically allocated region as no longer in use.
 * Adds the newly freed block to the free list.
 *
 * @param ptr Address of memory returned by the function bud_malloc.
 *
 * If bud_free is called with an invalid ptr, it should call abort()
 * to exit the program.
 */
void bud_free(void *ptr);
```

## Notes on `bud_free`

When implementing `bud_free`, you must first verify that the pointer being
passed to your function belongs to an allocated block. This can be done by
examining the pointer address itself and the fields in the pointed-at header.
For this assignment, a pointer is to be regarded as invalid if any of the
following hold:

- The pointer does not correspond to an address in the range defined by
  the return values of `bud_heap_start()` and `bud_heap_end()`
- The pointer is not aligned to a multiple of 8.
- The value in the `order` field does not lie in the range `[ORDER_MIN, ORDER_MAX)`
- The `allocated` bit in the header is 0
- The `padded` bit in the header is 0,
  but `requested_size + sizeof(bud_header) != (block size)`
- The `padded` bit in the header is 1,
  but `requested_size + sizeof(bud_header) == (block size)`
- The `requested_size` and `order` don't make sense; e.g.
  `requested_size` is 100, but `order` is 6 (it is supposed to be 7)

If an invalid pointer is passed to your function, you must call `abort` to exit
the program. Use the man page for the `abort` function to learn more about this.

After confirming that a valid pointer was given, you must free the block.
Also, you should check if the newly freed block can be coalesced with its buddy.
See the [Immediate Coalescing](#immediate-coalescing) section for more details.

```c
/*
 * Resizes the memory pointed to by ptr to size bytes.
 *
 * @param ptr Address of the memory region to resize.
 * @param size The minimum size to resize the memory to.
 *
 * @return If successful, a pointer to a valid region of memory is
 * returned, else NULL is returned and errno is set appropriately.
 *
 * A call to bud_realloc with a ptr == NULL should be equivalent to a
 * call to bud_malloc(size).
 * A call to bud_realloc with size == 0 should be equivalent to a
 * call to bud_free(ptr).
 * If bud_realloc is called with an invalid ptr it should call abort()
 * to exit the program.
 * If there is no memory available bud_realloc should set errno to ENOMEM.
 */
void *bud_realloc(void *ptr, uint32_t size);
```

## Notes on `bud_realloc`

When implementing your `bud_realloc` function, you must first verify that the
pointer and size parameters passed to your function are valid.
If the size is 0, free the block and return `NULL`.
If a null pointer is passed, then `bud_realloc` should behave like a call
to `bud_malloc` of the same size.
Otherwise, the criteria for validity of the size and pointer are the same as
those described under [Notes on `bud_malloc`](#notes-on-bud_malloc)
and [Notes on `bud_free`](#notes-on-bud_free).

After verifying both parameters, you should determine the size of the new block
that best accomodates the requested size.  If the calculated size is same as
the current block size, `bud_realloc` should do nothing and just return the
same pointer as was passed in.

If the calculated size is bigger than the existing block size, it is necessary
to allocate a new block.  This should be done as for `bud_malloc`.
Once the new block has been allocated, the payload data should be copied
(using `memcpy`) from the old block to the new block and the old block should
be freed before returning a pointer to the payload area of the new block.

If the calculated size is smaller than the existing block size,
the pointer that is returned should be the same as the one that was passed as a
parameter, except that before doing so the block should be split in half if
it is at least twice as large as necessary.  The unused half should be placed
on the appropriate free list and the order of the block should be reduced by
one.  If the resulting block is still at least twice as large as necessary,
it should be split again, and so on, until the final block is the minimum size
that can accomodate the payload and header.  All the unused portions of the
block should end up on the various free lists.

> As mentioned in the [Splitting Blocks](#splitting-blocks) section,
> the **left** buddy should be chosen for the next split target for each splitting step.
> In the end, your allocator will select the leftmost child block for allocation;
> this means that `bud_realloc` will return the same pointer as it was passed
> as a parameter.

As for `bud_malloc` the pointer returned by `bud_realloc` is to the payload area
of the newly allocated block.

## Block Size and Requested Size

The `requested size` of `bud_malloc` and `bud_realloc` is the number of bytes in memory
that the user wants to use.  A certain range of requested sizes can be matched to the same
**actual** block size. e.g. for requested size of 150 and 200, they will both get the blocks
of the same size, 256 bytes. The `header` of each block occupies the first 8 bytes of each
block in this assignment.
Thus, you cannot allocate a 256-byte free block in response to a request of 256 bytes,
as there will be no room to store the header.
A 512-byte block would have to be allocated in this case.

## Splitting Blocks

In practice, allocators typically split blocks to reduce the amount of internal fragmentation.
To satisty the buddy policy of this assignment, your allocator only ever splits a block
in half.  For example, suppose that the requested size of `bud_malloc` is 50, but the only
free block that currently exists is a 256-byte block. The free block is split into two free blocks
of 128 bytes. Then, the **left** one of the split blocks (i.e. the one with the
lower-numbered address) is split again to become two 64 byte free blocks.
The **left** 64 byte free block then will be used for the allocation.
These steps of splitting blocks have resulted in one 64-byte allocated block,
one 64-byte free block, and one 128-byte free block. Of course, the free blocks
produced by each step are put into the proper free lists.

As in the example above, you should always select the **left** buddy for a target of
next split or allocation.

Refer to the textbook for a general explanation of splitting blocks.

## Immediate Coalescing

Coalescing must **only** be done when `bud_free` is called, or in some cases when
`bud_realloc` is called.
According to the buddy policy, you should only coalesce the two free blocks
when they are **buddy**.  As stated above, blocks of size `S` at addresses
`A` and `B` are buddy exactly when `A^S = B` (equivalently `B^S = A`).
Remember that the buddy of a block must be of the same size.
If the two have different sizes, they are not buddy.

Refer to the textbook for a general explanation of coalescing.

# Getting Started

Fetch and merge the base code for `hw3` as described in `hw0` from the
following link: https://gitlab02.cs.stonybrook.edu/cse320/hw3

**Remember to use the `--strategy-option theirs` flag with the `git merge`
command as described in the `hw1` doc to avoid merge conflicts in the Gitlab
CI file.**

## Directory Structure

<pre>
.
├── .gitlab-ci.yml
└── hw3
    ├── include
    │   ├── budmm.h
    │   ├── budprint.h
    │   └── debug.h
    ├── Makefile
    ├── src
    │   ├── budmm.c
    │   ├── budprint.c
    │   ├── budutil.c
    │   └── main.c
    └── tests
        └── budmm_tests.c
</pre>

The program in `src/main.c` contains a basic example of using the initialization
and allocation functions together. Running `make` will create a `budmm`
executable in the `bin` directory. This can be run using the command `bin/budmm`.

The code in `src/budprint.c` (with corresponding header file `include/budprint.h`)
contains some print functions that you might find useful while debugging your code.
As these functions directly call `fprintf`, rather than using the `debug` macros,
you should not leave any calls to these functions in the final code that you submit.

The provided `Makefile` creates object files from the `.c` files in the `src`
directory, places the object files inside the `build` directory, and then links
the object files together to make executables, located in the `bin` directory.

> Any functions other than `bud_malloc`, `bud_free`, and `bud_realloc` **WILL NOT**
be graded.

> **DO NOT modify `budmm.h`, `budutil.c`, or the Makefile.** Both will be replaced when we run
> tests for grading. If you wish to add things to a header file, please create
> a new header file in the `include` folder.

# Unit Testing

For this assignment, we will use Criterion to test your allocator. We have
provided a basic set of test cases and you will have to write your own as well.
You will use the Criterion framework alongside the provided helper functions to
ensure your allocator works exactly as specified.

In the `tests/budmm_tests.c` file, there are nine unit test examples. These tests
check for the correctness of `bud_malloc`, `bud_realloc`, and `bud_free`.
We provide some basic assertions, but by no means are they exhaustive.
It is your job to ensure that your header bits are set correctly and that blocks are
allocated/freed as specified.

## Compiling and Running Tests

When you compile your program with `make`, a `budmm_tests` executable will be
created in the `bin` folder alongside the `main` executable. This can be run
with `bin/budmm_tests`. To obtain more information about each test run, you can
use the verbose print option: `bin/budmm_tests --verbose=0`.

## Writing Criterion Tests

The first test `easy_malloc_a_pointer` tests `bud_malloc`. It allocates space
for an integer and assigns a value to that space. It then runs an assertion to make
sure that the space returned by `bud_malloc` was properly assigned.

```c
cr_assert(*x == &a, "bud_malloc failed to give proper space for a pointer!");
```

The string after the assertion only gets printed to the screen if the assertion
failed (i.e. `*x != 4`). However, if there is a problem before the assertion,
such as a SEGFAULT, the unit test will print the error to the screen and
continue to run the rest of the unit tests.

## Your Additional Tests

For this assignment **you must write 5 additional unit tests
which test new functionality** and add them to `budmm_tests.c`
below the following comment:

```
//############################################
//STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
//DO NOT DELETE THESE COMMENTS
//############################################
```

> For additional information on Criterion library, take a look at the official
> documentation located [here](http://criterion.readthedocs.io/en/master/)! This
> documentation is VERY GOOD.

# Hand-in instructions
Make sure your directory tree looks like this and that your homework compiles.

<pre>
.
├── .gitlab-ci.yml
└── hw3
    ├── include
    │   ├── budmm.h
    │   ├── budprint.h
    │   └── debug.h
    ├── Makefile
    ├── src
    │   ├── budmm.c
    │   ├── budprint.c
    │   ├── budutil.c
    │   └── main.c
    └── tests
        └── budmm_tests.c
</pre>

This homework's tag is: `hw3`

<pre>
$ git submit hw3
</pre>

> :nerd: When writing your program try to comment as much as possible. Try to
> stay consistent with your formatting. It is much easier for your TA and the
> professor to help you if we can figure out what your code does quickly.
