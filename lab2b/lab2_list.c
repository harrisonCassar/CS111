//NAME: Harrison Cassar
//EMAIL: Harrison.Cassar@gmail.com
//ID: 505114980

#include "SortedList.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>

void parseOptions(int argc, char** argv);
void sighandler();
void createThreads(long numOfThreads);
void joinThreads(long numOfThreads);
void* thread_manage_elements(void* thread_id);
void getRandKey(char* buf);
void logResults(struct timespec* start, struct timespec* end);
void createElements();
void createLists();
void freeMemory();
void getTag(char* buf);
void protected_clock_gettime(clockid_t clk_id, struct timespec *tp);
long timeDifference(struct timespec* start, struct timespec* end);
int getSublistAssignment(char* key);
void thread_findMultiListLength(long long thread_num);
void unprotected_findMultiListLength();

#define KEYSIZE 11 //1 extra for null byte
#define TAG_MAXSIZE 15
#define SYNC_NONE 0
#define SYNC_MUTEX 1
#define SYNC_SPINLOCK 2

struct option long_options[] =
{
	{"threads", required_argument, 0, 't'},
	{"iterations", required_argument, 0, 'i'},
	{"yield", required_argument, 0, 'y'},
	{"sync", required_argument, 0, 's'},
	{"lists", required_argument, 0, 'l'},
	{0, 0, 0, 0} //last element of long_options array must contain all 0's (end)
};

typedef struct {
	int sublist_id;
	SortedListElement_t* element;
} SublistedElement;

//Globals
long long numThreads = 1; //default 1
long long numIterations = 1; //default 1
int numLists = 1; //default 1
long long numElements;
int lockType = SYNC_NONE;

int opt_yield = 0;
char yieldopts[5] = "-";

SortedList_t** lists = NULL; //array of ptrs to headers of sublists
SublistedElement** elements = NULL; //array of ptrs to "SublistedElement" struct, which contains a ptr to a list element

pthread_t* tid = NULL;
long* timers = NULL;
long long* thread_instance = NULL;
pthread_mutex_t* mutexlocks = NULL;
int* spinlocks = NULL;

int main(int argc, char* argv[])
{
	parseOptions(argc,argv);
	
	//setup necessary structures
	createElements();
	createLists();

	//note start time
	struct timespec starttime;
	protected_clock_gettime(CLOCK_MONOTONIC,&starttime);

	if (signal(SIGSEGV,sighandler) == SIG_ERR)
	{
		fprintf(stderr,"Error while attempting to set signal handler for signal: %d", SIGSEGV);
		exit(1);
	}

	//create threads
	createThreads(numThreads);
	
	//join/wait for all threads to finish
	joinThreads(numThreads);

	//note end time
	struct timespec endtime;
	protected_clock_gettime(CLOCK_MONOTONIC,&endtime);

	//check to make sure multi-list length is 0 (all sublists have 0 length)
	unprotected_findMultiListLength();

	//print to stdout CSV record
	logResults(&starttime,&endtime);

	exit(0);
}

