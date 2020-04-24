//#include <stdlib.h>
#include <stdio.h>
#include "arrays_c.h"



array_t *  array_construct(size_t size){
	array_t * array = malloc(sizeof(array_t));
	array->num_elems = size;
	array->num_data_pages = (size / NUM_ELEMS) + 1;
	array->num_l1_pages = (array->num_data_pages / PTRS_PER_PAGE) + 1;
	printf("Size of element is %lu\n", sizeof(int));
  	printf("Num pages allocated is %lu\n", array->num_data_pages);
  	printf("Number of l1 pages is %lu\n", array->num_l1_pages);
	
	if (array->num_data_pages > MAX_PAGES){
		printf("Can't support that many pages!!\n");
		exit(1);
	}
	array->num_elems = size;
	if(array->num_data_pages > 1){	 //What if num_data_pages = 1 (single)?
		if(array->num_l1_pages > 1){
			for(size_t i = 0; i < array->num_l1_pages; i++){
				void * l1_page = malloc(PAGE_SIZE);
				array->ptable[i] = l1_page; //First page contains pointers to other pages
				size_t l2pages = (i == array->num_l1_pages - 1) ? array->num_data_pages % PTRS_PER_PAGE : PTRS_PER_PAGE;
				for (size_t j = 0; j < l2pages; j++){
					void ** l1_page_cast = (void **)l1_page;
					l1_page_cast[j] = malloc(PAGE_SIZE);
				}
			}
		} 
		else { //If one level
			for (size_t i = 0; i < array->num_data_pages; i++){
				void * page = malloc(PAGE_SIZE);
				array->ptable[i] = page;
			}
		}	
	}
	return array;
}

void array_destruct(array_t * this) {
	if (!(this->num_data_pages == 1)) {
		if (this->num_l1_pages > 1) {
			for (size_t i = 0; i < this->num_l1_pages; i++) {
				void **l1_page = (void**) this->ptable[i];
				size_t l2pages = (i == this->num_l1_pages - 1) ? this->num_data_pages % PTRS_PER_PAGE : PTRS_PER_PAGE;
				for (size_t j = 0; j < l2pages; j++) {
					free(l1_page[j]);
				}
			free(l1_page); //What about ptable?
      		}
		}
		else {
			for (size_t i = 0; i < this->num_data_pages; i++) {
			free(this->ptable[i]);
      		}
		}
	}
}

array_t * resize(size_t old_size, array_t * orig, size_t new_size) {
	array_t * result = array_construct(new_size);
	size_t copysize = (new_size > old_size) ? old_size : new_size;
	for (size_t i = 0; i < copysize; i++) {
		int* result_p = at_ptr(result, i);
		*result_p = at(orig, i); //Change to use .at()
		//Do we need another function to assign value in array?
		//at, but return pointer (&)
	 }
	return result;
}

int at(array_t * this, size_t index) {
	if (this->num_data_pages == 1) {
		int* entries = (int*) this->ptable;
		return entries[index];
	} else if (this->num_l1_pages > 1) {
		int*** entries = (int***) this->ptable;
		size_t l1off = getL1Offset(index);
		int** l1_page = entries[l1off];
		size_t pageno = getL2Index(index);
		size_t offset = getL2Offset(index);
		int* page = l1_page[pageno];
		return page[offset];
	} else { //If one level
		int** entries = (int**) this->ptable;
		size_t pageno = index / NUM_ELEMS;
		size_t offset = index % NUM_ELEMS;
		int* page = entries[pageno];
		return page[offset];
	}
}

int* at_ptr(array_t * this, size_t index) {
	if (this->num_data_pages == 1) {
		int* entries = (int*) this->ptable;
		return &entries[index];
	} else if (this->num_l1_pages > 1) {
		int*** entries = (int***) this->ptable;
		size_t l1off = getL1Offset(index);
		int** l1_page = entries[l1off];
		size_t pageno = getL2Index(index);
		size_t offset = getL2Offset(index);
		int* page = l1_page[pageno];
		return &page[offset];
	} else { //If one level
		int** entries = (int**) this->ptable;
		size_t pageno = index / NUM_ELEMS;
		size_t offset = index % NUM_ELEMS;
		int* page = entries[pageno];
		return &page[offset];
	}
}

//See which can run with c++, c


MemRegion getRegion(array_t * this, size_t index){
	MemRegion result;
	if (this->num_data_pages == 1) {
		int* entries = (int*) this->ptable;
		size_t end = NUM_ELEMS > this->num_elems ? this->num_elems - 1 : NUM_ELEMS - 1;
		result.minValue = &(entries[index]);
		result.maxValue = &(entries[end]);
	} 
	else if (this->num_l1_pages > 1) {
		int*** entries = (int***) this->ptable;
		size_t l1off = getL1Offset(index);
		int** l1_page = entries[l1off];
		size_t pageno = getL2Index(index);
		size_t offset = getL2Offset(index);
		int* page = l1_page[pageno];
		size_t end = NUM_ELEMS + (index - offset) > this->num_elems ? this->num_elems - index + offset - 1 : NUM_ELEMS - 1;
		result.minValue = &(page[offset]);
		result.maxValue = &(page[end]);
	}
	else {
		int** entries = (int**) this->ptable;
		size_t pageno = index / NUM_ELEMS;
		size_t offset = index % NUM_ELEMS;
		int* page = entries[pageno];
		size_t end = NUM_ELEMS + (index - offset) > this->num_elems ? this->num_elems - index + offset - 1 : NUM_ELEMS - 1;
		result.minValue = &(page[offset]);
		result.maxValue = &(page[end]);
	}
	return result;
}

void arrayCopy(array_t * destptr, size_t deststart, array_t * srcptr, size_t srcstart, size_t count){
	size_t copied = 0;
	while(copied < count) {
		MemRegion src = getRegion(srcptr, copied + srcstart);
		MemRegion dest = getRegion(destptr, copied + deststart);
		while (copied < count && src.minValue <= src.maxValue && dest.minValue <= dest.maxValue) {
	  	*(dest.minValue) = *(src.minValue);
	  	src.minValue++;
	  	dest.minValue++;
	  	copied++;
		}
	}
}

