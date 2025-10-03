#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stddef.h>


#include "clow/slice.c"


static void slice_tests(void)
{

	{
		// Allocate and deallocate whole range
		slice_allocator s;
		memset(&s, 0, sizeof(s));
		slice_initialize(&s, 10);

		slice_t a = slice_alloc(&s, 10);
		assert(a.offset == 0);
		assert(a.count == 10);
		slice_free(&s, a);

		slice_destroy(&s);
	}

	// Allocate one by one
	{
		slice_allocator s;
		memset(&s, 0, sizeof(s));
		slice_initialize(&s, 10);
		slice_t slices[10];
		for (size_t i = 0; i < 10; i++)
		{
			slices[i] = slice_alloc(&s, 1);
			assert(slices[i].offset == i);
			assert(slices[i].count == 1);
		}
		for (size_t i = 0; i < 10; i++)
		{
			slice_free(&s, slices[i]);
		}
		slice_destroy(&s);
	}

	// Allocate in chunks and free in different order
	{
		slice_allocator s;
		memset(&s, 0, sizeof(s));
		slice_initialize(&s, 10);
		slice_t a = slice_alloc(&s, 3);
		assert(a.offset == 0 && a.count == 3);
		slice_t b = slice_alloc(&s, 4);
		assert(b.offset == 3 && b.count == 4);
		slice_t c = slice_alloc(&s, 3);
		assert(c.offset == 7 && c.count == 3);
		slice_free(&s, b);
		slice_free(&s, a);
		slice_free(&s, c);

		assert(s.free_slices_array_size == 1);
		assert(s.free_slices[0].offset == 0 && s.free_slices[0].count == 10);

		slice_destroy(&s);
	}

	// Free slices must coalesce
	{
		slice_allocator s;
		memset(&s, 0, sizeof(s));
		slice_initialize(&s, 10);
		slice_t a = slice_alloc(&s, 3);
		assert(a.offset == 0 && a.count == 3);
		slice_t b = slice_alloc(&s, 4);
		assert(b.offset == 3 && b.count == 4);
		slice_t c = slice_alloc(&s, 3);
		assert(c.offset == 7 && c.count == 3);
		slice_free(&s, a);
		slice_free(&s, c);
		slice_free(&s, b);
		assert(s.free_slices_array_size == 1);
		assert(s.free_slices[0].offset == 0 && s.free_slices[0].count == 10);
		slice_destroy(&s);
	}

};

int main(void)
{
	slice_tests();
	return 0;
}
