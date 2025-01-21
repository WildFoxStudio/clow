// //////////////////////////////////////////////////////////////////////////////////////////
// FILE: gpalloc.c
// 
// AUTHOR: Kirichenko Stanislav
// 
// DATE: 19 jan 2025
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

#include "clow/gpalloc.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#pragma region Private

// Punning void* for c and c++
#ifdef __cplusplus
#include <new>  // placement new

#define pun_cpy(dest, type, source) new((void*)dest) type{*source}

#else

#define pun_cpy(dest, type, source) memcpy((void*)dest, (void*)source, sizeof(type))

#endif


static void* gpalloc_offset_ptr(void* begin, size_t offset)
{
	return (void*)(((uintptr_t)begin) + offset);
}

static void* gpalloc_subtract_ptr(void* begin, size_t offset)
{
	return (void*)(((uintptr_t)begin) - offset);
}

static size_t gpalloc_ptr_diff(void* begin, void* end)
{
	assert(((uintptr_t)end) >= ((uintptr_t)begin));
	return ((uintptr_t)end) - ((uintptr_t)begin);
}

void* gpalloc_align(void* ptr, const size_t alignment) {
	uintptr_t addr = (uintptr_t)ptr;
	if (alignment && (alignment & (alignment - 1)) == 0) {
		return (void*)((addr + (alignment - 1)) & ~(alignment - 1));
	}
	return NULL;  // Alignment must be a power of two
}

inline size_t gpalloc_max(const size_t a, const size_t b) {
	return (a > b) ? a : b;
}

void gpalloc_clear_out_of_size(gpalloc_t* allocator)
{
	memset(allocator->allocation_array + allocator->allocation_array_size, 0, sizeof(gpalloc_allocation) * (allocator->allocation_array_capacity - allocator->allocation_array_size));
}

void gpalloc_check_for_duplicates(gpalloc_t* allocator)
{
	size_t i;
	for (i = 0; i < allocator->allocation_array_size; i++)
	{
		size_t j;
		for (j = 0; j < allocator->allocation_array_size; j++)
		{
			if (i == j)
				continue;
			gpalloc_allocation* a = allocator->allocation_array + i;
			gpalloc_allocation* b = allocator->allocation_array + j;
			assert(a->address != b->address && "Must not exists two elements with same address!");
		}
	}
}

void gpalloc_grow_array(gpalloc_t* allocator, const size_t new_capacity)
{
	if (new_capacity > allocator->allocation_array_capacity)
	{
		allocator->allocation_array_capacity = new_capacity;
		if (allocator->allocation_array == NULL)
		{
			allocator->allocation_array = (gpalloc_allocation*)malloc(allocator->allocation_array_capacity * sizeof(gpalloc_allocation));
			gpalloc_clear_out_of_size(allocator);
		}
		else
		{
			allocator->allocation_array = (gpalloc_allocation*)realloc((void*)allocator->allocation_array, allocator->allocation_array_capacity * sizeof(gpalloc_allocation));
			gpalloc_clear_out_of_size(allocator);
		}
	}
}

void gpalloc_emplace(gpalloc_t* allocator, gpalloc_allocation allocation)
{
	gpalloc_grow_array(allocator, ++allocator->allocation_array_size);
	assert(allocator->allocation_array_size + 1 <= allocator->allocation_array_capacity);

	pun_cpy((allocator->allocation_array + allocator->allocation_array_size - 1), gpalloc_allocation, &allocation);

	gpalloc_check_for_duplicates(allocator);
}

void gpalloc_insert(gpalloc_t* allocator, const size_t index, gpalloc_allocation allocation)
{
	allocator->allocation_array_size++;
	if (allocator->allocation_array_size >= allocator->allocation_array_capacity)
		gpalloc_grow_array(allocator, allocator->allocation_array_size);

	size_t i;
	for (i = allocator->allocation_array_size - 1; i > index; i--)
	{
		// Move all right from the end
		memmove(allocator->allocation_array + i, allocator->allocation_array + i - 1, sizeof(gpalloc_allocation));
	}

	// Copy element at index
	pun_cpy((allocator->allocation_array + index), gpalloc_allocation, &allocation);

	gpalloc_check_for_duplicates(allocator);
}


void gpalloc_erase_at(gpalloc_t* allocator, const size_t index)
{
	assert(index < allocator->allocation_array_size);
	assert(allocator->allocation_array_size > 0);

	allocator->allocation_array_size--;

	size_t i;
	for (i = index; i <= allocator->allocation_array_size - 1; i++)
	{
		// Move all left from the index
		memmove(allocator->allocation_array + i, allocator->allocation_array + i + 1, sizeof(gpalloc_allocation));
	}
	gpalloc_clear_out_of_size(allocator);
}


void gpalloc_pop(gpalloc_t* allocator)
{
	if (allocator->allocation_array_size > 0)
	{
		allocator->allocation_array_size--;
		gpalloc_clear_out_of_size(allocator);
	}
}

