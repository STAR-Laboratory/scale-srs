#ifndef __MALLOC_LIB_H__

typedef struct SMalloc_Raw_struct{
    char * ptr;
    int  cur_size;
} SMalloc_Raw;

typedef struct SMalloc_Entry_struct{
    void * data;
    struct SMalloc_Entry_struct *next;
} SMalloc_Entry;

void *smalloc(int nbytes);
void sfree(int nbytes, void *item);


#define __MALLOC_LIB_H__
#endif 


