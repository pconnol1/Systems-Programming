/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))


typedef struct{
	size_t size;
	header * next;
	header * prev;
}header;

typedef struct{
	header * head;
}footer;

int mm_init(void){
	header * p= mem_sbrk(SIZE_T_SIZE+ALIGN(sizeof(footer));
		p->next=p;
		p->prev=p;
		p->size=SIZE_T_SIZE+ALIGN(sizeof(footer)+1;
	
	footer *q=(footer *)((char *)p+SIZE_T_SIZE);
		q->head = p;
	
	return 0;
}

//figure out freecount
void *mm_malloc(size_t size)
{
	size_t blk_size = ALIGN(size+SIZE_T_SIZE+ALIGN(sizeof(footer));
	header *p;
	if(freecount > 0 && (p=find_fit(blk_size))!=NULL){
		if(((p->size)&~1)-newsize>=7+SIZE_T_SIZE+ALIGN(sizeof(footer))){
			header *p = mem_sbrk(size);
			p->size=size;
			((footer *)((char *)p+((p->size)&~1)-ALIGN(sizeof(footer)))->head = p;	
			header *split =(header *)((char *)p+(p->size &~1)
			split->size=oldsize-p->size;
			((bFooter *)((char *)split+((split->size)&~1)-ALIGN(sizeof(footer))))->head = split;
			int index = getIndex(split->size-SIZE_T_SIZE-ALIGN(sizeof(footer)));
			split->next = heads[index].next;
			heads[index].next=split;
			split->next->prev=split;
			split->prev=&heads[index];
			++freecount;
		}
	}
}