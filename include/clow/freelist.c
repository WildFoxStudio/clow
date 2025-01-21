// //////////////////////////////////////////////////////////////////////////////////////////
// FILE: freelist.c
// 
// AUTHOR: Kirichenko Stanislav
// 
// DATE: 01 jan 2025
// 
// DESCRIPTION: A freelist is a pool allocator that internally tracks free 
// space using a linked list where each allocation has some little overhead, 
// and input buffer must be allocated externally.
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
// //////////////////////////////////////////////////////////////////////////////////////////

#include "clow/freelist.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Punning void* for c and c++
#ifdef __cplusplus
#include <new>  // placement new

#define pun_cpy(dest, type, source) new((void*)dest) type{*source}

#else

#define pun_cpy(dest, type, source) memcpy(dest, source, sizeof(*source))

#endif

// The header preceding each allocation freelist_block
typedef struct
{
	size_t size;

}freelist_header;



static void* freelist_offset_ptr(void* begin, size_t offset)
{
	return (void*)(((uintptr_t)begin) + offset);
}

static void* freelist_subtract_ptr(void* begin, size_t offset)
{
	return (void*)(((uintptr_t)begin) - offset);
}

/* When error occurred returns 0, when nothing wrong is detected 1. */
static int verify_freelist(freelist* const allocator, freelist_block* current)
{
	size_t blocks_sum;
	blocks_sum = 0;
	while (current != NULL)
	{
		// This could mean that externally an allocation has written outside the requested bytes and corrupted internal metadata.
		if (current->block_size > allocator->buffer_size && "Single block can't be bigger than the whole buffer!")
			return 0;
		if (current->block_size == 0 && "Must not have free blocks of size 0!")
			return 0;

		blocks_sum += current->block_size;
		// Advance
		current = current->next;
	}
	assert(blocks_sum <= allocator->buffer_size);

	return 1;
}



/* Debug asserts for internal corruption */
#ifdef _DEBUG
#define verify(allocator, freelist_block) assert(verify_freelist(allocator, freelist_block) == 1);

static void assert_ptr_in_free_block(freelist* const allocator, void* ptr)
{
	//freelist_block* block;

	((void)ptr);
	//block = allocator->free_block;

	//while (block != NULL)
	//{
	//	
	//	assert(((uintptr_t)freelist_subtract_ptr(ptr, sizeof(freelist_header))) < (uintptr_t)block && freelist_subtract_ptr(ptr, sizeof(freelist_header)) > (uintptr_t)block && "Ptr must not be at header location!");
	//	// Advance
	//	block = block->next;
	//}
}

#else
#define verify(allocator, freelist_block) 
#endif

/* Merge multiple consequent blocks into a single one */
static void coalescence(freelist* const allocator, freelist_block* current) {
	((void)allocator);
	verify(allocator, allocator->free_block)

		while (current && ((void*)current->next) == (freelist_offset_ptr(current, current->block_size)) && current < current->next) {
			assert(current->block_size <= allocator->buffer_size);
			assert(current->block_size > 0 && current->next->block_size > 0);
			assert(current->next->block_size <= allocator->buffer_size && "Next block is corrupted!");
			assert(current->block_size + current->next->block_size <= allocator->buffer_size && "Next block is corrupted!");
			current->block_size += current->next->block_size;
			assert(current->block_size <= allocator->buffer_size && "BlockSize can't be bigger than the memory pool");
			current->next = current->next->next;
			assert(allocator->free_block->next != allocator->free_block && "Reference to self, pointer was already released");
		}

	verify(allocator, current)
}



size_t freelist_alloc_overhead(void) {
	return sizeof(freelist_header);
}

size_t freelist_min_alloc_block(void) {
	return sizeof(freelist_block);
}

void freelist_initialize(freelist_t* allocator, void* buffer, size_t poolSize) {
	freelist fl;
	freelist_block temp_block;
	assert(allocator != NULL);
	assert(buffer != NULL);
	assert(poolSize >= freelist_min_alloc_block() && "Memory size must be equal or greater than min_alloc_block");


	fl.buffer = buffer;
	fl.buffer_size = poolSize;
	fl.free_block = (freelist_block*)buffer;
	temp_block.next = NULL;
	temp_block.block_size = poolSize;
	pun_cpy(fl.free_block, freelist_block, &temp_block);
	pun_cpy(allocator, freelist, &fl);

	verify(allocator, allocator->free_block)
}

void* freelist_get_buffer(freelist_t* allocator)
{
	assert(allocator != NULL);
	return allocator->buffer;
}

