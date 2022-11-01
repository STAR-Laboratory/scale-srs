#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "global_types.h"
#include "malloc_lib.h"
#include "hash_lib.h"

#define HASH_INDEX(table, key)	(((uns)(key)) % (table)->buckets)


/**************************************************************************************/
/* init_hash_table: */

void init_hash_table (Hash_Table *table, char name[], uns buckets, uns data_size)
{
/*    char *result = malloc(strlen(name) + 1);
    if (result != NULL) {
    strcpy(table->name, result);
    }*/
    table->name	     = strdup(name);
    table->buckets   = buckets;
    table->data_size = data_size;
    table->count     = 0;
    table->entries   = (Hash_Table_Entry **)calloc(buckets, sizeof(Hash_Table_Entry *));
}



/**************************************************************************************/
/* hash_table_clear: */

void hash_table_clear (Hash_Table *table)
{
    Hash_Table_Entry *temp0;
    Hash_Table_Entry *temp1;
    uns count = 0;
    uns ii;

    for (ii = 0 ; ii < table->buckets ; ii++) {
      temp0 = table->entries[ii];
      while (temp0) {
	temp1 = temp0->next;
	sfree(table->data_size, temp0->data);
	sfree(sizeof(Hash_Table_Entry), temp0);
	temp0 = temp1;
	count++;
      }
      table->entries[ii] = NULL;
    }
    assert( count == (uns)table->count);
    table->count = 0;
}


/**************************************************************************************/
/* Return the data pointer if it hits, NULL otherwise */

void *hash_table_access (Hash_Table const *table, int64 key)
{
  uns index = HASH_INDEX(table, key);
  Hash_Table_Entry *bucket = table->entries[index];
  Hash_Table_Entry *temp;
  
  for (temp = bucket ; temp != NULL ; temp = temp->next){
    if (temp->key == key) return temp->data;
  }
  
  return NULL;
}


/**************************************************************************************/
/* Return the data pointer if it hits an existing entry.  Otherwise, allocate a new
   entry and return its data pointer. */

void *hash_table_access_create (Hash_Table *table, int64 key, Flag *new_entry)
{
  uns index = HASH_INDEX(table, key);
  Hash_Table_Entry *bucket   = table->entries[index];
  Hash_Table_Entry *new_hash = NULL;
  Hash_Table_Entry *temp;
  Hash_Table_Entry *prev     = NULL;

  *new_entry = FALSE;
  assert(index < table->buckets);
  for (temp = bucket ; temp != NULL ; temp = temp->next) {
    if (temp->key == key) return temp->data; 
    else                  prev = temp;
  }
  table->count++;
  *new_entry = TRUE;
  new_hash = (Hash_Table_Entry *)smalloc(sizeof(Hash_Table_Entry));
  assert(new_hash);
  new_hash->key   = key;
  new_hash->next  = NULL;
  new_hash->data  = (void *)smalloc(table->data_size);
  assert(new_hash->data);
  
  if (prev) prev->next = new_hash;
  else      table->entries[index] = new_hash;
  
  return new_hash->data;
}


/**************************************************************************************/
/* hash_table_access_delete: look up an entry and delete it. return
   TRUE if it was found, FALSE otherwise */

Flag hash_table_access_delete (Hash_Table *table, int64 key)
{
  uns index = HASH_INDEX(table, key);
  Hash_Table_Entry *bucket = table->entries[index];
  Hash_Table_Entry *temp;
  Hash_Table_Entry *prev   = NULL;
  
  for (temp = bucket ; temp != NULL ; temp = temp->next) {
    if (temp->key == key) {
      Hash_Table_Entry *next_ptr = temp->next;
      sfree(table->data_size, temp->data);
      sfree(sizeof(Hash_Table_Entry), temp);
      if (prev) prev->next = next_ptr;
      else table->entries[index] = next_ptr;
      table->count--;
      assert(table->count >= 0);
      return TRUE;
    } else
      prev = temp;
  }

    return FALSE;
}
