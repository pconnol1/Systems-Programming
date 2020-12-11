#include "cachelab.h"
#include <getopt.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <limits.h>


int readAndEval();
//int dataLoad();
//int dataStore();
//int dataModify();
void printUsage();

//typedefs for the cache

typedef struct {
	int s; //2^s cache sets
	int b; //cacheline block size 2^b bytes
	int E; // num lines per set
	int S; //num sets  S=2^s
	int B; //block size B=2^b bytes
}parameters_t;

//struct for 1 line of the cache to be used in an array to create the cache.
typedef struct {
	int valid;
	unsigned long long tag;
	int count;
}line_t;
//cache set made out of an array of line pointers
typedef struct{
	line_t *lines;
}cache_set;

//cache made up of all the cache sets
typedef struct {
	cache_set *sets;
}cache_t;


//global variables
int verbose;

int main(int argc, char **argv)
{
	//Counters for the print summary at the end of main.
	int hit_count=0;
	int miss_count=0;
	int eviction_count=0;
    
	char *tracefile;
	parameters_t par;
	//parses command line args
	char c;
	while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
		switch(c){
			case 's':
				par.s= atoi(optarg); 
				break;
			case 'E':
				par.E = atoi(optarg); 
				break;
			case 'b':
				par.b = atoi(optarg);
				break;
			case 't':
				tracefile= optarg;
				break;
			case 'v':
				verbose=1;
				break;
			case 'h':
				printUsage();
				exit(0);
			default:
				printUsage();
				exit(1);
		//could be an error in what was entered by the user. if any parameters = 0 then print the help statement
		if(par.s==0 || par.E == 0 || par.b==0 || tracefile==NULL){
			printf("Missing command line args\n");
			exit(1); // messsed up exit program
		}
		//determine set size & block size based off of b and s.
			par.S = (1<<par.s); //2^s
			par.B = (1<<par.b); //2^b
		}
		
		
    }
	
	/*TO DO*/
	/*Set up cache with given parameters and allocate memory for it*/
	cache_t cache;
	cache.sets=malloc(sizeof(cache_set)*par.S);
	for(int i=0; i<par.S; i++)
	{
		cache.sets[i].lines=malloc(sizeof(line_t)*par.E);
	}
	
	/*TO DO*/
	/*Read in the file and evaluate line by line*/
	char op;		//operation (L,S,M)
	unsigned long long addr;//address
	int size;		//size read from file
	int time_stamp=0;//LRU value
	int empty = -1;
	int H= 0;		//boolean for Hit
	int E= 0;		//boolean for Eviction
	
	FILE *traceFile = fopen(tracefile, "r");
	while ( fscanf(traceFile, " %c %llx,%d", &op, &addr, &size)==3)
	{
		int toEvict=0;
		if(op!='I')
		{
			//tag bits= address bit length(64)- exponent of index - exponent of offset
			int tagsize = (64 - (par.s+par.b));
			unsigned long long tag= addr>>(par.s+par.b);
			unsigned long long temp=addr<< (tagsize);
			unsigned long long setid=temp>>(tagsize+par.b);
			cache_set set = cache.sets[setid];
			int low = INT_MAX;
			for ( int e = 0; e < par.E; e++ ) {
               if ( set.lines[e].valid == 1 ) {
                  //look for hit before eviction
                  if ( set.lines[e].tag == tag ) {
                     hit_count++;
                     H = 1;
                     set.lines[e].count = time_stamp;
                     time_stamp++;
                  }
				  
                  // Look for oldest eviction
                  else if ( set.lines[e].count < low) {
                     low = set.lines[e].count;
                     toEvict = e;
                  }
               }
			   else if( empty == -1 ) {
                  empty = e;
               }
			}
			
			//if miss
            if ( H != 1 )
            {
               miss_count++;
               //if we have an empty line
               if ( empty > -1 )
               {
                  set.lines[empty].valid = 1;
                  set.lines[empty].tag = tag;
                  set.lines[empty].count = time_stamp;
                  time_stamp++; 
               }
               //if the set is full evict
               else if ( empty < 0 )
               {
                  E = 1;
                  set.lines[toEvict].tag = tag;
                  set.lines[toEvict].count = time_stamp;
                  time_stamp++; 
                  eviction_count++;
               }
            }
			//always get a hit with modify
			 if ( op == 'M' )
			{
				   hit_count++;
			}
			if ( verbose == 1 )
			{
				printf( "%c ", op );
				printf( "%llx,%d", addr, size );
				if ( H == 1 )
				{
					printf( "Hit " );
				}
				else if ( H != 1 )
				{
					printf( "Miss " );
				}
				if ( E == 1 )
				{
					printf( "Eviction " );
				}
				printf( "\n" );
				}
			empty = -1;
			H = 0;
			E = 0;
		}
	}
	
	
	//print the hits, misses, and evictions
	printSummary(hit_count, miss_count, eviction_count);
    return 0;
}

//Functions

//print user usage info
void printUsage()
{
	printf("Usage: ./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
	printf("-h: Optional help flag that prints usage info\n");
	printf("-v: Optional verbose flag that displays trace info\n");
	printf("-s <s>: Number of set index bits (S = 2s is the number of sets)\n");
	printf("E <E>: Associativity (number of lines per set\n");
	printf("-b <b>: Number of block bits (B = 2b is the block size)\n");
	printf("-t <tracefile>: Name of the valgrind trace to replay\n");
	exit(0); //terminates program successfuly
}


/*int dataLoad(cache_t cache, long long address)
{
	
}
int dataStore(cache_t cache, long long address)
{
	
}
int dataModify(cache_t cache, long long address)
{
	
}
*/
