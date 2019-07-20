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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hashtable.h"

/* A struct to hold test data  */
typedef struct {
    int data;
} athing_t;


void do_sample() {
    printf("Sample usage\n");
    printf("\tCreate a new table with an initial capacity of 30\n");
    HT_T * ht = HT_new(30);
    if (ht == NULL) printf("Could not create hash table");

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


    printf("\tFree the table\n");
    //This will also free any Values that are still in the table
    HT_free(ht);
}

/* Test application */
int main(int charc, char ** argv) {
    do_sample();

    printf("Run some basic tests.... \n");
    printf("Make a new hash table\n");
    HT_T * t = HT_new(HT_DEFAULT_CAPACITY);
    if (! t) {
        printf("\tFailed to create new table. \n");
        return 1;
    }

    printf("Put some things into the table: \n");
    for(int i = 0; i < 12; i++) {
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        athing_t * athing = (athing_t*)malloc(sizeof(athing_t));
        athing->data = i;
        if (HT_put(t, key, len, athing) == NULL) {
            printf("PUT failed");
            return 1;
        }
    }

    printf("Elements in table [%zu] Cap [%zu]\n", t->n, t->cap);
    for(int i = 0; i < t->cap; i++) {
        printf("\t[%d] {hashcode: %zu, state: %d, loc: %zu}\n", 
            i, t->index[i].hashcode, t->index[i].state, t->index[i].loc );

    }

    printf("Check elements added exists\n");
    for(int i = 0; i < 12; i++) {
  
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        printf("\tKey exists: [%s : %d]\n", key, HT_exists(t, key, len));
    }
    
    printf("Checking ... Following keys should not exist\n");
    for(int i = 100; i < 102; i++) {
        char key[10] ; 
        int len = sprintf(key, "key%d", i); 
        printf("\tKey exists: [%s : %d]\n", key, HT_exists(t, key, len));
        if (HT_exists(t, key, len)) {
            printf("\tERROR - Key exists: { key: %s }", key);
            return 1;
        }
    }

    printf("Fetch items from the table\n");
    for(int i = 0; i < 12; i++) {
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        athing_t * athing = (athing_t *)HT_get(t, key, len);
        printf("\tGet A Thing:  { key: %s, data: %d}\n", key, (athing) ? athing->data : -9999);
        if (! athing) {
            printf("\tERROR - Could not fetch: { key: %s }", key);
            return 1;
        }
     }

    printf("Fetch non-existant items from the table\n");
    for(int i = 100; i < 102; i++) {
        /* the key is copied into the hash table. */
        /* so this key value an be replaced within the loop*/
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        athing_t * athing = (athing_t *)HT_get(t, key, len);
        if (athing) {
            printf("\tERROR - Returned value should be NULL: { key: %s }", key);
            return 1;
        }
     }

    printf("Remove items from the table\n");
    for(int i = 0; i < 12; i+=2) {
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        athing_t * athing = (athing_t *)HT_remove(t, key, len);
        printf("\tRemove a thing:  { key: %s, data: %d} REMOVED\n", key, (athing) ? athing->data : -9999);

        /*dont need the thing that have been removed */
        free(athing);
     }

    printf("Elements in table after removal [%zu]\n", t->n);
    for(int i = 0; i < t->cap; i++) {
        printf("\t[%d] {hashcode: %zu, state: %d, loc: %zu}\n", 
            i, t->index[i].hashcode, t->index[i].state, t->index[i].loc );
    }

    printf("Following keys should not exist - they have been removed \n");
    for(int i = 0; i < 12; i+=2) {
        /* the key is copied into the hash table. */
        /* so this key value an be replaced within the loop*/
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        if (HT_exists(t, key, len)) {
            printf("\tERROR - Key exists: { key: %s }", key);
            return 1;
        }
    }

    printf("Following keys should exist \n");
    for(int i = 1; i < 12; i+=2) {
        /* the key is copied into the hash table. */
        /* so this key value an be replaced within the loop*/
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        printf("\tKey exists: [%s : %d]\n", key, HT_exists(t, key, len));
        if (! HT_exists(t, key, len)) {
            printf("\tERROR - Key does not exists: { key: %s }", key);
            return 1;
        }
    }

    printf("Put **more** things into the table: \n");
    for(int i = 100; i < 106; i++) {
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        athing_t * athing = (athing_t*)malloc(sizeof(athing_t));
        athing->data = i;
        if (HT_put(t, key, len, athing) == NULL) {
            printf("PUT failed");
            return 1;
        }
    }

    printf("Check the keys exist in the table after add. \n");
    for(int i = 1; i < 12; i+=2) {
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        athing_t * athing = (athing_t *)HT_get(t, key, len);
        if (! athing) {
            printf("\tGet Failed - key does not exists: {  key: %s }", key);
            return 1; 
        } 
    }
    for(int i = 100; i < 106; i++) {
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        athing_t * athing = (athing_t *)HT_get(t, key, len);
        if (! athing) {
            printf("\tGet Failed - key does not exists: {  key: %s }", key);
            return 1; 
        } 
     }
    printf("elements in table  [%zu] Cap [%zu]\n", t->n, t->cap);
    for(int i = 0; i < t->cap; i++) {
        printf("\t[%d] {hashcode: %zu, state: %d, loc: %zu}\n", 
            i, t->index[i].hashcode, t->index[i].state, t->index[i].loc );
    }

    printf("Put even **more** things into the table to make it expand \n");
    for(int i = 200; i < 230; i++) {
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        athing_t * athing = (athing_t*)malloc(sizeof(athing_t));
        athing->data = i;
        if (HT_put(t, key, len, athing) == NULL) {
            printf("PUT failed\n");
            return 1;
        }
    }
    printf("elements in table  [%zu] Cap [%zu]\n", t->n, t->cap);
    for(int i = 0; i < t->cap; i++) {
        printf("\t[%d] {hashcode: %zu, state: %d, loc: %zu}\n", 
            i, t->index[i].hashcode, t->index[i].state, t->index[i].loc );
    }

    printf("Check added keys exist.\n");
    for(int i = 1; i < 12; i+=2) {
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        athing_t * athing = (athing_t *)HT_get(t, key, len);
        if (! athing) {
            printf("\tGet Failed - key does not exists: {  key: %s }", key);
            return 1; 
        } 
    }
    for(int i = 100; i < 106; i++) {
        /* the key is copied into the hash table. */
        /* so this key value an be replaced within the loop*/
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        athing_t * athing = (athing_t *)HT_get(t, key, len);
        if (! athing) {
            printf("\tGet failed: {  key: %s }", key);
            return 1; 
        }
    }
    for(int i = 200; i < 230; i++) {
        /* the key is copied into the hash table. */
        /* so this key value an be replaced within the loop*/
        char key[10]; 
        int len = sprintf(key, "key%d", i); 
        athing_t * athing = (athing_t *)HT_get(t, key, len);
        if (! athing) {
            printf("\tGet failed: {  key: %s }", key);
            return 1; 
        }
    }

    printf("Put item will empty string as key.\n");
    {
         athing_t * athing = (athing_t*)malloc(sizeof(athing_t));
         athing->data = 0; 
         if (HT_put(t, "", 0, athing) == NULL) {
            printf("PUT failed\n");
            return 1;
        }
    }
    printf("Check item with empty string key exists.\n");
    {
        if (! HT_exists(t, "", 0)) {
            printf("\tItem does not exist\n");
            return 1;    
        }
    }

    printf("Put item with NULL value.\n");
    {
        char * key = "null"; 
        int len = strlen(key);

        //Cannot tell if the put was successful by examining the return code 
        HT_put(t, key, len, NULL);
        if (! HT_exists(t, key, len)) {
            printf("\tItem was not added\n");
            return 1;  
        } 

        printf("Try to get item with NULL value.\n");
        athing_t * athing = HT_get(t, key, len);
        if (athing) {
            printf("\tERR - Expected item to be NULL\n");
            return 1;  
        }
    }

    printf("Put item with NULL as key\n");
    {
        athing_t * athing = (athing_t*)malloc(sizeof(athing_t));
        athing->data = 99; 
        if (HT_put(t, NULL, 0, athing) != NULL) {
            printf("\tERR - NULL key is not permitted\n");
            return 1; 
        }
    }

    printf("Free the table\n");
    HT_free(t);

    printf("Finished!!\n");
}
