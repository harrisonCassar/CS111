# NAME: Harrison Cassar
# EMAIL: Harrison.Cassar@gmail.com
# ID: 505114980

#compiler
CC = gcc

#compiler flags:
# -g    adds debugging information to the executable file
# -Wall turns on most, but not all, compiler warnings
# -Wextra enables some extra warning flags not enabled by -Wall
CFLAGS = -Wall -Wextra
CLIBRARIES = -pthread -lprofiler

# default build
default lab2_list:
	$(CC) $(CFLAGS) $(CLIBRARIES) -o lab2_list SortedList.c lab2_list.c 

graphs: clean tests
	./lab2b_list.gp

profile profile.out: lab2_list
	-rm -f raw.gperf
	LD_PRELOAD=/usr/lib64/libprofiler.so
	CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list raw.gperf >profile.out
	pprof --list=thread_manage_elements ./lab2_list raw.gperf >>profile.out
	-rm -f raw.gperf

# build the distribution tarball
dist: clean graphs profile.out
	tar -zvcf lab2b-505114980.tar.gz lab2b_list.gp profile.out Makefile README lab2_list.c SortedList.h SortedList.c lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png lab2b_list.csv

# delete all files created by the Makefile, and return the directory to its freshly untared state
clean:
	-rm -f *~ *.tar.gz *.o lab2_list

#test targets
tests: default
	-rm -f lab2b_list.csv
	#lab2b_1, 	#lab2b_2
	-./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=1000 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2b_list.csv

	#lab2b_3
	-./lab2_list --threads=1 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=1 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=2 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=4 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=8 --lists=4 --yield=id >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=16 --lists=4 --yield=id >> lab2b_list.csv
	
	-./lab2_list --threads=1 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=10 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=20 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=40 --lists=4 --yield=id --sync=s >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=80 --lists=4 --yield=id --sync=s >> lab2b_list.csv

	-./lab2_list --threads=1 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=10 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=20 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=40 --lists=4 --yield=id --sync=m >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=80 --lists=4 --yield=id --sync=m >> lab2b_list.csv

	#lab2b_4
	#-./lab2_list --threads=1 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	#-./lab2_list --threads=2 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	#-./lab2_list --threads=4 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	#-./lab2_list --threads=8 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	#-./lab2_list --threads=12 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv

	#lab2b_5
	#-./lab2_list --threads=1 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	#-./lab2_list --threads=2 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	#-./lab2_list --threads=4 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	#-./lab2_list --threads=8 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	#-./lab2_list --threads=12 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv

.PHONY: clean dist graphs default tests