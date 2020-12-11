/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

#ifdef DEBUG_COUNT
int op_count;
#endif

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define HEADER_SIZE ALIGN(sizeof(header))
#define FOOTER_SIZE ALIGN(sizeof(footer))
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

typedef struct bHeader header;
typedef struct bFooter footer;
header * find_fit(size_t size);

struct bHeader{
    size_t size;
    header * next;
    header * prior;
};
struct bFooter{
    header * head;
};

#define NUM_BINS 10
struct bHeader heads[NUM_BINS];
//
int cutoff[NUM_BINS]={17,65,113,129,449,1621,4073,4096,8191,INT_MAX};

int getIndex(size_t size){
    int index;
    for(index = 0; index<NUM_BINS; ++index){
        if(size< cutoff[index])
            break;
    }
    return index;
}



int freecount;
/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{

    freecount = 0;
    header *p = mem_sbrk(HEADER_SIZE+FOOTER_SIZE);
    p->next=p;
    p->prior=p;
    p->size=HEADER_SIZE+FOOTER_SIZE+1;
    footer *q=(footer *)((char *)p+HEADER_SIZE);
    q->head=p;
    int i;
    for(i = 0; i<NUM_BINS;++i){
        heads[i].next=&heads[i];
        heads[i].prior=&heads[i];
        heads[i].size=HEADER_SIZE+FOOTER_SIZE+1;
    }
    
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t newsize = ALIGN(size + SIZE_T_SIZE+ FOOTER_SIZE);
    header *p;
    if(freecount > 0 && (p=find_fit(newsize))!=NULL){
        if(((p->size)&~1)-newsize>=7+HEADER_SIZE+FOOTER_SIZE){
            size_t oldsize = p->size;
            p->size = newsize;
            ((footer *)((char *)p+((p->size)&~1)-FOOTER_SIZE))->head = p;
            header *split=(header *)((char *)p + (p->size & ~1));
            split->size=oldsize-p->size;
            ((footer *)((char *)split+((split->size)&~1)-FOOTER_SIZE))->head = split;
            int index = getIndex(split->size-SIZE_T_SIZE-FOOTER_SIZE);
            split->next = heads[index].next;
            heads[index].next=split;
            split->next->prior=split;
            split->prior=&heads[index];
            ++freecount;
        }
        if(freecount>0){
            --freecount;
        }
        p->prior->next=p->next;
        p->next->prior=p->prior;
        p->size|= 1;
    }else{
        header * last=(((footer *)((char *)mem_heap_hi()+1-FOOTER_SIZE))->head);
        if((freecount>0)&&(!(last->size &1))){
            --freecount;
            mem_sbrk(newsize-last->size);
            last->size = newsize | 1;
            ((footer *)((char *)last+((last->size)&~1)-FOOTER_SIZE))->head = last;
            last->prior->next=last->next;
            last->next->prior=last->prior;
            p = last;
        } else {
            p = mem_sbrk(newsize);
            if ((long)p == -1){
                return NULL;
            }
            else {
                p->size=newsize | 1;
                footer *q=(footer *)((char *)p+((p->size)&~1)-FOOTER_SIZE);
                q->head=p;
            }
        }
    }
    
    return (void *)((char *)p+sizeof(size_t));
}


header * find_fit(size_t size){
    int index = getIndex(size);
    header * p ;
    for(; index<NUM_BINS; ++index){
        for(p = heads[index].next; (p != &heads[index])&&(p->size<size+SIZE_T_SIZE+FOOTER_SIZE);p=p->next);
        if(p != &heads[index])
            return p;
    }
    return NULL;
}




/*
 * mm_free
 */
void mm_free(void *ptr){
    ++freecount;
    //Get the header of the current block
    header *p = (header *)((char *)ptr - SIZE_T_SIZE);
    //back up and get the previous block's header
    header *prior = ((footer *)((char *)p - FOOTER_SIZE))->head;
    //set the free bit
    p->size &= ~1;
    //if the previous block is also free we coallese
    if(!((prior->size)&1)){
        //link the list around the current block
        prior->prior->next=prior->next;
        prior->next->prior=prior->prior;
        prior->size+=p->size;
        ((footer *)((char *)p+((p->size)&~1)-FOOTER_SIZE))->head = prior;
        p = prior;
        --freecount;
    }
    header *next=(header *)((char *)p + (p->size &= ~1));
    if(((void *)((char *)p+p->size) <= mem_heap_hi()) && !((next->size)&1)){
        p->size+=next->size;
        ((footer *)((char *)p+((p->size)&~1)-FOOTER_SIZE))->head = p;
        next->prior->next=next->next;
        next->next->prior=next->prior;
        --freecount;
    }
    header * front = &heads[getIndex(p->size-SIZE_T_SIZE-FOOTER_SIZE)];
    p->prior=front;
    p->next=front->next;
    front->next->prior = p;
    front->next=p;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t newsize = ALIGN(size + SIZE_T_SIZE+ FOOTER_SIZE);
    header *p = (header *)((char *)ptr - sizeof(size_t));
    header *next=(header *)((char *)p + (p->size &= ~1));
    if((p->size&~1)>newsize){
        return ptr;
    }
    if(((void *)((char *)p+p->size) <= mem_heap_hi()) && !((next->size)&1) && (next->size + (p->size&~1) > newsize)){
        p->size=(p->size+next->size)|1;
        ((footer *)((char *)p+((p->size)&~1)-FOOTER_SIZE))->head = p;
        next->prior->next=next->next;
        next->next->prior=next->prior;
        return ptr;
    }
    if(((void *)((char *)p+p->size) >= mem_heap_hi())){
        mem_sbrk(newsize-p->size);
        p->size=newsize;
        ((footer *)((char *)p+((p->size)&~1)-FOOTER_SIZE))->head = p;
        return ptr;
    }
    void * new = mm_malloc(size);
    memcpy(new,ptr,p->size-SIZE_T_SIZE-FOOTER_SIZE);
    mm_free(ptr);
    return new;
}