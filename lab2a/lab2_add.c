//NAME: Harrison Cassar
//EMAIL: Harrison.Cassar@gmail.com
//ID: 505114980

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>

void parseOptions(int argc, char** argv);
void add(long long *pointer, long long value);
void cas_add(long long *pointer, long long value);
void createThreads(long numOfThreads);
void joinThreads(long numOfThreads);
void* thread_add_and_subtract();
void logResults(struct timespec* start, struct timespec* end);
void getTag(char* buf);
void freeMem();

struct option long_options[] =
{
	{"threads", required_argument, 0, 't'},
	{"iterations", required_argument, 0, 'i'},
	{"yield", no_argument, 0, 'y'},
	{"sync", required_argument, 0, 's'},
	{0, 0, 0, 0} //last element of long_options array must contain all 0's (end)
};

//Global Constants
#define SYNC_NONE 0
#define SYNC_MUTEX 1
#define SYNC_SPINLOCK 2
#define SYNC_CAS 3
#define TAG_MAXSIZE 15

//Globals
long long numThreads = 1; //default 1
long long numIterations = 1; //default 1
int FL_yield = 0;
int lockType = SYNC_NONE; //default 0 for no synchronization
pthread_t* tid;
pthread_mutex_t mutexlock;

long long counter = 0;
int spinlock = 0;

int main(int argc, char* argv[])
{
	parseOptions(argc,argv);
	
	//setup necessary structures
	if (lockType == SYNC_MUTEX)
		pthread_mutex_init(&mutexlock,NULL);

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
}

void add(long long *pointer, long long value)
{
	long long sum = *pointer + value;
	if (FL_yield)
		sched_yield();
	*pointer = sum;
}

void cas_add(long long *pointer, long long value)
{
	long long oldval;
	long long newval;

	for (;;)
	{
		oldval = *pointer;
		newval = oldval + value;
		
		if (FL_yield)
			sched_yield();

		//check if value still the same, and swap in newval if so (otherwise, repeat access & sum)
		if (__sync_bool_compare_and_swap(pointer,oldval,newval))
			break;
	}
}

void* thread_add_and_subtract()
{
	switch (lockType)
	{
		default:
		case SYNC_NONE:
			for (int i = 0; i < numIterations; i++)
				add(&counter,1);
			for (int i = 0; i < numIterations; i++)
				add(&counter,-1);
			break;
		case SYNC_MUTEX:
			for (int i = 0; i < numIterations; i++)
			{
				pthread_mutex_lock(&mutexlock);
				add(&counter,1);
				pthread_mutex_unlock(&mutexlock);
			}
			for (int i = 0; i < numIterations; i++)
			{
				pthread_mutex_lock(&mutexlock);
				add(&counter,-1);
				pthread_mutex_unlock(&mutexlock);
			}
			break;
		case SYNC_SPINLOCK:
			for (int i = 0; i < numIterations; i++)
			{
				//wait until spinlock is 0 (unlocked) before moving on
				while (__sync_lock_test_and_set(&spinlock,1));
				
				add(&counter,1);

				__sync_lock_release(&spinlock);
			}
			for (int i = 0; i < numIterations; i++)
			{
				//wait until spinlock is 0 (unlocked) before moving on
				while (__sync_lock_test_and_set(&spinlock,1));
				
				add(&counter,-1);

				__sync_lock_release(&spinlock);
			}
			break;
		case SYNC_CAS:
			for (int i = 0; i < numIterations; i++)
				cas_add(&counter,1);
			for (int i = 0; i < numIterations; i++)
				cas_add(&counter,-1);
			break;
	}

	return NULL;
}

void parseOptions(int argc, char** argv)
{
	int c; //stores returned character from getopt call
	int option_index; //stores index of found option
	char* raw_threads;
	char* raw_iterations;

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
				fprintf(stderr, "Specified option '%s' not recognized.\nusage: %s [--threads=#] [--iterations=#] [--yield] [--sync=type]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces threads to yield in-between shared variable access and assignment after modification\n\t--sync: Specifies type of synchronization method to use ('m' for mutex, 's' for spin-lock, 'c' for compare-and-swap)\n\n", argv[optind-1], argv[0]);
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
				FL_yield = 1;
				break;
			case 's':
				if (strlen(optarg) != 1 || (optarg[0] != 'c' && optarg[0] != 's' && optarg[0] != 'm'))
				{
					fprintf(stderr, "Specified option argument '%s' not recognized.\nusage: %s [--threads=#] [--iterations=#] [--yield] [--sync=type]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces threads to yield in-between shared variable access and assignment after modification\n\t--sync: Specifies type of synchronization method to use ('m' for mutex, 's' for spin-lock, 'c' for compare-and-swap)\n\n", optarg, argv[0]);
					exit(1);
				}
				else if (optarg[0] == 'c')
					lockType = SYNC_CAS;
				else if (optarg[0] == 'm')
					lockType = SYNC_MUTEX;
				else if (optarg[0] == 's')
					lockType = SYNC_SPINLOCK;
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
		fprintf(stderr, "Specified option '%s' not recognized.\nusage: %s [--threads=#] [--iterations=#] [--yield] [--sync=type]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces threads to yield in-between shared variable access and assignment after modification\n\t--sync: Specifies type of synchronization method to use ('m' for mutex, 's' for spin-lock, 'c' for compare-and-swap)\n\n", argv[optind-1], argv[0]);
		exit(1);
	}
}

void createThreads(long numOfThreads)
{
	//create threads
	tid = malloc(numOfThreads*sizeof(pthread_t));

	if (tid == NULL)
	{
		fprintf(stderr, "Error with memory allocation for thread IDs\n");
		exit(1);
	}

    atexit(freeMem);

	for (int i = 0; i < numOfThreads; i++)
	{
		int retval = pthread_create(&tid[i],NULL,thread_add_and_subtract,NULL);
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
			
			exit(2);
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

			exit(2);
		}
	}
}

void logResults(struct timespec* start, struct timespec* end)
{
	char tag[TAG_MAXSIZE];
	getTag(tag);
	
	long numOperations = numThreads*numIterations*2;

	time_t dsec = end->tv_sec - start->tv_sec;
	long dnsec = end->tv_nsec - start->tv_nsec;
	long runTime = dsec*1000000000 + dnsec;

	long avgTimePerOperation = runTime / numOperations;

	fprintf(stdout,"%s,%lld,%lld,%ld,%ld,%ld,%lld\n",tag,numThreads,numIterations,numOperations,runTime,avgTimePerOperation,counter);
}

void getTag(char* buf)
{
	strcpy(buf,"add");
	
	if (FL_yield == 1)
		strcat(buf,"-yield");

	switch (lockType)
	{
		default:
		case SYNC_NONE:
			strcat(buf,"-none");
			break;
		case SYNC_MUTEX:
			strcat(buf,"-m");
			break;
		case SYNC_SPINLOCK:
			strcat(buf,"-s");
			break;
		case SYNC_CAS:
			strcat(buf,"-c");
			break;
	}
}

void freeMem()
{
	free(tid);
}