void parseOptions(int argc, char** argv)
{
	int c; //stores returned character from getopt call
	int option_index; //stores index of found option
	char* raw_threads;
	char* raw_iterations;
	char* raw_lists;
	int len;

	opterr = 0;

	for(;;)
	{
		//process next inputted option
		option_index = 0;
		c = getopt_long(argc,argv,"",long_options,&option_index);

		//check for end of options
		if (c == -1)
			break;
		
		switch(c)
		{
			default:
			case '?':
				fprintf(stderr, "Specified option '%s' not recognized.\nusage: %s [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=type]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces yields at critical sections for lookups (l), deletes (d), or inserts (i)\n\t--sync: Specifies type of synchronization method to use ('m' for mutex, 's' for spin-lock)\n\n", argv[optind-1], argv[0]);
				exit(1);
			case 't':
				raw_threads = optarg;
				numThreads = (int) strtol(raw_threads,NULL,10);
				break;
			case 'i':
				raw_iterations = optarg;
				numIterations = (int) strtol(raw_iterations,NULL,10);
				break;
			case 's':
				if (strlen(optarg) != 1 || (optarg[0] != 's' && optarg[0] != 'm'))
				{
					fprintf(stderr, "Specified option argument '%s' not recognized.\nusage: %s [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=type]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces yields at critical sections for lookups (l), deletes (d), or inserts (i)\n\t--sync: Specifies type of synchronization method to use ('m' for mutex, 's' for spin-lock)\n\n", optarg, argv[0]);
					exit(1);
				}
				else if (optarg[0] == 'm')
					lockType = SYNC_MUTEX;
				else if (optarg[0] == 's')
					lockType = SYNC_SPINLOCK;
				break;
			case 'l':
				raw_lists = optarg;
				numLists = (int) strtol(raw_lists,NULL,10);
				break;
			case 'y':
				len = strlen(optarg);
				if (len > 3)
				{
					fprintf(stderr, "Specified option argument '%s' not recognized (or is too many characters).\nusage: %s [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=type]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces yields at critical sections for lookups (l), deletes (d), or inserts (i)\n\t--sync: Specifies type of synchronization method to use ('m' for mutex, 's' for spin-lock)\n\n", optarg, argv[0]);
					exit(1);
				}
				for (int i = 0; i < len; i++)
				{
					switch (optarg[i])
					{
						case 'i':
							opt_yield = opt_yield | INSERT_YIELD;
							strcat(yieldopts,"i");
							break;
						case 'd':
							opt_yield = opt_yield | DELETE_YIELD;
							strcat(yieldopts,"d");
							break;
						case 'l':
							opt_yield = opt_yield | LOOKUP_YIELD;
							strcat(yieldopts,"l");
							break;
						default:
							fprintf(stderr, "Specified option argument '%s' not recognized.\nusage: %s [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=type]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces yields at critical sections for lookups (l), deletes (d), or inserts (i)\n\t--sync: Specifies type of synchronization method to use ('m' for mutex, 's' for spin-lock)\n\n", optarg, argv[0]);
							exit(1);
					}
				}
				break;
		}
	}

	if (numThreads == 0 && strcmp(raw_threads,"0") != 0)
	{
		fprintf(stderr, "Specified # of threads '%s' is inconvertable to a numeric value.\n", raw_threads);
		exit(1);
	}

	if (numThreads < 0)
	{
		fprintf(stderr,"Specified # of threads '%lld' cannot be negative. Please pick a different #.\n", numThreads);
		exit(1);
	}

	if (numIterations == 0 && strcmp(raw_iterations,"0") != 0)
	{
		fprintf(stderr, "Specified # of iterations '%s' is inconvertable to a numeric value.\n", raw_iterations);
		exit(1);
	}

	if (numIterations < 0)
	{
		fprintf(stderr,"Specified # of iterations '%lld' cannot be negative. Please pick a different #.\n", numIterations);
		exit(1);
	}

	if (numLists == 0 && strcmp(raw_lists,"0") != 0)
	{
		fprintf(stderr, "Specified # of lists '%s' is inconvertable to a numeric value.\n", raw_lists);
		exit(1);
	}

	if (numLists < 0)
	{
		fprintf(stderr,"Specified # of lists '%d' cannot be negative. Please pick a different #.\n", numLists);
		exit(1);
	}

	//check if non-option arguments were specified (not supported)
	if (argc-optind != 0)
	{
		fprintf(stderr, "Specified option '%s' not recognized.\nusage: %s [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=type]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces yields at critical sections for lookups (l), deletes (d), or inserts (i)\n\t--sync: Specifies type of synchronization method to use ('m' for mutex, 's' for spin-lock, 'c' for compare-and-swap)\n\n", argv[optind-1], argv[0]);
		exit(1);
	}
}

void sighandler()
{
	fprintf(stderr,"Received and caught SIGSEGV signal (segmentation fault).\n");
	exit(2);
}

void createElements()
{
	numElements = numThreads*numIterations;
	elements = calloc(numElements,sizeof(SublistedElement*));
	
	if (elements == NULL)
		exit(2);

	atexit(freeMemory);

	for (int i = 0; i < numElements; i++)
	{
		elements[i] = malloc(sizeof(SublistedElement));
		if (elements[i] == NULL)
			exit(2);

		elements[i]->element = malloc(sizeof(SortedListElement_t));
		if (elements[i]->element == NULL)
			exit(2);

		char* key = (char *) malloc((KEYSIZE)*sizeof(char));
		if (key == NULL)
			exit(2);

		getRandKey(key);
		elements[i]->element->key = key;

		elements[i]->sublist_id = getSublistAssignment(key);
	}

	timers = calloc(numThreads,sizeof(long));

	if (timers == NULL)
		exit(2);
}

