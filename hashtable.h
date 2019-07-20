/*
MIT License

Copyright (c) 2019 Alec Au

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __SIMPLE_HASHTABLE__
#define __SIMPLE_HASHTABLE__

/*
A minimal hash table implementation.

Keys are null terminated C style strings.
Maximum key size is 31 bytes. Attempting to add an item into 
the table with a key size greater then the maximum
allowed will fail. 

The key is copied into the hash table. 

Values must be objects allocated on the free store. The
hash table will store a pointer to the Value. Values added
to the table should not be free'd unless it is removed from
the table first. 

When the table is first created, the expected initial capacity
should be supplied. The actual size of the table created would
be rounded up to the next power of 2. (This is a performance
optimisation). 

As items are added to the table, it will automatically 
increase in capacity if required. By default, the table will
increase in capacity when it is 75% full. A different loading
factor can optionally be supplied at the time the table is 
created. 

To prevent memory leaks, use the provided HT_free function
to free the table. Note that this function will also 
free Values added to the table. If you do not want the
Values to be free'd ensure they are removed from the 
table before HT_free is called. For Values that require 
special destruction logic, a user defined free function can 
optionally be supplied at the time the table is created.

The implementation provides amortised O(1) access performance
for adding, removal and retrieving items. Hash collisions 
are managed using a basic linear probing algorithm. (i.e
the table is searched until a free spot is found). This can
cause performance to degrade for very full tables.

Sample code:

    #include "hashtable.h"

    printf("\tCreate a new table with an initial capacity of 30\n");
    HT_T * ht = HT_new(30);
    if (ht == NULL) printf("Could not create HT");

    printf("\tCreate a Value to be added to the table\n");
    int * value = (int*) malloc(sizeof(int)); 

    printf("\tAdd a value to the table\n");
    char * key = "key1";
    size_t key_len = 4;
    if (HT_put(ht, key, key_len, value) == NULL)  printf("Put failed\n");

    printf("\tCheck if the key is in the table - ");
    if (HT_exists(ht, key, key_len)) 
        printf("Key exists\n");
    else 
        printf("Key does not exists\n");

    printf("\tRetrieve a Value - ");
    int * new_v = (int*) HT_get(ht, key, key_len);
    if (new_v) 
        printf("Got the value\n");
    else 
        printf("Value not found or is NULL\n");

    printf("\tRemove an item - ");
    int * removed_v = (int*) HT_remove(ht, key, key_len);
    if (removed_v) 
        printf("Value was removed from the table\n");
    else 
        printf("Value not found or is NULL\n");

    //This will also free any Values that are still in the table
    printf("\tFree the table\n");
    HT_free(ht);
*/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define HT_MAX_KEY_LEN          32
#define HT_DEFAULT_CAPACITY     16
#define HT_MAX_CAPACITY         ( 1 << 30 )
#define HT_DEFAULT_LOAD_FACTOR  0.75
#define HT_BUCKET_OCCUPIED      0
#define HT_EMPTY_BUCKET         -1
#define HT_DELETED_BUCKET       -2

#define HT_CALC_LOC(ht, hashcode)  ( hashcode & ( ht->cap - 1 )   )

/* A hashtable bucket - used to create an index to the item */
typedef struct {
    unsigned long  hashcode;    /* Hashcode of the key */
    size_t  loc;                /* Location where the value can be found */
    int     state;              /* Can be EMPTY, DELETED or OCCUPIED */
} HT_BUCKET_T; 

/* Store the key and the value */
typedef struct {
    char    k[HT_MAX_KEY_LEN]; /* The key */
    int     k_len;             /* Length of the key */
    void *  v;                 /* A pointer to the value */
} HT_ENTRY_T; 

/* The Hashtable */
typedef struct {
    HT_BUCKET_T * index;       /* Pointer to an array of indexes */
    HT_ENTRY_T  * values;      /* Pointer to an array of values  */
    size_t        n;           /* Number of elements currently stored */
    size_t        cap;         /* Maximum number of elements that can currently be stored */
    size_t        expand;      /* High water mark - expand the table when n > expand */
    size_t        max_probe;   /* Maximum num of probes used to find a bucket to store in the Index */
    size_t        ins_loc;     /* Location to insert new Values */
    float         load_factor; /* Used to calculate the value of "expand" */

    /* Ptr to function to compare keys for equality */
    int           (*cmp)      (const char*, const char*, size_t);

    /* Ptr to function to calculate a hash code from a given key */ 
    unsigned long (*hashcode) (const char*, size_t);

    /* Ptr to function to deallocate a value stored in the table */
    void          (*dealloc)  (void *); 
} HT_T;

