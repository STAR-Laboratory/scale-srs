#ifndef __HASH_LIB_H__

#include "global_types.h"


/**************************************************************************************/
/* Types */

typedef struct Hash_Table_Entry_struct {
    int64 key;
    void *data;
    struct Hash_Table_Entry_struct *next;
} Hash_Table_Entry;

typedef struct Hash_Table_struct {
    char *name;
    uns   buckets;
    uns   data_size;
    int  count;		// total number of elements in the hash table
    Hash_Table_Entry **entries;
} Hash_Table;


/**************************************************************************************/
/* Prototypes */

void    init_hash_table                  (Hash_Table *, char [], uns, uns);
void    hash_table_clear                 (Hash_Table *);
void *  hash_table_access                (Hash_Table const *, int64);
void *  hash_table_access_create         (Hash_Table *, int64, Flag *);
Flag    hash_table_access_delete         (Hash_Table *, int64);

/**************************************************************************************/



#define __HASH_LIB_H__
#endif 