void freelist_reset(freelist_t* allocator) {
	assert(allocator != NULL);
	((void)allocator);

	//Do nothing

	//freelist* alloc{ (freelist*)allocator };
	//alloc->buffer = NULL;
	//alloc->buffer_size = 0;
	//alloc->blockSize = 0;
	//alloc->free_block = NULL;
}

void* freelist_malloc(freelist_t* allocator, size_t bytes) {
	freelist* alloc;
	void* result;
	freelist_block* newNode;
	freelist_block temp_block;
	freelist_header header;
	assert(allocator != NULL);
	verify(allocator, allocator->free_block)

		alloc = (freelist*)allocator;

	assert(bytes >= freelist_min_alloc_block() && "Memory size must be equal or greater than min_alloc_block");
	if (!alloc->free_block || alloc->free_block->block_size < bytes + freelist_alloc_overhead())
	{
		//Requesting more memory than available
		return NULL;
	}
	assert(alloc->free_block != NULL && alloc->free_block->block_size > 0 && "All memory is being used");

	if (alloc->free_block->block_size >= bytes + freelist_alloc_overhead()) {
		alloc->free_block->block_size -= bytes + freelist_alloc_overhead();
		header.size = bytes;

		if (alloc->free_block->block_size == 0) {
			result = alloc->free_block;

			alloc->free_block = alloc->free_block->next;

			memcpy(result, &header, sizeof(freelist_header));
			result = freelist_offset_ptr(result, freelist_alloc_overhead());

			assert(alloc->free_block == NULL || (alloc->free_block != NULL && alloc->free_block->block_size > 0 && "All memory is being used"));
			verify(allocator, allocator->free_block)
				return result;
		}
		else {
			assert(freelist_range_check(alloc, (void*)alloc->free_block));

			newNode = (freelist_block*)freelist_offset_ptr(alloc->free_block, bytes + freelist_alloc_overhead());
			assert(freelist_range_check(alloc, (void*)newNode));

			temp_block.next = alloc->free_block->next;
			temp_block.block_size = alloc->free_block->block_size;

			pun_cpy(newNode, freelist_block, &temp_block);

			result = alloc->free_block;
			memcpy(result, &header, sizeof(freelist_header));
			result = freelist_offset_ptr(result, freelist_alloc_overhead());

			alloc->free_block = newNode;
			assert(alloc->free_block != NULL && alloc->free_block->block_size > 0 && "All memory is being used");

			verify(allocator, allocator->free_block)
				return result;
		}
	}

	return NULL;
}

int freelist_range_check(freelist_t* allocator, void* ptr) {
	return ptr >= allocator->buffer && ptr < freelist_offset_ptr(allocator->buffer, allocator->buffer_size);
}

void freelist_free(freelist_t* allocator, void* ptr) {
	freelist* alloc;
	freelist_block temp_block;
	freelist_header* header;
	assert(allocator != NULL);

	if (!ptr)
		return;

	// Do nothing if pointer is outside the buffer range"
	if (freelist_range_check(allocator, ptr) > 0)
	{

		verify(allocator, allocator->free_block)

			alloc = (freelist*)allocator;
		header = (freelist_header*)freelist_subtract_ptr(ptr, freelist_alloc_overhead());

		temp_block.next = alloc->free_block;
		temp_block.block_size = header->size + sizeof(freelist_header);
		pun_cpy((void*)header, freelist_block, &temp_block);
		assert(((freelist_block*)header)->block_size <= allocator->buffer_size && "Next block is corrupted!");

		alloc->free_block = (freelist_block*)header;

		assert(alloc->free_block->block_size == temp_block.block_size);
		assert(alloc->free_block->next != alloc->free_block && "Reference to self, pointer was already released");
		coalescence(alloc, alloc->free_block);
	}
}

size_t freelist_get_allocation_size(freelist_t* allocator, void* ptr)
{
	freelist_header* header;
	assert(allocator != NULL);
	assert(freelist_range_check(allocator, ptr) && "Pointer must be inside the buffer range");

	if (!ptr)
		return 0;

#ifdef _DEBUG
	assert_ptr_in_free_block(allocator, ptr);
#endif

	header = (freelist_header*)freelist_subtract_ptr(ptr, freelist_alloc_overhead());
	assert(header->size <= allocator->buffer_size && "Header is corrupted!");

	return header->size;
}

int freelist_verify_corruption(freelist_t* allocator)
{
	return verify_freelist(allocator, allocator->free_block);
}




