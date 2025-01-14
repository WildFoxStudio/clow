#include <stdio.h>
#include <assert.h>
#include <string.h>


#include "clow/freelist.c"

static void* offset_ptr(void* begin, size_t offset)
{
	return (void*)(((uintptr_t)begin) + offset);
}

#define BUF_INIT_VALUE ((size_t)'A')
#define BUF_ALLOC_VALUE ((size_t)'W')

static void init(freelist_t* f, char* buffer, size_t size)
{
	memset((void*)buffer, BUF_INIT_VALUE, size);

	freelist_initialize(f, buffer, size);
}

static void* alloc(freelist_t* f, size_t size)
{
	void* a;
	a = freelist_malloc(f, size);
	if (a)
	{
		assert(freelist_range_check(f, a) == 1);
		memset(a, BUF_ALLOC_VALUE, size);
	}

	assert(freelist_verify_corruption(f) == 1);

	return a;
}

static void deinit(freelist_t* f)
{
	freelist_verify_corruption(f);
	freelist_reset(f);
}

static void freelist_tests(void)
{

	{
		assert(freelist_alloc_overhead() == sizeof(size_t));
	}

	{
		assert(freelist_min_alloc_block() == sizeof(freelist_block));
	}

	{
		char buffer[16];
		freelist_t f;
		init(&f, buffer, sizeof(buffer));
		assert(freelist_get_buffer(&f) == (void*)buffer);
		deinit(&f);
	}

	// Allocate 1 element
	{
		char buffer[16 + 8];
		freelist_t f;
		void* a;

		init(&f, buffer, sizeof(buffer));

		a = alloc(&f, 16);
		assert(a);
		assert(a == offset_ptr(buffer, freelist_alloc_overhead()));
		freelist_free(&f, a);

		deinit(&f);
	}

	// freelist_get_allocation_size must return correct size for allocation
	{
		char buffer[16 + 8];
		freelist_t f;
		void* a;
		size_t a_size;

		init(&f, buffer, sizeof(buffer));

		a = alloc(&f, 16);
		a_size = freelist_get_allocation_size(&f, a);
		assert(a_size == 16);

		freelist_free(&f, a);

		deinit(&f);
	}

	// Allocate 10 blocks of 16 bytes
	{
		char buffer[(16 + 8) * 10];
		freelist_t f;
		size_t i;
		void* a;
		size_t expected_offset;

		init(&f, buffer, sizeof(buffer));

		freelist_initialize(&f, buffer, (16 + 8) * 10);

		for (i = 0; i < 10; i = i + 1)
		{
			a = alloc(&f, 16);
			assert(a);
			expected_offset = i * (16 + freelist_alloc_overhead()) + freelist_alloc_overhead();
			assert(a == offset_ptr(buffer, expected_offset));

		}

		deinit(&f);
	}



	// Allocate multiple blocks
	{
		char buffer[(16 + 8) * 10];
		freelist_t f;
		size_t i;
		void* a;
		void* b;
		size_t expected_offset;

		init(&f, buffer, sizeof(buffer));

		for (i = 0; i < 10; i += 1)
		{
			a = alloc(&f, 16);
			assert(a);
			expected_offset = i * (16 + freelist_alloc_overhead()) + freelist_alloc_overhead();
			assert(a == offset_ptr(buffer, expected_offset));
			// Allocate then free b to scramble free blocks.
			b = alloc(&f, 16);
			freelist_free(&f, b);
		}
		// No space left, must not succeed
		b = alloc(&f, 16);
		assert(!b);
		((void)b);


		deinit(&f);
	}

	// Allocate second blocks to be outside the memory boundaries
	if (0/*This test throws also a memory corruption violation*/)
	{
		char buffer[31];
		freelist_t f;
		void* a;
		void* b;

		init(&f, buffer, sizeof(buffer));

		a = alloc(&f, 16);
		assert(a);
		assert(a == offset_ptr(buffer, freelist_alloc_overhead()));


		b = alloc(&f, 16);
		assert(!b);
		((void)b);


		deinit(&f);
	}

	// Catch metadata corruption when writing out of the allocation bounds
	if (0/*This test throws also a memory corruption violation*/)
	{
		char buffer[48];
		freelist_t f;
		void* a;

		init(&f, buffer, sizeof(buffer));

		a = freelist_malloc(&f, 16);
		assert(a);
		assert(a == offset_ptr(buffer, freelist_alloc_overhead()));
		// Write more than 16 bytes allocated to corrupt internal pool data
		memset(a, BUF_ALLOC_VALUE, 16 + 8);

		// Verify that the corruption has been catched
		assert(freelist_verify_corruption(&f) == 0);
	}

}

int main(void)
{
	freelist_tests();
	return 0;
}