size_t gpalloc_lower_bound(gpalloc_t* allocator, void* ptr)
{
	const uintptr_t address = (uintptr_t)ptr;

	size_t count = allocator->allocation_array_size;
	size_t first = 0;
	while (0 < count)// divide and conquer, find half that contains answer
	{

		const size_t count2 = count / 2;
		const size_t mid = first + count2;

		const gpalloc_allocation* const allocation = allocator->allocation_array + mid;
		const uintptr_t block_address = ((uintptr_t)allocation->address);
		if (block_address < (uintptr_t)address)//try top half
		{
			first = mid + 1;
			count -= count2 + 1;
		}
		else
		{
			count = count2;
		}
	}
	return first;
}

/* See if index-1 and index +1 can be merged with index */
void gpalloc_coalescence(gpalloc_t* allocator, size_t index)
{

	/* Blocks must be contiguos to do this */
	gpalloc_allocation* current = allocator->allocation_array + (index);
	assert(current->used == false && "Must be free!");

	// Try to merge with previous if has
	if (index > 0)
	{
		gpalloc_allocation* previous = allocator->allocation_array + (index - 1);

		if (!previous->used)
		{
			previous->size += current->size;
			gpalloc_erase_at(allocator, index--);
			current = allocator->allocation_array + (index);
		}
	}
	// try to merge with next
	if (index + 1 < allocator->allocation_array_size)
	{
		gpalloc_allocation* next = allocator->allocation_array + (index + 1);

		if (!next->used)
		{
			current->size += next->size;
			gpalloc_erase_at(allocator, index + 1);
		}
	}
}



void* gpalloc_malloc_first_fit_block(gpalloc_t* allocator, const size_t bytes, const size_t alignment)
{
	size_t i;
	for (i = 0; i < allocator->allocation_array_size; i++)
	{
		gpalloc_allocation* block = allocator->allocation_array + i;
		if (block->used)
			continue;

		void* aligned_ptr = gpalloc_align(block->address, alignment);
		const uintptr_t unaligned_block_end = (uintptr_t)gpalloc_offset_ptr(block->address, block->size);
		const uintptr_t aligned_block_end = (uintptr_t)gpalloc_offset_ptr(aligned_ptr, bytes);

		// If aligned block overflows the current block skip
		if (aligned_block_end > unaligned_block_end)
			continue;

		const uintptr_t alignment_offset = gpalloc_ptr_diff(block->address, aligned_ptr);

		// If already aligned then do this:
		// Split block in two:
		// First part is used
		// Second part is free
		if (alignment_offset == 0)
		{
			// Second free block
			gpalloc_allocation free_block = { .address = (void*)aligned_block_end, .size = block->size - bytes, .used = false };

			// First used block
			{
				block->size = bytes;
				block->used = true;
			}

			if (free_block.size > 0)
				gpalloc_insert(allocator, i + 1, free_block);

			return aligned_ptr;
		}

		// Allocation is not at alignment requirement
		// Must split into three blocks: | free | used | free |
		const size_t original_block_size = block->size;
		const size_t third_block_size = original_block_size - bytes - alignment_offset;
		gpalloc_allocation third_block = { .address = (void*)aligned_block_end, .size = third_block_size, .used = false };

		gpalloc_allocation second_block = { .address = aligned_ptr, .size = bytes, .used = true };
		assert(second_block.size > 0);

		// first block
		block->size = original_block_size - (third_block_size + second_block.size);
		assert(block->size > 0);
		assert(block->size + second_block.size + third_block_size == original_block_size);

		if (third_block_size > 0)
		{
			gpalloc_insert(allocator, i + 1, third_block);
		}
		gpalloc_insert(allocator, i + 1, second_block);

		return aligned_ptr;
	}

	return (void*)NULL;
}

#pragma endregion


void gpalloc_initialize(gpalloc_t* allocator, void* buffer, const size_t pool_size) {
	assert(allocator != NULL);
	assert(buffer != NULL);
	assert(pool_size > 0 && "Memory size must be greater than 0");

	{
		// Initialize
		gpalloc_t gpa = { .buffer = buffer, .buffer_size = pool_size, .allocation_array_size = 0, .allocation_array_capacity = 0 };
		pun_cpy(allocator, gpalloc_t, &gpa);
	}

	// Increase to 10 of slack so we always have some spare space
	const size_t initial_capacity = 10;
	gpalloc_grow_array(allocator, initial_capacity);

	// Mark free block of whole size
	gpalloc_allocation allocation = { .address = buffer, .size = pool_size };
	gpalloc_emplace(allocator, allocation);
}

void gpalloc_destroy(gpalloc_t* allocator)
{
	assert(allocator != NULL);
	free(allocator->allocation_array);
	memset((void*)allocator, 0, sizeof(gpalloc_t));
}

void* gpalloc_malloc(gpalloc_t* allocator, size_t bytes, const size_t alignment) {

	const size_t worstAlignmentSize = bytes + gpalloc_max(alignment, __alignof(gpalloc_allocation)) + sizeof(gpalloc_allocation);

	return gpalloc_malloc_first_fit_block(allocator, bytes, alignment);
}

void gpalloc_free(gpalloc_t* allocator, void* ptr) {
	assert(allocator != NULL);
	assert(ptr != NULL);

	const size_t index = gpalloc_lower_bound(allocator, ptr);
	gpalloc_allocation* const allocation = allocator->allocation_array + index;
	if (allocation->address == ptr)
	{
		assert(allocation->used == true && "Must not be already free!");
		allocation->used = false;
		gpalloc_coalescence(allocator, index);
	}
}