void createLists()
{
	lists = calloc(numLists,sizeof(SortedList_t*));

	if (lists == NULL)
		exit(2);

	if (lockType == SYNC_MUTEX)
	{
		mutexlocks = malloc(numLists*sizeof(pthread_mutex_t));
		if (mutexlocks == NULL)
			exit(2);

		for (int i = 0; i < numLists; i++)
			pthread_mutex_init(&(mutexlocks[i]),NULL);
	}
	else if (lockType == SYNC_SPINLOCK)
	{
		spinlocks = calloc(numLists,sizeof(int));
		if (spinlocks == NULL)
			exit(2);
	}

	for (int i = 0; i < numLists; i++)
	{
		lists[i] = malloc(sizeof(SortedList_t));
		if (lists[i] == NULL)
			exit(2);

		lists[i]->next = lists[i];
		lists[i]->prev = lists[i];
		lists[i]->key = NULL;
	}
}

void freeMemory()
{
	//free element and list header pools
	for (int i = 0; i < numElements; i++)
	{
		if (elements[i] != NULL)
		{
			if (elements[i]->element != NULL)
			{
				if (elements[i]->element->key != NULL)
					free((char *) elements[i]->element->key);

				free(elements[i]->element);
			}

			free(elements[i]);
		}
	}

	if (elements != NULL)
		free(elements);

	for (int i = 0; i < numLists; i++)
	{
		if (lists[i] != NULL)
			free(lists[i]);
	}

	if (lists != NULL)
		free(lists);	

	//free locks associated with sublists
	if (mutexlocks != NULL)
		free(mutexlocks);

	if (spinlocks != NULL)
		free(spinlocks);

	//free thread-related memory
	if (timers != NULL)
		free(timers); 

	if (tid != NULL)
		free(tid);

	if (thread_instance != NULL)
		free(thread_instance);
}

void getRandKey(char* buf)
{
	static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	int range = strlen(charset);

	for (int i = 0; i < KEYSIZE-1; i++)
		buf[i] = charset[rand() % range];

	buf[KEYSIZE-1] = '\0';
}

void createThreads(long numOfThreads)
{
	//create threads
	tid = malloc(numOfThreads*sizeof(pthread_t));

	if (tid == NULL)
	{
		fprintf(stderr, "Error with memory allocation for thread IDs\n");
		exit(2);
	}

	thread_instance = (long long*) malloc(numOfThreads*sizeof(long long));
	if (thread_instance == NULL)
	{
		fprintf(stderr, "Error with memory allocation for thread IDs\n");
		exit(2);
	}

	for (int i = 0; i < numOfThreads; i++)
	{
		thread_instance[i] = (long long) i+1;
		int retval = pthread_create(&tid[i],NULL,thread_manage_elements,(void *) &thread_instance[i]);
		if (retval != 0)
		{
			fprintf(stderr, "Error with creating thread %d: ", i+1);
			switch (retval)
			{
				case EAGAIN:
					fprintf(stderr,"System lacked necessary resources to create another thread, or the system-imposed limit on the total number of threads in a process would be exceeded.\n");
					break;
				case EPERM:
					fprintf(stderr,"Caller does not have appropiate permission to set the required scheduling parameters or scheduling policy.\n");
					break;
				default:
					fprintf(stderr,"Failed with an unrecognized error.\n");
					break;
			}
			
			exit(1);
		}
	}
}

void joinThreads(long numOfThreads)
{
	for (int i = 0; i < numOfThreads; i++)
	{
		int retval = pthread_join(tid[i], NULL);
		if (retval != 0)
		{
			fprintf(stderr, "Error with joining thread %d: \n", i+1);
			switch (retval)
			{
				case EDEADLK:
					fprintf(stderr, "A deadlock was detected or indicated thread specifies the calling thread.\n");
					break;
				case EINVAL:
					fprintf(stderr, "The implementation has detected that the indicated thread does not refer to a joinable thread.\n");
					break;
				default:
					fprintf(stderr, "Failed with an unrecognized error.\n");
			}

			exit(1);
		}
	}
}

