// //////////////////////////////////////////////////////////////////////////////////////////
// FILE: slice.c
//
// AUTHOR: Kirichenko Stanislav
//
// DATE: 3 OCT 2025
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

#include "clow/slice.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void
slice_initialize(slice_allocator* allocator, const size_t maxNumOfElements)
{
    // Must be zero initialized
    assert(allocator != NULL);
    assert(maxNumOfElements > 0);
    assert(allocator->free_slices == NULL);
    assert(allocator->free_slices_array_size == 0);

    allocator->max_elements = maxNumOfElements;
    allocator->free_slices  = (slice_t*)malloc(sizeof(slice_t));
    assert(allocator->free_slices != NULL);
    if (allocator->free_slices != NULL)
        ;
    {
        allocator->free_slices_array_size     = 1;
        allocator->free_slices_array_capacity = 1;

        allocator->free_slices[0].offset = 0;
        allocator->free_slices[0].count  = maxNumOfElements;
    }
}

void
slice_destroy(slice_allocator* allocator)
{
    assert(allocator != NULL);
    free(allocator->free_slices);
    allocator->free_slices            = NULL;
    allocator->free_slices_array_size = 0;
    allocator->max_elements           = 0;
}

slice_t
slice_alloc(slice_allocator* allocator, const size_t count)
{
    assert(allocator != NULL);
    assert(count > 0);
    if (allocator->free_slices == NULL)
        {
            slice_t invalid = { .offset = 0, .count = 0 };
            return invalid;
        }

    // Find the first slice that fits
    for (size_t i = 0; i < allocator->free_slices_array_size; ++i)
        {
            slice_t* current_slice = &allocator->free_slices[i];
            if (current_slice->count == count)
                {
                    // Exact match, remove the slice from the free list
                    slice_t allocated_slice = *current_slice;
                    // Shift remaining slices down
                    memmove(&allocator->free_slices[i], &allocator->free_slices[i + 1], (allocator->free_slices_array_size - i - 1) * sizeof(slice_t));
                    allocator->free_slices_array_size--;
                    return allocated_slice;
                }
            else if (current_slice->count > count)
                {
                    // Allocate from the beginning of the slice
                    slice_t allocated_slice = { .offset = current_slice->offset, .count = count };
                    current_slice->offset += count;
                    current_slice->count -= count;
                    return allocated_slice;
                }
        }

    slice_t invalid = { .offset = 0, .count = 0 };
    return invalid;
}

void
slice_free(slice_allocator* allocator, const slice_t slice)
{
    assert(allocator != NULL);
    assert(slice.count > 0);

    size_t insert_index = 0;
    while (insert_index < allocator->free_slices_array_size && allocator->free_slices[insert_index].offset < slice.offset)
        {
            insert_index++;
        }

    // Try merge with previous
    if (insert_index > 0 && allocator->free_slices[insert_index - 1].offset + allocator->free_slices[insert_index - 1].count == slice.offset)
        {
            allocator->free_slices[insert_index - 1].count += slice.count;

            // Also try merge with next
            if (insert_index < allocator->free_slices_array_size && slice.offset + slice.count == allocator->free_slices[insert_index].offset)
                {
                    allocator->free_slices[insert_index - 1].count += allocator->free_slices[insert_index].count;

                    memmove(&allocator->free_slices[insert_index], &allocator->free_slices[insert_index + 1], (allocator->free_slices_array_size - insert_index - 1) * sizeof(slice_t));

                    allocator->free_slices_array_size--;
                }
            return;
        }

    // Try merge with next only
    if (insert_index < allocator->free_slices_array_size && slice.offset + slice.count == allocator->free_slices[insert_index].offset)
        {
            allocator->free_slices[insert_index].offset = slice.offset;
            allocator->free_slices[insert_index].count += slice.count;
            return;
        }

    // Otherwise insert new slice
    if (allocator->free_slices_array_size == allocator->free_slices_array_capacity)
        {
            size_t   new_capacity = allocator->free_slices_array_capacity ? allocator->free_slices_array_capacity * 2 : 8;
            slice_t* new_array    = (slice_t*)realloc(allocator->free_slices, new_capacity * sizeof(slice_t));
            if (!new_array)
                return; // OOM
            allocator->free_slices                = new_array;
            allocator->free_slices_array_capacity = new_capacity;
        }

    memmove(&allocator->free_slices[insert_index + 1], &allocator->free_slices[insert_index], (allocator->free_slices_array_size - insert_index) * sizeof(slice_t));

    allocator->free_slices[insert_index] = slice;
    allocator->free_slices_array_size++;
}

size_t
slice_compute_unused_count(const slice_allocator* allocator)
{
    assert(allocator != NULL);
    size_t total = 0;
    for (size_t i = 0; i < allocator->free_slices_array_size; ++i)
        {
            total += allocator->free_slices[i].count;
        }
    return total;
}
