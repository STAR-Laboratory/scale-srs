#include <stdlib.h>
#include <assert.h>
#include "global_types.h"
#include "malloc_lib.h"

/* Defines */         

#define MAX_SMALLOC   32768
#define WRAPPER_BLOCK 1024
#define SMALLOC_BLOCK (0x1 << 20)

/* Global Variables */
static char * raw_mem_ptr  = NULL;
static int  raw_mem_size   = 0;
static SMalloc_Entry * wrapper_free_list = NULL;
static SMalloc_Entry * smalloc_free_list[MAX_SMALLOC];

static inline SMalloc_Entry * get_wrapper();
static inline void free_wrapper(SMalloc_Entry *wrap);

/**************************************************************************************/
/* get_wrapper */ 
static inline SMalloc_Entry * get_wrapper(){
    SMalloc_Entry * tmp;

    if (wrapper_free_list == NULL){
	char * wrap_mem = (char *) malloc (sizeof(SMalloc_Entry) * WRAPPER_BLOCK);
	SMalloc_Entry * cur_wrap;
	int ii;
	assert(wrap_mem);
	for (ii = 0; ii < WRAPPER_BLOCK; ii++){
	    cur_wrap = (SMalloc_Entry *) wrap_mem;
	    cur_wrap->next = wrapper_free_list;
	    wrapper_free_list = cur_wrap;
	    wrap_mem += sizeof (SMalloc_Entry);
	}
    }
    tmp = wrapper_free_list;
    wrapper_free_list = wrapper_free_list->next;
    return tmp;
}

/**************************************************************************************/
/* free_wrapper */ 
static inline void free_wrapper(SMalloc_Entry *wrap){
    wrap->next = wrapper_free_list;
    wrapper_free_list = wrap;
}

/**************************************************************************************/
/* smalloc */
void *smalloc (int nbytes){
    SMalloc_Entry * tmp;
    void * ptr = NULL;

    assert(nbytes < MAX_SMALLOC);
    if (smalloc_free_list[nbytes]){
	tmp = smalloc_free_list[nbytes];
	smalloc_free_list[nbytes] = smalloc_free_list[nbytes]->next;
	ptr = tmp->data;
	free_wrapper(tmp);
    } else { /* need to allocate some memory */
	if (nbytes > raw_mem_size) {
	  while (raw_mem_size >= (int)sizeof(SMalloc_Entry)){ /* never can have too many wrappers */
		tmp = (SMalloc_Entry *) raw_mem_ptr;
		free_wrapper(tmp);
		raw_mem_ptr  += sizeof(SMalloc_Entry);
		raw_mem_size -= sizeof(SMalloc_Entry);
	    }
	    /* Yes, we waste up to 7 bytes per SMALLOC_BLOCK allocation */
	    raw_mem_ptr = (char *) malloc(SMALLOC_BLOCK);
	    assert(raw_mem_ptr);
	    raw_mem_size = SMALLOC_BLOCK;
	}
	ptr = raw_mem_ptr;
	raw_mem_ptr      += nbytes;
	raw_mem_size     -= nbytes;
    }
    assert(ptr);
    return ptr;
}

/**************************************************************************************/
/* sfree */
void sfree (int nbytes, void * item){
    SMalloc_Entry *wrapper = get_wrapper();
    wrapper->next = smalloc_free_list[nbytes];
    wrapper->data = item;
    smalloc_free_list[nbytes] = wrapper;
}

