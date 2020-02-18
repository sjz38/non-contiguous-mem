#ifndef ARRAYS_H
#define ARRAYS_H
#include <stdlib.h>
#include <stdio.h>


#define PAGE_SIZE (1024*32)
#define PTRS_PER_PAGE (PAGE_SIZE / sizeof(void*))
#define MAX_PAGES (PTRS_PER_PAGE * PTRS_PER_PAGE)
#define NUM_ELEMS (PAGE_SIZE / sizeof(T))
#define ELEMS_PER_L2 (PTRS_PER_PAGE * NUM_ELEMS)
#define single (num_data_pages == 1)
#define two_level (num_l1_pages > 1)

//#define single (0)
//#define two_level (1)

#define getL1Offset(i) (i / ELEMS_PER_L2)
#define getL2Index(i)  ((i / NUM_ELEMS) % PTRS_PER_PAGE)
#define getL2Offset(i) (i % NUM_ELEMS)

template<class T>
struct MemRegion {
  T* minValue;
  T* maxValue;
};

template <typename T>
class Array
{
private:
  void* ptable[PTRS_PER_PAGE];
  size_t num_elems;
  size_t num_l1_pages;
  size_t num_data_pages;
  
public:
  Array(size_t size, size_t off);
  Array(size_t size);
  ~Array();
  T &operator[](size_t index);
  MemRegion<T> getRegion(size_t index);
  //  void resize(size_t size);
};

template <typename T>
Array<T>::Array(size_t size)
{
  num_data_pages = (size / NUM_ELEMS) + 1;
  num_l1_pages = (num_data_pages / PTRS_PER_PAGE) + 1;
  //  printf("Size of element is %lu\n", sizeof(T));
  //printf("Num pages allocated is %lu\n", num_data_pages);
  // printf("Number of l1 pages is %lu\n", num_l1_pages);
    
  if (num_data_pages > MAX_PAGES) {
    printf("Cant support that many pages!!\n");
    exit(1);
  }
  num_elems = size;
  if (num_data_pages > 1) {
    if (two_level) {
      for (size_t i = 0; i < num_l1_pages; i++) {
	void* l1_page = malloc(PAGE_SIZE);
	ptable[i] = l1_page;
	size_t l2pages = (i == num_l1_pages - 1) ? num_data_pages % PTRS_PER_PAGE : PTRS_PER_PAGE;
	for (size_t j = 0; j < l2pages; j++) {
	  void **l1_page_cast = (void **)l1_page;
	  l1_page_cast[j] = malloc(PAGE_SIZE);
	}
      }
    } else {
      for (size_t i = 0; i < num_data_pages; i++) {
	void* page = malloc(PAGE_SIZE);
	ptable[i] = page;
      }
    }
  }
}

template <typename T>
Array<T>::~Array() {
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
    } else {
      for (size_t i = 0; i < num_data_pages; i++) {
	free(ptable[i]);
      }
    }
  }
}

/*
TODO implement a good implementation or resize
template <typename T>
Array<T>::resize(size_t size) {
  size_t new_data_pages = (size / NUM_ELEMS) + 1;
  size_t new_l1_pages = (new_data_pages / PTRS_PER_PAGE) + 1;
  if (size < num_elems) {
    num_elems = size;
    //free the unnecessary pages
    return;
  } else {
    //allocate any new pages
    //copy data and pointers to appropriate locations
    return;
  }  
}
*/
//this this the dumb and bad version of resize
template <typename T>
Array<T>* resize(size_t old_size, Array<T>* orig, size_t new_size) {
  Array<T>* result = new Array<T>(new_size);
  size_t copysize = (new_size > old_size) ? old_size : new_size;
  for (size_t i = 0; i < copysize; i++) {
    (*result)[i] = (* orig)[i];
  }
  return result;
}

template <typename T>
T &Array<T>::operator[](size_t index) {
  if (single) {
    T *entries = (T*) ptable;
    return entries[index];
  } else if (two_level) {
    T ***entries = (T***) ptable;
    size_t l1off = getL1Offset(index);
    T **l1_page = entries[l1off];
    size_t pageno = getL2Index(index);
    size_t offset = getL2Offset(index);
    T *page = l1_page[pageno];
    return page[offset];
  } else {
    T **entries = (T**) ptable;
    size_t pageno = index / NUM_ELEMS;
    size_t offset = index % NUM_ELEMS;
    T *page = entries[pageno];
    return page[offset];
  }
}

template <typename T>
void arrayCopy(Array<T>* destptr, size_t deststart, Array<T>* srcptr, size_t srcstart, size_t count) {
  size_t copied = 0;
  while(copied < count) {
    MemRegion<T> src = srcptr->getRegion(copied + srcstart);
    MemRegion<T> dest = destptr->getRegion(copied + deststart);
    while (copied < count && src.minValue <= src.maxValue && dest.minValue <= dest.maxValue) {
      *(dest.minValue) = *(src.minValue);
      src.minValue++;
      dest.minValue++;
      copied++;
    }
  }
}

template <typename T>
void copyInto(Array<T>* destptr, size_t startIdx, void* srcptr, size_t count) {
  size_t copied = 0;
  T* source = (T*) srcptr;
  while(copied < count) {
    MemRegion<T> dest = destptr->getRegion(copied + startIdx);
    while (copied < count && dest.minValue <= dest.maxValue) {
      *(dest.minValue) = *source;
      source++;
      dest.minValue++;
      copied++;
    }
  }
}

template <typename T>
void copyOutOf(void* destptr, Array<T>* srcptr, size_t startIdx, size_t count) {
  size_t copied = 0;
  T* dest = (T*) destptr;
  while(copied < count) {
    MemRegion<T> src = srcptr->getRegion(copied + startIdx);
    while (copied < count && src.minValue <= src.maxValue) {
      *dest = *(src.minValue);
      src.minValue++;
      dest++;      
      copied++;
    }
  }
}

template <typename T>
MemRegion<T> Array<T>::getRegion(size_t index) {
  MemRegion<T> result;
  if (single) {
    T *entries = (T*) ptable;
    result.minValue = &(entries[index]);
    result.maxValue = &(entries[NUM_ELEMS-1]);
  } else if (two_level) {
    T ***entries = (T***) ptable;
    size_t l1off = getL1Offset(index);
    T **l1_page = entries[l1off];
    size_t pageno = getL2Index(index);
    size_t offset = getL2Offset(index);
    T *page = l1_page[pageno];
    result.minValue = &(page[offset]);
    result.maxValue = &(page[NUM_ELEMS-1]);
  } else {
    T **entries = (T**) ptable;
    size_t pageno = index / NUM_ELEMS;
    size_t offset = index % NUM_ELEMS;
    T *page = entries[pageno];
    result.minValue = &(page[offset]);
    result.maxValue = &(page[NUM_ELEMS-1]);
  }
  return result;
}

#endif