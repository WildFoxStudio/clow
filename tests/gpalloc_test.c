#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stddef.h>


#include "clow/gpalloc.c"

static void* offset_ptr(void* begin, size_t offset)
{
	return (void*)(((uintptr_t)begin) + offset);
}

#define BUF_INIT_VALUE ((size_t)'A')
#define BUF_ALLOC_VALUE ((size_t)'W')

static void gpalloc_tests(void)
{
	// Allocate 1 element
	{
		_Alignas(2) char buffer[256];
		memset(buffer, BUF_INIT_VALUE, 256);

		gpalloc_t gpa;

		gpalloc_initialize(&gpa, buffer, 256);

		void* allocation = gpalloc_malloc(&gpa, 16, 8);

		assert(allocation && "Must return valid ptr!");
		assert(((uintptr_t)allocation) % 8 == 0 && "Must be aligned!");

		gpalloc_free(&gpa, allocation);
	}

	// Allocate 10 blocks with increasing alignment
	{
		_Alignas(2) char buffer[4096];
		memset(buffer, BUF_INIT_VALUE, 4096);

		gpalloc_t gpa;

		gpalloc_initialize(&gpa, buffer, 4096);
		void* allocations[10] = { 0,0,0,0,0,0,0,0,0,0 };

		size_t i;
		for (i = 0; i < 10; i++)
		{
			const size_t alignment = (size_t)pow(2., 10. - i);
			void* ptr = gpalloc_malloc(&gpa, 8, alignment);
			size_t j;
			for (j = 0; j < 10; j++)
			{
				assert(allocations[j] != ptr && "Must not return the same pointer twice!");
			}
			allocations[i] = ptr;
			assert(allocations[i] && "Must return valid ptr!");
			assert(((uintptr_t)allocations[i]) % alignment == 0 && "Must be aligned!");
		}


		for (i = 0; i < 10; i++) {
			gpalloc_free(&gpa, allocations[i]);
		}

	}

	// Allocate 10 blocks with increasing alignment
	{
		_Alignas(__alignof(double)) char buffer[4096];
		memset(buffer, BUF_INIT_VALUE, 4096);

		gpalloc_t gpa;

		gpalloc_initialize(&gpa, buffer, 4096);
		void* allocations[10] = { 0,0,0,0,0,0,0,0,0,0 };

		size_t i;
		for (i = 0; i < 10; i++)
		{
			const size_t alignment = (size_t)pow(2., 10. - i);
			allocations[i] = gpalloc_malloc(&gpa, 8, alignment);
			assert(allocations[i] && "Must return valid ptr!");
			assert(((uintptr_t)allocations[i]) % alignment == 0 && "Must be aligned!");
		}


		for (i = 0; i < 10; i++) {
			gpalloc_free(&gpa, allocations[i]);
		}

	}
}

int main(void)
{
	gpalloc_tests();
	return 0;
}
