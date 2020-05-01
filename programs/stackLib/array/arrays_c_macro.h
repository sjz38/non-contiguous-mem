#include <stdlib.h>
#ifndef ARRAYS_C_MACRO_H
#define ARRAYS_C__MACRO_H
#include <stdio.h>

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

typedef struct{	
		void* ptable[PTRS_PER_PAGE];		
		size_t num_elems;								
		size_t num_l1_pages;						
		size_t num_data_pages;					
}array_t
	
//How to return value from function?
//How to handle if statement syntax
//How to malloc
/*
#define MEMREGION_T(type)
	struct{ 					\
		type * minValue;				\
		type * maxValue;				\
	}
*/
#define MALLOC(p,s) (p=malloc(s))

#define ARRAY_CONSTRUCT(array, size)	({\
	/*array_t* array = malloc(sizeof(array_t));	\ */
	MALLOC(array, sizeof(array_t));															\
	(array)->num_elems = size;																							\
	(array)->num_data_pages = (size / NUM_ELEMS) + 1;												\
	(array)->num_l1_pages = ((array)->num_data_pages / PTRS_PER_PAGE) + 1;	\
	if ((array)->num_data_pages > MAX_PAGES){																\
		printf("Can't support that many pages!!\n");													\
		exit(1);																															\
	}																																				\
	(array)->num_elems = size;																								\
	if(!single_level(array)){	 																												\
		if(two_level(array)){																											\
			for(size_t i = 0; i < (array)->num_l1_pages; i++){																	\
				void * l1_page = malloc(PAGE_SIZE);																	\
				(array)->ptable[i] = l1_page; 																									\
				size_t l2pages = (i == (array)->num_l1_pages - 1) ? (array)->num_data_pages % PTRS_PER_PAGE : PTRS_PER_PAGE;	\
				for (size_t j = 0; j < l2pages; j++){																			\
					void ** l1_page_cast = (void **)l1_page;															\
					l1_page_cast[j] = malloc(PAGE_SIZE);																							\
				}																																			\
			}																																		\
		} 																																				\
		else { 																																			\
			for (size_t i = 0; i < (array)->num_data_pages; i++){											\
				void * page = malloc(PAGE_SIZE);																			\
				(array)->ptable[i] = page;																								\
			}																																					\
		}																																			\
	}																																											\
	})																																	

/*
#define ARRAY_DESTRUCT(array)	\
	if (!single_level(array)) {	\
		if (two_level(array)) {	\
			for (size_t i = 0; i < array->num_l1_pages; i++) {	\
				void **l1_page = (void**) array->ptable[i];	\
				size_t l2pages = (i == array->num_l1_pages - 1) ? array->num_data_pages % PTRS_PER_PAGE : PTRS_PER_PAGE;	\
				for (size_t j = 0; j < l2pages; j++) {	\
					free(l1_page[j]);	\
				}	\
			free(l1_page);	\	
      		}	\
		}	\
		else {	\
			for (size_t i = 0; i < array->num_data_pages; i++) {	\
				free(array->ptable[i]);	\
      		}	\
		}	\
	}	\
}
*/



#endif // ARRAYS_C_MACRO_H
