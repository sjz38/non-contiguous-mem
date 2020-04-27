#include <stdlib.h>
#ifndef ARRAYS_C_H
#define ARRAYS_C_H

#define PAGE_SIZE (1024*32)
#define PTRS_PER_PAGE (PAGE_SIZE / sizeof(void*))
#define MAX_PAGES (PTRS_PER_PAGE * PTRS_PER_PAGE)
#define NUM_ELEMS (PAGE_SIZE / sizeof(int))
#define ELEMS_PER_L2 (PTRS_PER_PAGE * NUM_ELEMS)
#define single_level(array) (array->num_data_pages == 1)
#define two_level(array) (array->num_l1_pages > 1)

#define getL1Offset(i) (i / ELEMS_PER_L2)
#define getL2Index(i)  ((i / NUM_ELEMS) % PTRS_PER_PAGE)
#define getL2Offset(i) (i % NUM_ELEMS)
//getL2Offset(int);
typedef struct{
	void* ptable[PTRS_PER_PAGE];
	size_t num_elems;
	size_t num_l1_pages;
	size_t num_data_pages;
} array_t;

struct MemRegion{ //generic, void poitners
	int * minValue;
	int * maxValue;
};

array_t * array_construct   (size_t size);
void array_destruct	   (array_t * this);
array_t * resize	   (size_t old_size, array_t * orig, size_t new_size);
int at				   (array_t * this, size_t idx);
int* at_ptr			   (array_t * this, size_t index);

void arrayCopy 		   (array_t* destptr, size_t deststart, array_t* srcptr, size_t srcstart, size_t count);
struct MemRegion getRegion	   (array_t * this, size_t index);
void copyInto		   (array_t * destptr, size_t startIdx, void * srcptr, size_t count);
void copyOutOf		   (void * destptr, array_t * srcptr, size_t startIdx, size_t count);
#endif