/*
 * forward declarations.
 */

HT_T *  
HT_new(size_t requested_capacity);

HT_T *  
HT_new_ex(
    size_t        requested_capacity, 
    float         load_factor,
    unsigned long (*hashcode) (const char*, size_t),
    int           (*cmp)      (const char*, const char*, size_t),
    void          (*dealloc)  (void *)
);

void *
HT_put(
    HT_T * table, 
    const char * key,
    size_t key_len,
    void * value
);

void *
HT_get(
    HT_T * table,
    const char * key,
    size_t key_len
);

int 
HT_exists(
   HT_T * table,
   const char * key,
   size_t key_len
);

void *
HT_remove(
    HT_T * table,
    const char * key,
    size_t key_len
);

void
HT_free(
    HT_T * table
);

unsigned long 
_HT_hash(
   const char * key,
   size_t key_len
); 

 
size_t
_HT_calc_expansion (
    float  load_factor, 
    size_t capacity
);


size_t
_HT_round_up (
    size_t val
);

HT_T *  
_HT_expand(
    HT_T * table
);

/* Implementations */

HT_T *  
HT_new(
    size_t requested_capacity
)
{
    return 
        HT_new_ex(
            requested_capacity,
            HT_DEFAULT_LOAD_FACTOR,
            &_HT_hash,
            &strncmp,
            &free
        );
}


HT_T *  
HT_new_ex(
    size_t        requested_capacity, 
    float         load_factor,
    unsigned long (*hashcode) (const char*, size_t),
    int           (*cmp)      (const char*, const char*, size_t),
    void          (*dealloc)  (void *)
)
{
    size_t capacity; 
    size_t expand;
    HT_BUCKET_T * index;
    HT_ENTRY_T  * values;
    HT_T        * table;

    if (requested_capacity < HT_DEFAULT_CAPACITY) requested_capacity = HT_DEFAULT_CAPACITY;
    if (requested_capacity > HT_MAX_CAPACITY)     requested_capacity = HT_MAX_CAPACITY;
    if (load_factor < 0.1 || load_factor > 1.0)   load_factor = HT_DEFAULT_LOAD_FACTOR;

    capacity = _HT_round_up(requested_capacity);
    expand = _HT_calc_expansion(load_factor, capacity);
    
    index = (HT_BUCKET_T *) calloc(capacity, sizeof(HT_BUCKET_T));
    if (index == NULL) return NULL;
    for(size_t i = 0; i < capacity; i++) {
        index[i].hashcode = 0;
        index[i].loc      = 0;
        index[i].state    = HT_EMPTY_BUCKET;     
    }
    
    values = (HT_ENTRY_T *) calloc(capacity, sizeof(HT_ENTRY_T));
    if (values == NULL) {
        free(index);
        return NULL;
    }
    for(size_t i = 0; i < capacity; i++) {
        memset(values[i].k, '\0', HT_MAX_KEY_LEN);
        values[i].k_len = 0;
        values[i].v     = NULL;
    }

    table = (HT_T *) malloc(sizeof(HT_T));
    if (table == NULL) {
        free(values);
        free(index);
        return NULL;
    }

    table->index       = index;
    table->values      = values;
    table->n           = 0;
    table->cap         = capacity;
    table->expand      = expand; 
    table->ins_loc     = 0;
    table->max_probe   = 0;
    table->load_factor = load_factor;
    table->cmp         = cmp;
    table->hashcode    = hashcode;
    table->dealloc     = dealloc;

    return table;
}


