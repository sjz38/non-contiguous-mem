#include <stdlib.h>
#ifndef ARRAYS_C_MACRO_H
#define ARRAYS_C__MACRO_H
#include <stdio.h>

#define PAGE_SIZE (1024*32)
#define PTRS_PER_PAGE (PAGE_SIZE / sizeof(void*))
#define MAX_PAGES (PTRS_PER_PAGE * PTRS_PER_PAGE)
//#define NUM_ELEMS (PAGE_SIZE / sizeof(int))
//#define ELEMS_PER_L2 (PTRS_PER_PAGE * NUM_ELEMS)
//#define single_level(array) (array->num_data_pages == 1)
//#define two_level(array) (array->num_l1_pages > 1)

//#define getL1Offset(i) (i / ELEMS_PER_L2)
//#define getL2Index(i)  ((i / NUM_ELEMS) % PTRS_PER_PAGE)
//#define getL2Offset(i) (i % NUM_ELEMS)

typedef struct array_t{	
		void* ptable[PTRS_PER_PAGE];		
		size_t num_elems;								
		size_t num_l1_pages;						
		size_t num_data_pages;					
}array_t;
	
//How to return value from function?
//How to handle if statement syntax
//How to malloc

typedef struct{ //generic, void poitners
	void * minValue;
	void * maxValue;
}MemRegion;


#define ARRAY_CONS_MAKER(type)	\
	array_t * array_construct_##type(size_t size){	\
		size_t NUM_ELEMS = PAGE_SIZE / sizeof(type);	\
		size_t ELEMS_PER_L2 = PTRS_PER_PAGE * NUM_ELEMS;	\
		array_t * array = malloc(sizeof(array_t));	\
		array->num_elems = size;	\
		array->num_data_pages = (size / NUM_ELEMS) + 1;	\
		array->num_l1_pages = (array->num_data_pages / PTRS_PER_PAGE) + 1;	\
		/*printf("Size of element is %lu\n", sizeof(int));*/\
		/*printf("Num pages allocated is %lu\n", array->num_data_pages);*/	\
		/*printf("Number of l1 pages is %lu\n", array->num_l1_pages);*/	\
		if (array->num_data_pages > MAX_PAGES){	\
			printf("Can't support that many pages!!\n");	\
			exit(1);	\
		}	\
		array->num_elems = size;	\
		if(!(array->num_data_pages == 1)){	 /*What if num_data_pages = 1 (single)?*/	\
			if(array->num_l1_pages > 1){	\
				for(size_t i = 0; i < array->num_l1_pages; i++){	\
					void * l1_page = malloc(PAGE_SIZE);	\
					array->ptable[i] = l1_page; /*First page contains pointers to other pages*/\
					size_t l2pages = (i == array->num_l1_pages - 1) ? array->num_data_pages % PTRS_PER_PAGE : PTRS_PER_PAGE;	\
					for (size_t j = 0; j < l2pages; j++){	\
						void ** l1_page_cast = (void **)l1_page;	\
						l1_page_cast[j] = malloc(PAGE_SIZE);	\
					}	\
				}	\
			} 	\
			else { /*If one level*/	\
				for (size_t i = 0; i < array->num_data_pages; i++){	\
					void * page = malloc(PAGE_SIZE);	\
					array->ptable[i] = page;	\
				}	\
			}		\
		}	\
		return array;	\
	}
																																		\
ARRAY_CONS_MAKER(int);
ARRAY_CONS_MAKER(double);																														

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
	free(this);	
}

#define AT_MAKER(type)\
	type* at_##type(array_t * array, size_t index) {\
		size_t NUM_ELEMS = PAGE_SIZE / sizeof(type);	\
		size_t ELEMS_PER_L2 = PTRS_PER_PAGE * NUM_ELEMS;	\
		if (array->num_data_pages == 1) {\
			type* entries = (type*) array->ptable;\
			return &(entries[index]);\
		} else if (array->num_l1_pages > 1) {\
			type*** entries = (type***) array->ptable;\
			size_t l1off = index / ELEMS_PER_L2;\
			type** l1_page = entries[l1off];\
			size_t pageno = (index / NUM_ELEMS) % PTRS_PER_PAGE;\
			size_t offset = index % NUM_ELEMS;\
			type* page = l1_page[pageno];\
			return &(page[offset]);\
		} else { /*If one level*/\
			type** entries = (type**) array->ptable;\
			size_t pageno = index / NUM_ELEMS;\
			size_t offset = index % NUM_ELEMS;\
			type* page = entries[pageno];\
			return &(page[offset]);\
		}\
	}
	
AT_MAKER(int);
AT_MAKER(double);

#define RESIZE_MAKER(type)\
	array_t * resize_##type(size_t old_size, array_t * orig, size_t new_size) {\
		array_t * result = array_construct_##type(new_size);\
		size_t copysize = (new_size > old_size) ? old_size : new_size;\
		for (size_t i = 0; i < copysize; i++) {\
			type* result_p = at_##type(result, i);\
			*result_p = *(at_##type(orig, i));\
		}\
		array_destruct(orig);\
		return result;\
	}

RESIZE_MAKER(int);
RESIZE_MAKER(double);

#define GETREGION_MAKER(type)	\
	MemRegion getRegion_##type (array_t * array, size_t index) {\
		size_t NUM_ELEMS = PAGE_SIZE / sizeof(type);\
		size_t ELEMS_PER_L2 = PTRS_PER_PAGE * NUM_ELEMS;\
		MemRegion result;\
		if (array->num_data_pages == 1) {\
			type* entries = (type*) array->ptable;\
			size_t end = NUM_ELEMS > array->num_elems ? array->num_elems - 1 : NUM_ELEMS - 1;\
			result.minValue = &(entries[index]);\
			result.maxValue = &(entries[end]); \
		} \
		else if (array->num_l1_pages > 1) {\
			type*** entries = (type***) array->ptable;\
			size_t l1off = index / ELEMS_PER_L2;\
			type** l1_page = entries[l1off];\
			size_t pageno = (index / NUM_ELEMS) % PTRS_PER_PAGE;\
			size_t offset = index % NUM_ELEMS;\
			type* page = l1_page[pageno];\
			size_t end = NUM_ELEMS + (index - offset) > array->num_elems ? array->num_elems - index + offset - 1 : NUM_ELEMS - 1;\
			result.minValue = &(page[offset]);\
			result.maxValue = &(page[end]);\
		}\
		else {\
			type** entries = (type**) array->ptable;\
			size_t pageno = index / NUM_ELEMS;\
			size_t offset = index % NUM_ELEMS;\
			type* page = entries[pageno];\
			size_t end = NUM_ELEMS + (index - offset) > array->num_elems ? array->num_elems - index + offset - 1 : NUM_ELEMS - 1;\
			result.minValue = &(page[offset]);\
			result.maxValue = &(page[end]);\
		}\
		return result;\
	}
	
GETREGION_MAKER(int);
GETREGION_MAKER(double);
#endif // ARRAYS_C_MACRO_H
