// //////////////////////////////////////////////////////////////////////////////////////////
// FILE: slice.h
// 
// AUTHOR: Kirichenko Stanislav
// 
// DATE: 3 OCT 2025
// 
// DESCRIPTION: A index based slice allocator.
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
// 3 OCT 2025 ~ Kirichenko Stanislav ~ First version.
//
// USAGE ////////////////////////////////////////////////////////////////////////////////////
//
//
// //////////////////////////////////////////////////////////////////////////////////////////


#ifndef INCLUDED_SLICE
#define INCLUDED_SLICE

#include <stdint.h>

typedef struct {
	size_t offset;
	size_t count;
} slice_t;

/* Defines the slice index allocator. free_slices is a sorted array of free slices or null if there aren't free blocks. */
typedef struct {
	/* The max number of elements to allocate*/
	size_t max_elements;
	/* The addresses must be ordered for binary search*/
	slice_t* free_slices;
	size_t free_slices_array_size;
	size_t free_slices_array_capacity;
} slice_allocator;

#if defined(__cplusplus)
extern "C" {
#endif

	/* Initialize the allocator. */
	void slice_initialize(slice_allocator* allocator, const size_t maxNumOfElements);

	/* Deinitialize the allocator. */
	void slice_destroy(slice_allocator* allocator);

	/* Allocates slice from the allocator if has any. */
	slice_t slice_alloc(slice_allocator* allocator, const size_t count);

	/* Release memory back to the allocator. */
	void slice_free(slice_allocator* allocator, void* ptr);

#if defined(__cplusplus)
};
#endif


#endif /*INCLUDED_SLICE*/