void* thread_manage_elements(void* thread_id)
{
	struct timespec starttime;
	struct timespec acquiretime;

	long long* thread_num = (long long*) thread_id;

	long long set_begin = (*thread_num - 1)*numIterations;
	long long set_end = set_begin + numIterations - 1;

	int sublistID;

	//insert all allocated elements
	for (long long i = set_begin; i <= set_end; i++)
	{
		sublistID = elements[i]->sublist_id;
		switch (lockType)
		{
			default:
			case SYNC_NONE:
				SortedList_insert(lists[sublistID],elements[i]->element);
				break;
			case SYNC_MUTEX:
				protected_clock_gettime(CLOCK_MONOTONIC,&starttime);
				
				if (pthread_mutex_lock(&(mutexlocks[sublistID])) != 0)
				{
					fprintf(stderr,"Error while attempting to lock mutex.\n");
					exit(1);
				}

				protected_clock_gettime(CLOCK_MONOTONIC,&acquiretime);
				timers[(int) *thread_num] += timeDifference(&starttime,&acquiretime);

				SortedList_insert(lists[sublistID],elements[i]->element);

				if (pthread_mutex_unlock(&(mutexlocks[sublistID])) != 0)
				{
					fprintf(stderr,"Error while attempting to unlock mutex.\n");
					exit(1);
				}

				break;
			case SYNC_SPINLOCK:
				protected_clock_gettime(CLOCK_MONOTONIC,&starttime);

				//wait until spinlock is 0 (unlocked) before moving on
				while (__sync_lock_test_and_set(&(spinlocks[sublistID]),1));

				protected_clock_gettime(CLOCK_MONOTONIC,&acquiretime);
				timers[(int) *thread_num] += timeDifference(&starttime,&acquiretime);
				
				SortedList_insert(lists[sublistID],elements[i]->element);

				__sync_lock_release(&(spinlocks[sublistID]));
				break;
		}
	}

	//find length of entire multi-list
	thread_findMultiListLength(*thread_num);

	//delete all allocated elements
	SortedListElement_t* toDelete;

	for (long long i = set_begin; i <= set_end; i++)
	{
		sublistID = elements[i]->sublist_id;
		switch (lockType)
		{
			default:
			case SYNC_NONE:
				toDelete = SortedList_lookup(lists[sublistID],elements[i]->element->key);

				if (toDelete == NULL)
				{
					fprintf(stderr, "Unexpected error: Lookup of previously added element to list has failed (non-existant).\n");
					exit(2);
				}

				if (SortedList_delete(toDelete) == 1)
				{
					fprintf(stderr, "Unexpected error: Corrupted list found when attempting to delete element.\n");
					exit(2);
				}

				break;
			case SYNC_MUTEX:
				protected_clock_gettime(CLOCK_MONOTONIC,&starttime);

				if (pthread_mutex_lock(&(mutexlocks[sublistID])) != 0)
				{
					fprintf(stderr,"Error while attempting to lock mutex.\n");
					exit(1);
				}

				protected_clock_gettime(CLOCK_MONOTONIC,&acquiretime);
				timers[(int) *thread_num] += timeDifference(&starttime,&acquiretime);

				toDelete = SortedList_lookup(lists[sublistID],elements[i]->element->key);

				if (toDelete == NULL)
				{
					fprintf(stderr, "Unexpected error: Lookup of previously added element to list has failed (non-existant).\n");
					exit(2);
				}

				if (SortedList_delete(toDelete) == 1)
				{
					fprintf(stderr, "Unexpected error: Corrupted list found when attempting to delete element.\n");
					exit(2);
				}

				if (pthread_mutex_unlock(&(mutexlocks[sublistID])) != 0)
				{
					fprintf(stderr,"Error while attempting to unlock mutex.\n");
					exit(1);
				}

				break;
			case SYNC_SPINLOCK:
				protected_clock_gettime(CLOCK_MONOTONIC,&starttime);

				//wait until spinlock is 0 (unlocked) before moving on
				while (__sync_lock_test_and_set(&(spinlocks[sublistID]),1));
				
				protected_clock_gettime(CLOCK_MONOTONIC,&acquiretime);
				timers[(int) *thread_num] += timeDifference(&starttime,&acquiretime);

				toDelete = SortedList_lookup(lists[sublistID],elements[i]->element->key);

				if (toDelete == NULL)
				{
					fprintf(stderr, "Unexpected error: Lookup of previously added element to list has failed (non-existant).\n");
					exit(2);
				}

				if (SortedList_delete(toDelete) == 1)
				{
					fprintf(stderr, "Unexpected error: Corrupted list found when attempting to delete element.\n");
					exit(2);
				}

				__sync_lock_release(&(spinlocks[sublistID]));
				break;
		}
	}

	return NULL;
}

void logResults(struct timespec* start, struct timespec* end)
{
	char tag[TAG_MAXSIZE];
	getTag(tag);
	
	long numOperations = numThreads*numIterations*3;

	long runTime = timeDifference(start,end);

	long avgTimePerOperation = runTime / numOperations;

	long throughput = 1000000000/avgTimePerOperation;

	long avgWaitForLock = 0;
	if (lockType != SYNC_NONE)
	{
		for (int i=0; i < numThreads; i++)
			avgWaitForLock += timers[i];

		avgWaitForLock /= numOperations;
	}

	fprintf(stdout,"%s,%lld,%lld,%d,%ld,%ld,%ld,%ld,%ld\n",tag,numThreads,numIterations,numLists,numOperations,runTime,avgTimePerOperation,avgWaitForLock,throughput);
}

