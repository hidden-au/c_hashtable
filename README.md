# A minimal hash table implementation.

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
```C
    #include "hashtable.h"

    printf("\tCreate a new table with an initial capacity of 30\n");
    HT_T * ht = HT_new(30);
    if (ht == NULL) printf("Could not create hash table.");

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
```
