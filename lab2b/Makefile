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
CLIBRARIES = -pthread

# default build
default:
	$(CC) $(CFLAGS) $(CLIBRARIES) -o lab2_list SortedList.c lab2_list.c 

graphs: clean tests
	./lab2b_list.gp

profile:


# build the distribution tarball
dist: clean graphs
	tar -zvcf lab2b-505114980.tar.gz lab2b_list.gp Makefile README lab2_list.c SortedList.h SortedList.c lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png lab2b_list.csv

# delete all files created by the Makefile, and return the directory to its freshly untared state
clean:
	-rm -f *~ *.tar.gz *.o lab2_list

#test targets
tests: default
	-rm -f lab2_list.csv
	#lab2b_list-1
	-./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2_list.csv
	-./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2_list.csv
	-./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2_list.csv
	-./lab2_list --threads=1 --iterations=1000 --sync=s >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1000 --sync=s >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2_list.csv
	-./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2_list.csv
	-./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2_list.csv
	
.PHONY: clean dist graphs default tests