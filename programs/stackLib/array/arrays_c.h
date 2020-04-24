#ifndef ARRAYS_C_H
#define ARRAYS_C_H
#include <stdlib.h>
#include <stdio.h>

#define PAGE_SIZE (1024*32)
#define PTRS_PER_PAGE (PAGE_SIZE / sizeof(void*))
#define MAX_PAGES (PTRS_PER_PAGE * PTRS_PER_PAGE)
#define NUM_ELEMS (PAGE_SIZE / sizeof(T))
#define ELEMS_PER_L2 (PTRS_PER_PAGE * NUM_ELEMS)
#define single (num_data_pages == 1)
#define two_level (num_l1_pages > 1)

#define getL1Offset(i) (i / ELEMS_PER_L2)
#define getL2Index(i)  ((i / NUM_ELEMS) % PTRS_PER_PAGE)
#define getL2Offset(i) (i % NUM_ELEMS)

typedef struct{
	void* ptable[PTRS_PER_PAGE];
	size_t num_elems;
	size_t num_l1_pages;
	size_t num_data_pages;
} array_t;

void array_construct(size_t size){
	num_elems = size;
	num_data_pages = (size / NUM_ELEMS) + 1;
	num_l1_pages = (num_data_pages / PTRS_PER_PAGE) + 1;
	// printf("Size of element is %lu\n", sizeof(int));
  	// printf("Num pages allocated is %lu\n", num_data_pages);
  	// printf("Number of l1 pages is %lu\n", num_l1_pages);
	
	if (num_data_pages > MAX_PAGES){
		printf("Can't support that many pages!!\n");
		exit(1);
	}
	num_elems = size;
	if(num_data_pages > 1){	 //What if num_data_pages = 1?
		if(two_level){
			for(size_t i = 0; i < num_l1_pages; i++){
				void * l1_page = malloc(PAGE_SIZE);
				ptable[i] = l1_page; //First page contains pointers to other pages
				size_t l2pages = (i == num_l1_pages - 1) ? num_data_pages % PTRS_PER_PAGE : PTRS_PER_PAGE;
				for (size_t j = 0; j < l2pages; j++){
					void ** l1_page_cast = (void **)l1_page;
					l1_page_cast[j] = malloc(PAGE_SIZE);
				}
			}
		} 
		else {
			for (size_t i = 0; i < num_data_pages; i++){
				void * page = malloc(PAGE_SIZE);
				ptable[i] = page;
			}
		}	
	}
}

void array_destruct() {
	if (!single) {
		if (two_level) {
			for (size_t i = 0; i < num_l1_pages; i++) {
				void **l1_page = (void**) ptable[i];
				size_t l2pages = (i == num_l1_pages - 1) ? num_data_pages % PTRS_PER_PAGE : PTRS_PER_PAGE;
				for (size_t j = 0; j < l2pages; j++) {
					free(l1_page[j]);
				}
			free(l1_page);
      		}
		}
		else {
			for (size_t i = 0; i < num_data_pages; i++) {
			free(ptable[i]);
      		}
		}
	}
}












#endif
