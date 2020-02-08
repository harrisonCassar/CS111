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
void createThreads(long numOfThreads);
void joinThreads(long numOfThreads);
void* thread_add_and_subtract();
void logResults(struct timespec* start, struct timespec* end);
void freeMem();

struct option long_options[] =
{
	{"threads", required_argument, 0, 't'},
	{"iterations", required_argument, 0, 'i'},
	{"yield", no_argument, 0, 'y'},
	{0, 0, 0, 0} //last element of long_options array must contain all 0's (end)
};

//Globals
long long numThreads = 1; //default 1
long long numIterations = 1; //default 1
int FL_yield = 0;
pthread_t* tid;

long long counter = 0;

int main(int argc, char* argv[])
{
	parseOptions(argc,argv);
	
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

void* thread_add_and_subtract()
{
	for (int i = 0; i < numIterations; i++)
	{
		add(&counter,1);
		add(&counter,-1);
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
				fprintf(stderr, "Specified option '%s' not recognized.\nusage: %s [--threads=#] [--iterations=#] [--yield]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces threads to yield in-between shared variable access and assignment after modification\n\n", argv[optind-1], argv[0]);
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
		fprintf(stderr, "Specified option '%s' not recognized.\nusage: %s [--threads=#] [--iterations=#] [--yield]\n\t--threads: indicate number of parallel threads to run\n\t--iterations: indicate number of iterations for parallel thread(s) to run\n\t--yield: forces threads to yield in-between shared variable access and assignment after modification\n\n", argv[optind-1], argv[0]);
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
	char addnone[9] = "add-none\0";
	
	long numOperations = numThreads*numIterations*2;

	time_t dsec = end->tv_sec - start->tv_sec;
	long dnsec = end->tv_nsec - start->tv_nsec;
	long runTime = dsec*1000000000 + dnsec;

	long avgTimePerOperation = runTime / numOperations;

	fprintf(stdout,"%s,%lld,%lld,%ld,%ld,%ld,%lld\n",addnone,numThreads,numIterations,numOperations,runTime,avgTimePerOperation,counter);
}

void freeMem()
{
	free(tid);
}