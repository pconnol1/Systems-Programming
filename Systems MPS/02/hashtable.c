#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

/* Patrick Connolly
   9/13/2016
   CS351
   Daniel J. Bernstein's "times 33" string hash function, from comp.lang.C;
   See https://groups.google.com/forum/#!topic/comp.lang.c/lSKWXiuNOAk */
unsigned long hash(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

hashtable_t *make_hashtable(unsigned long size) {
  hashtable_t *ht = malloc(sizeof(hashtable_t));
  ht->size = size;
  ht->buckets = calloc(size, sizeof(bucket_t *));
  return ht;
}

void ht_put(hashtable_t *ht, char *key, void *val) {
  /* FIXME: the current implementation doesn't update existing entries */
  unsigned int idx = hash(key) % ht->size;
  bucket_t *b = malloc(sizeof(bucket_t));
  b->key=key;
  b->val=val; 
  b->next = NULL;

  if(ht->buckets[idx] != NULL)
    {
      bucket_t *bt = ht->buckets[idx];
      while(1)
	{
	  if(strcmp(bt->key,b->key)==0)
	    {
	      bt->val = b->val;
	      free(b->key);
	      free(b->next);
	      //free(b->val);
	      free(b);
	      break;
	    }
	    if(bt->next == NULL)
	      {
		bt->next = b;
		break;
	      }
	    bt=bt->next;
	}
    }
      else{
	b->next = ht->buckets[idx];
	ht->buckets[idx]=b;
      }
  /* b->key = key;
  b->val = val;
  b->next = ht->buckets[idx];
  ht->buckets[idx] = b;*/
}

void *ht_get(hashtable_t *ht, char *key) {
  unsigned int idx = hash(key) % ht->size;
  bucket_t *b = ht->buckets[idx];
  while (b) {
    if (strcmp(b->key, key) == 0) {
      return b->val;
    }
    b = b->next;
  }
  return NULL;
}

void ht_iter(hashtable_t *ht, int (*f)(char *, void *)) {
  bucket_t *b;
  unsigned long i;
  for (i=0; i<ht->size; i++) {
    b = ht->buckets[i];
    while (b) {
      if (!f(b->key, b->val)) {
        return ; // abort iteration
      }
      b = b->next;
    }
  }
}

void free_hashtable(hashtable_t *ht) {
  free(ht); // FIXME: must free all substructures!
}

/* TODO */
void  ht_del(hashtable_t *ht, char *key) {
	unsigned int idx = hash(key) % ht->size;
	bucket_t *b = ht->buckets[idx];
	if(strcmp(b->key,key)==0)
	{
		ht->buckets[idx]=b->next;
		free(b->key);
		free(b->val);
		//free(b->next);
		free(b);
		return;
	}
	bucket_t *last = b;
	b=b->next;
	while(1)
	{
		if(strcmp(b->key,key)==0)
		{
			last->next=b->next;
			free(b->key);
			free(b->val);
			//free(b->next);
			free(b);
			break;
		}
		last=last->next;
		b=b->next;
	}
}

void  ht_rehash(hashtable_t *ht, unsigned long newsize) {
	hashtable_t *newHT = make_hashtable(newsize);
	
	bucket_t *b;
	unsigned long i;
	for(i=0; i<ht->size; i++)
	{
		b=ht->buckets[i];
		while(b)
		{
			ht_put(newHT,b->key,b->val);
			b=b->next;
		}
	}
	ht->size =newsize;
	ht->buckets = newHT->buckets;
	
	
}
