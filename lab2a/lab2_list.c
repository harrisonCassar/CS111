//NAME: Harrison Cassar
//EMAIL: Harrison.Cassar@gmail.com
//ID: 505114980

#include "SortedList.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void parseOptions(int argc, char** argv);
void sighandler();
void createThreads(long numOfThreads);
void joinThreads(long numOfThreads);
void* thread_manage_elements();
void logResults(struct timespec* start, struct timespec* end);

#define KEYSIZE 11 //1 extra for null byte

struct option long_options[] =
{
	{"threads", required_argument, 0, 't'},
	{"iterations", required_argument, 0, 'i'},
	{"yield", required_argument, 0, 'y'},
	//{"sync", required_argument, 0, 's'},
	{0, 0, 0, 0} //last element of long_options array must contain all 0's (end)
};

//Globals
long long numThreads = 1; //default 1
long long numIterations = 1; //default 1
SortedList_t head;
char yieldopts[5] = "-";
int opt_yield = 0;
SortedListElement_t** elements; //array of ptrs to list elements
long long numElements;

int main(int argc, char* argv[])
{
	parseOptions(argc,argv);

	createElements();

	//setup necessary structures
	head.next = &head;
	head.prev = &head;
	head.key = NULL;

	//note start time
	struct timespec starttime;
	if (clock_gettime(CLOCK_MONOTONIC,&starttime) == -1)
	{
		fprintf(stderr,"Error: %s; ", strerror(errno));
        switch (errno)
        {
        	case EFAULT:
        		fprintf(stderr,"Attempted recording of start time failed because timespec struct ptr points outside the accessible address space.\n");
        		break;
        	case EINVAL:
        		fprintf(stderr,"Attempted recording of start time failed because specified 'CLOCK_MONOTONIC' is not supported on this system.\n");
            	break;
            default:
            	fprintf(stderr,"Attempted recording of start time failed with an unrecognized error.\n");
            	break;
        }
		
		exit(1);
	}

	//***********************************//

	//create threads
	createThreads(numThreads);
	
	//join/wait for all threads to finish
	joinThreads(numThreads);

	//note end time
	struct timespec endtime;
	if (clock_gettime(CLOCK_MONOTONIC,&endtime) == -1)
	{
		fprintf(stderr,"Error: %s; ", strerror(errno));
        switch (errno)
        {
        	case EFAULT:
        		fprintf(stderr,"Attempted recording of end time failed because timespec struct ptr points outside the accessible address space.\n");
        		break;
        	case EINVAL:
        		fprintf(stderr,"Attempted recording of end time failed because specified 'CLOCK_MONOTONIC' is not supported on this system.\n");
            	break;
            default:
            	fprintf(stderr,"Attempted recording of end time failed with an unrecognized error.\n");
            	break;
        }
		
		exit(1);
	}

	//print to stdout CSV record
	logResults(&starttime,&endtime);

	exit(0);

	//create threads
		//assign them a set of elements to work on (based on iterations)
		//inserts all into shared list
		//gets list length
		//looks up and deletes each keys it had previously inserted
		//exits to rejoin main

	//join threads

	//note end time

	//check length of list to confirm its 0

	//logRecords()
		//name of test
		//numThreads
		//numIterations
		//numLists (1 always in this project)
		//total operations peformed (numThreads*numIterations*3)
		//total runtime for all threads (ns)
		//average time per operation (ns)

	//exit(0)
}

void parseOptions(int argc, char** argv)
{
	int c; //stores returned character from getopt call
	int option_index; //stores index of found option
	char* raw_threads;
	char* raw_iterations;
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
				fprintf(stderr, "Specified option '%s' not recognized.\nusage: %s [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=type]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces yields at critical sections for lookups (l), deletes (d), or inserts (i)\n\t--sync: Specifies type of synchronization method to use ('m' for mutex, 's' for spin-lock, 'c' for compare-and-swap)\n\n", argv[optind-1], argv[0]);
				exit(1);
			case 't':
				raw_threads = optarg;
				numThreads = (int) strtol(raw_threads,NULL,10);
				break;
			case 'i':
				raw_iterations = optarg;
				numIterations = (int) strtol(raw_iterations,NULL,10);
				break;
			case 'y':
				len = strlen(optarg);
				if (len > 3)
				{
					fprintf(stderr, "Specified option argument '%s' not recognized (or is too many characters).\nusage: %s [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=type]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces yields at critical sections for lookups (l), deletes (d), or inserts (i)\n\t--sync: Specifies type of synchronization method to use ('m' for mutex, 's' for spin-lock, 'c' for compare-and-swap)\n\n", optarg, argv[0]);
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
							fprintf(stderr, "Specified option argument '%s' not recognized.\nusage: %s [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=type]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces yields at critical sections for lookups (l), deletes (d), or inserts (i)\n\t--sync: Specifies type of synchronization method to use ('m' for mutex, 's' for spin-lock, 'c' for compare-and-swap)\n\n", optarg, argv[0]);
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

	//check if non-option arguments were specified (not supported)
	if (argc-optind != 0)
	{
		fprintf(stderr, "Specified option '%s' not recognized.\nusage: %s [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=type]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces yields at critical sections for lookups (l), deletes (d), or inserts (i)\n\t--sync: Specifies type of synchronization method to use ('m' for mutex, 's' for spin-lock, 'c' for compare-and-swap)\n\n", argv[optind-1], argv[0]);
		exit(1);
	}
}

void sighandler()
{
	fprintf(stderr,"Received and caught segmentation fault.\n");
	exit(2);
}

void createElements()
{
	numElements = numThreads*numIterations;
	elements = calloc(numElements*sizeof(SortedListElement_t*));
	
	if (elements == NULL)
		exit(2);

	atexit(freeElements);

	for (int i = 0; i < numElements; i++)
	{
		elements[i] = malloc(sizeof(SortedListElement_t));
		if (elements[i] == NULL)
			exit(2);

		char* key = (char *) malloc((KEYSIZE)*sizeof(char))
		if (key == NULL)
			exit(2);

		getRandKey(key);

		elements[i]->key = key;
	}
}

void freeElements()
{
	for (int i = 0; i < numElements; i++)
	{
		if (elements[i] != NULL)
		{
			if (elements[i]->key != NULL)
				free(elements[i]->key);

			free(elements[i]);
		}
	}

	free(elements);
}

void getRandKey(char* buf)
{
	static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	int range = strlen(charset);

	for (int i = 0; i < KEYSIZE-1; i++)
		buf[i] = dict[rand() % range];

	buf[KEYSIZE-1] = '\0';
}

void createThreads(long numOfThreads)
{
	//TODO
}

void joinThreads(long numOfThreads)
{
	//TODO
}

void* thread_manage_elements()
{
	//TODO
	return NULL;
}

void logResults(struct timespec* start, struct timespec* end)
{
	//TODO
}
