// //////////////////////////////////////////////////////////////////////////////////////////
// FILE: freelist.h
// 
// AUTHOR: Kirichenko Stanislav
// 
// DATE: 01 jan 2025
// 
// DESCRIPTION: A freelist is a pool allocator that internally tracks free 
// space using a linked list where each allocation has some little overhead, 
// and input buffer must be allocated externally.
// It uses first fit algorithm and does not take into account alignment.
// 
// LICENSE: BSD-2
// Copyright (c) 2025, Kirichenko Stanislav
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions, and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions, and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// MODIFICATIONS ////////////////////////////////////////////////////////////////////////////
// 11 JAN 2025 ~ Kirichenko Stanislav ~ First version.
//
// USAGE ////////////////////////////////////////////////////////////////////////////////////
//
// We want to allocate 1 element of size 16 bytes so we alloc that + 1 allocation overhead
// void* mem;
// size_t size;
// size = 16 + freelist_alloc_overhead();
// mem = malloc(size);
//
// Initialize the freelist now
// freelist_t allocator;
// freelist_initialize(&allocator, mem, size)
//
// Allocate the element
// void* element;
// element = freelist_malloc(&allocator, 16);
//
//
// Do things with element, once done free it
// freelist_free(&allocator, element);
//
//
// Once done with the freelist free it and the buffer
// freelist_reset(&allocator);
// free(mem);
//
// //////////////////////////////////////////////////////////////////////////////////////////


#ifndef INCLUDED_FREELIST
#define INCLUDED_FREELIST

/* Defines a starting point of a block with a size. */
typedef struct freelist_block {
	struct freelist_block* next;
	size_t block_size;
} freelist_block;

/* Defines the freelist allocator. free_block is a linked list of free blocks or null if there aren't free blocks. */
typedef struct {
	void* buffer;
	size_t buffer_size;
	freelist_block* free_block;
} freelist;

#if defined(__cplusplus)
extern "C" {
#endif

	typedef freelist freelist_t;

	/* Overhead for each allocation. */
	size_t freelist_alloc_overhead(void);

	/* Minimum allocation size. */
	size_t freelist_min_alloc_block(void);

	/* Initialize the free list allocator. */
	void freelist_initialize(freelist_t* allocator, void* buffer, size_t poolSize);

	/* Returns the memory buffer. */
	void* freelist_get_buffer(freelist_t* allocator);

	/* Resets the allocator. */
	void freelist_reset(freelist_t* allocator);

	/* Allocates memory from the allocator if has any. */
	void* freelist_malloc(freelist_t* allocator, size_t bytes);

	/* Release memory back to the allocator. */
	void freelist_free(freelist_t* allocator, void* ptr);

	/* Returns the size requested for the allocation of the ptr. */
	size_t freelist_get_allocation_size(freelist_t* allocator, void* ptr);

	/* Check if a pointer is in buffer range. */
	int freelist_range_check(freelist_t* allocator, void* ptr);

	/* Sanity check, to verify if the freelist metadata still has sense. Success is 1 while 0 is error. */
	int freelist_verify_corruption(freelist_t* allocator);

#if defined(__cplusplus)
};
#endif


#endif /*INCLUDED_FREELIST*/
