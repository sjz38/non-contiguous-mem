#ifndef ARRAYS_C_H
#define ARRAYS_C_H
#include <stdlib.h>
#include <stdio.h>



typedef struct{
	void* ptable[PTRS_PER_PAGE];
	size_t num_elems;
	size_t num_l1_pages;
	size_t num_data_pages;
} array_t;

typedef struct{
	int * minValue;
	int * maxValue;
} MemRegion;

array_t * array_construct   (size_t size);
void array_destruct	   (array_t * this);
array_t * resize	   (size_t old_size, array_t * orig, size_t new_size);
int at				   (array_t * this, size_t idx);
int* at_ptr			   (array_t * this, size_t index);

array_t * arrayCopy    (array_t * destptr, size_t deststart, array_t * srcptr, size_t srcstart, size_t count)
void copyInto		   (array_t * destptr, size_t startIdx, void * srcptr, size_t count);
void copyOutOf		   (void * destptr, array_t * srcptr, size_t startIdx, size_t count);
MemRegion getRegion	   (array_t * this, size_t index);




#endif