void *
HT_put(
    HT_T * table, 
    const char * key,
    size_t key_len,
    void * value
)
{
    unsigned long hashcode;
    size_t        index_loc;
    size_t        probe_len; 

    if (table == NULL)                  return NULL;
    if (key == NULL)                    return NULL;
    if (key_len > HT_MAX_KEY_LEN - 1)   return NULL;

    if (table->n > table->expand || table->n == table->cap || table->ins_loc == table->cap) {
        /* expand the table size to hold more elements */
        if (_HT_expand(table) == NULL)   return NULL; 
    }

    hashcode = table->hashcode(key, key_len); 

    /* Loop through the table 
        1) start at the index calculated by from the hash code
        2) check if the location is free * if so - can insert into this location 
        3) if location is occupied - check to see if it is possible to update
    */
    for (
        probe_len = 0, index_loc = HT_CALC_LOC(table, hashcode); 
        probe_len <= table->cap;
        index_loc = HT_CALC_LOC(table, hashcode + (++probe_len))
     ) {
         HT_BUCKET_T * b = &table->index[index_loc];
         HT_ENTRY_T  * e;
         
         if ( b->state == HT_BUCKET_OCCUPIED && b->hashcode == hashcode ) {
             /* Update an existing element ?? */
              e = &table->values[b->loc];
              if (e->k_len == key_len && table->cmp(key, e->k, key_len) == 0) {
                  /* keys are equal so can update the value */
                  e->v = value;
                  return value;
              }
         }
         else if (b->state != HT_BUCKET_OCCUPIED) {
             size_t ins_loc = (b->state == HT_EMPTY_BUCKET) ? table->ins_loc++ : b->loc;
             
             e = &table->values[ins_loc];        
             e->k_len = key_len;
             e->v = value;   
             strncpy(e->k, key, key_len);
             e->k[key_len + 1] = '\0';

             b->loc = ins_loc;
             b->state = HT_BUCKET_OCCUPIED;
             b->hashcode = hashcode;

             if (probe_len > table->max_probe) table->max_probe = probe_len;
             table->n++; 

             return value;
         }
     }

    /* could not insert the element 
        - this would most likely occur if we cannot find an unoccupied location in the table */
    return NULL;
}

void *
HT_get(
    HT_T * table,
    const char * key,
    size_t key_len
)
{
    unsigned long hashcode;
    size_t        index_loc;
    size_t        probe_len; 

    if (table == NULL)                  return NULL;
    if (key == NULL)                    return NULL;
    if (key_len > HT_MAX_KEY_LEN - 1)   return NULL;

    hashcode = table->hashcode(key, key_len); 

     /* Loop through the table 
        1) start at the index calculated by from the hash code
        2) if location is occupied - check to see if the key exists.
        3) If the location is empty - then could  not locate item.
    */
    for (
        probe_len = 0, index_loc = HT_CALC_LOC(table, hashcode); 
        probe_len <= table->max_probe;
        index_loc = HT_CALC_LOC(table, hashcode + (++probe_len))
     ) {
         HT_BUCKET_T * b = &table->index[index_loc];
         HT_ENTRY_T  * e;
        
         if ( b->state == HT_BUCKET_OCCUPIED && b->hashcode == hashcode ) {
            /* have we found the key in the table */
            e = &table->values[b->loc];
            if (e->k_len == key_len && table->cmp(key, e->k, key_len) == 0) {
                return e->v;
            }
         }
         else if ( b->state == HT_EMPTY_BUCKET ) {
             /* did not find the key */
             return NULL;
         }
     }

     /* did not find the key */
     return NULL;
}

void *
HT_remove(
    HT_T * table,
    const char * key,
    size_t key_len
)
{
    unsigned long hashcode;
    size_t        index_loc;
    size_t        probe_len; 

    if (table == NULL)                  return NULL;
    if (key == NULL)                    return NULL;
    if (key_len > HT_MAX_KEY_LEN - 1)   return NULL;

    hashcode = table->hashcode(key, key_len); 

     /* Loop through the table 
        1) start at the index calculated by from the hash code
        2) if location is occupied - check to see if the key exists.
        3) If the location is empty - then could  not locate item.
    */
    for (
        probe_len = 0, index_loc = HT_CALC_LOC(table, hashcode); 
        probe_len <= table->max_probe;
        index_loc = HT_CALC_LOC(table, hashcode + (++probe_len))
     ) {
         HT_BUCKET_T * b = &table->index[index_loc];
         HT_ENTRY_T  * e;
        
         if ( b->state == HT_BUCKET_OCCUPIED && b->hashcode == hashcode ) {
            /* have we found the key in the table */
            e = &table->values[b->loc];
            if (e->k_len == key_len && table->cmp(key, e->k, key_len) == 0) {
                void * found_value = e->v;

                e->k[0] = '\0';
                e->k_len = 0;
                e->v = NULL;

                b->state = HT_DELETED_BUCKET;
                b->hashcode = 0;

                table->n--;

                return found_value;
            }
         }
         else if ( b->state == HT_EMPTY_BUCKET ) {
             /* did not find the key */
             return NULL;
         }
     }

     /* did not find the key */
     return NULL;
}


