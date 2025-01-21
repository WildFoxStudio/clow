// //////////////////////////////////////////////////////////////////////////////////////////
// FILE: gpalloc.h
// 
// AUTHOR: Kirichenko Stanislav
// 
// DATE: 19 jan 2025
// 
// DESCRIPTION: A General purpose allocator with aligned allocations with first fit algorithm with binary search on heap allocated array of blocks.
// Free after free detection assert.
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
// 19 JAN 2025 ~ Kirichenko Stanislav ~ First version.
//
// USAGE ////////////////////////////////////////////////////////////////////////////////////
//
//
// //////////////////////////////////////////////////////////////////////////////////////////


#ifndef INCLUDED_GPALLOC
#define INCLUDED_GPALLOC

#include <stdint.h>

typedef struct {
	void* address;
	size_t size : sizeof(size_t) * 8 - 1;  // All bits except the most significant one
	size_t used : 1;                       // 1-bit flag for "used" (MSB)
} gpalloc_allocation;

/* Defines the freelist allocator. free_block is a linked list of free blocks or null if there aren't free blocks. */
typedef struct {
	void* buffer;
	size_t buffer_size;
	/* The addresses must be ordered for binary search*/
	gpalloc_allocation* allocation_array;
	size_t allocation_array_size;
	size_t allocation_array_capacity;
} gpalloc;

#if defined(__cplusplus)
extern "C" {
#endif

	typedef gpalloc gpalloc_t;

	/* Initialize the allocator. */
	void gpalloc_initialize(gpalloc_t* allocator, void* buffer, const size_t poolSize);

	/* Deinitialize the allocator. */
	void gpalloc_destroy(gpalloc_t* allocator);

	/* Allocates memory from the allocator if has any. */
	void* gpalloc_malloc(gpalloc_t* allocator, const size_t bytes, const size_t alignment);

	/* Release memory back to the allocator. */
	void gpalloc_free(gpalloc_t* allocator, void* ptr);

#if defined(__cplusplus)
};
#endif


#endif /*INCLUDED_GPALLOC*/