long timeDifference(struct timespec* start, struct timespec* end)
{
	time_t dsec = end->tv_sec - start->tv_sec;
	long dnsec = end->tv_nsec - start->tv_nsec;
	return dsec*1000000000 + dnsec;
}

void getTag(char* buf)
{
	strcpy(buf,"list-");
	
	if (opt_yield != 0)
	{
		if (opt_yield & INSERT_YIELD)
			strcat(buf,"i");
		if (opt_yield & DELETE_YIELD)
			strcat(buf,"d");
		if (opt_yield & LOOKUP_YIELD)
			strcat(buf,"l");

		strcat(buf,"-");
	}
	else
		strcat(buf,"none-");

	//add sync ops to buf
	switch (lockType)
	{
		default:
		case SYNC_NONE:
			strcat(buf,"none");
			break;
		case SYNC_MUTEX:
			strcat(buf,"m");
			break;
		case SYNC_SPINLOCK:
			strcat(buf,"s");
			break;
	}
}

void protected_clock_gettime(clockid_t clk_id, struct timespec *tp)
{
	if (clock_gettime(clk_id,tp) == -1)
	{
		fprintf(stderr,"Error: %s; ", strerror(errno));
        switch (errno)
        {
        	case EFAULT:
        		fprintf(stderr,"Attempted recording of time failed because timespec struct ptr points outside the accessible address space.\n");
        		break;
        	case EINVAL:
        		fprintf(stderr,"Attempted recording of time failed because specified clock is not supported on this system.\n");
            	break;
            default:
            	fprintf(stderr,"Attempted recording of time failed with an unrecognized error.\n");
            	break;
        }
		
		exit(1);
	}
}

int getSublistAssignment(char* key)
{
	int keyval = 0;

	for (int i = 0; i < KEYSIZE; i++)
		keyval += (int) key[i];

	return keyval % numLists;
}

void thread_findMultiListLength(long long thread_num)
{
	struct timespec starttime;
	struct timespec acquiretime;
 
	for (int i = 0; i < numLists; i++)
	{
		//obtain lock for certain list
		switch (lockType)
		{
			default:
			case SYNC_NONE:
				if (SortedList_length(lists[i]) == -1)
				{
					fprintf(stderr, "Unexpected error: List found to be corrupted upon requesting its length.\n");
					exit(2);
				}
				
				break;
			case SYNC_MUTEX:
				protected_clock_gettime(CLOCK_MONOTONIC,&starttime);

				if (pthread_mutex_lock(&(mutexlocks[i])) != 0)
				{
					fprintf(stderr,"Error while attempting to lock mutex.\n");
					exit(1);
				}

				protected_clock_gettime(CLOCK_MONOTONIC,&acquiretime);
				timers[(int) thread_num] += timeDifference(&starttime,&acquiretime);

				if (SortedList_length(lists[i]) == -1)
				{
					fprintf(stderr, "Unexpected error: List found to be corrupted upon requesting its length.\n");
					exit(2);
				}

				if (pthread_mutex_unlock(&(mutexlocks[i])) != 0)
				{
					fprintf(stderr,"Error while attempting to unlock mutex.\n");
					exit(1);
				}

				break;
			case SYNC_SPINLOCK:
				protected_clock_gettime(CLOCK_MONOTONIC,&starttime);

				//wait until spinlock is 0 (unlocked) before moving on
				while (__sync_lock_test_and_set(&(spinlocks[i]),1));
				
				protected_clock_gettime(CLOCK_MONOTONIC,&acquiretime);
				timers[(int) thread_num] += timeDifference(&starttime,&acquiretime);

				if (SortedList_length(lists[i]) == -1)
				{
					fprintf(stderr, "Unexpected error: List found to be corrupted upon requesting its length.\n");
					exit(2);
				}

				__sync_lock_release(&(spinlocks[i]));
				break;
		}
	}
}

void unprotected_findMultiListLength()
{
	for (int i = 0; i < numLists; i++)
	{
		int length = SortedList_length(lists[i]);
		if (length == -1)
		{
			fprintf(stderr, "Unexpected error: List found to be corrupted upon requesting its length.\n");
			exit(2);
		}

		if (length != 0)
		{
			fprintf(stderr, "Error in main thread: Length of a sublist is not 0 as expected. Length is: %d\n", length);
			exit(2);
		}
	}
}