int 
HT_exists(
   HT_T * table,
   const char * key,
   size_t key_len
)
{
    unsigned long hashcode;
    size_t        index_loc;
    size_t        probe_len; 

    if (table == NULL)                  return 0;
    if (key == NULL)                    return 0;
    if (key_len > HT_MAX_KEY_LEN - 1)   return 0;

    hashcode = table->hashcode(key, key_len); 

     /* Loop through the table 
        1) start at the index calculated by from the hash code
        2) if location is occupied - check to see if the key exists.
        3) If the location is empty - then could  not locate item.
    */
    for (
        probe_len = 0, index_loc = HT_CALC_LOC(table, hashcode); 
        probe_len <= table->max_probe;
        index_loc = HT_CALC_LOC(table, hashcode + (++probe_len))
     ) {
         HT_BUCKET_T * b = &table->index[index_loc];
         HT_ENTRY_T  * e;

         if ( b->state == HT_BUCKET_OCCUPIED && b->hashcode == hashcode ) {
            /* have we found the key in the table */
            e = &table->values[b->loc];
            if (e->k_len == key_len && table->cmp(key, e->k, key_len) == 0) {
                return 1;
            }
         }
         else if ( b->state == HT_EMPTY_BUCKET ) {
             /* did not find the key */
             return 0;
         }
     }

     /* did not find the key */
     return 0;

}

void
HT_free(
    HT_T * table
)
{
    if (table == NULL)   return;

    /* free the values stored in the table */
    for(int i = 0; i < table->cap; i++) {
        HT_BUCKET_T * b = &table->index[i];
        if (b->state == HT_BUCKET_OCCUPIED) {
            void * value = table->values[b->loc].v;
            if (value != NULL)   table->dealloc(value);
        }
    }

    free(table->values);
    free(table->index);
    free(table);
}

HT_T *  
_HT_expand(
    HT_T * table
)
{
    size_t capacity; 
    size_t existing_capacity;
    size_t expand;
    HT_BUCKET_T * index;
    HT_ENTRY_T  * values;
    HT_BUCKET_T * existing_index;
    HT_ENTRY_T  * existing_values;
    size_t        index_loc;
    size_t        probe_len; 

    existing_capacity = table->cap;
    existing_index = table->index;
    existing_values = table->values; 

    capacity = _HT_round_up(table->cap * 2);
    expand = _HT_calc_expansion(table->load_factor, capacity);

    index = (HT_BUCKET_T *) calloc(capacity, sizeof(HT_BUCKET_T));
    if (index == NULL) return NULL;
    for(size_t i = 0; i < capacity; i++) {
        index[i].hashcode = 0;
        index[i].loc      = 0;
        index[i].state    = HT_EMPTY_BUCKET;     
    }
    
    values = (HT_ENTRY_T *) calloc(capacity, sizeof(HT_ENTRY_T));
    if (values == NULL) {
        free(index);
        return NULL;
    }

    for(size_t i = 0; i < capacity; i++) {
        memset(values[i].k, '\0', HT_MAX_KEY_LEN);
        values[i].k_len = 0;
        values[i].v     = NULL;
    }

    table->index      = index;
    table->values     = values;
    table->cap        = capacity;
    table->expand     = expand; 
    table->max_probe  = 0;

    /* copy to new values */
    for(int i = 0; i < existing_capacity ; i++) {
        values[i] = existing_values[i];
    }

    /* copy to new index */    
    for(int i = 0; i < existing_capacity ; i++) {
        HT_BUCKET_T * existing_b = &existing_index[i];
        unsigned long hashcode   = existing_b->hashcode; 

        /* find the location in the new index to copy the bucket to */
        if (existing_b->state == HT_BUCKET_OCCUPIED) {
            int added = 0;
            for (
                probe_len = 0, index_loc = HT_CALC_LOC(table, hashcode); 
                probe_len <= table->cap;
                index_loc = HT_CALC_LOC(table, hashcode + (++probe_len))
            ) {
                HT_BUCKET_T * new_b = &index[index_loc];
                if (new_b->state != HT_BUCKET_OCCUPIED) {
                    index[index_loc] = existing_index[i];
                 
                    if (probe_len > table->max_probe) table->max_probe = probe_len;

                    added = 1;
                    break;
                }
            }

            if (!added) {
                printf("error !! item not added");
            }
        }
    }

    /* free the old index and old values array */
    free(existing_index);
    free(existing_values);

    return table;
}

size_t
_HT_calc_expansion (
    float  load_factor, 
    size_t capacity
) 
{
    return (size_t)(load_factor * capacity);
}

size_t
_HT_round_up (
    size_t v
)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;

    return v;
}

unsigned long
_HT_hash(
    const char * key,
    size_t key_len
)
{
    unsigned long hash = 5381;
    int c;
    size_t n = 0;

    while ((c = *key++) && n++ <= key_len) {
        hash = ((hash << 5) + hash) + c; 
    }

    return hash;
}


#endif 
