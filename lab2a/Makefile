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
build: lab2_add.c lab2_list.c SortedList.c
	#$(CC) $(CFLAGS) $(CLIBRARIES) -o lab2_add lab2_add.c
	$(CC) $(CFLAGS) $(CLIBRARIES) -o lab2_list SortedList.c lab2_list.c 

build-add: lab2_add.c
	$(CC) $(CFLAGS) $(CLIBRARIES) -o lab2_add lab2_add.c

tests: tests-add tests-list

tests-list:
	-rm -f lab2_list.csv
	-./lab2_list --threads=1 --iterations=10 >> lab2_list.csv
	-./lab2_list --threads=1 --iterations=100 >> lab2_list.csv
	-./lab2_list --threads=1 --iterations=1000 >> lab2_list.csv
	-./lab2_list --threads=1 --iterations=10000 >> lab2_list.csv
	-./lab2_list --threads=1 --iterations=20000 >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=1 >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=10 >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=100 >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1000 >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=1 >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=10 >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=100 >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1000 >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=1 >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=10 >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=100 >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1000 >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=1 >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=10 >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=100 >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1000 >> lab2_list.csv

	#yield=i
	-./lab2_list --threads=2 --iterations=1 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1 --yield=i >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=1 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=i >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=1 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=i >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=1 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=i >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=2 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=2 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=2 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=2 --yield=i >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=2 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=i >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=2 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=i >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=2 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=i >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=4 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=4 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=4 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=4 --yield=i >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=4 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=i >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=4 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=i >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=4 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=i >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=8 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=8 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=8 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=8 --yield=i >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=8 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=i >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=8 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=i >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=8 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=i >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=16 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=16 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=16 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=16 --yield=i >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=16 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=i >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=16 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=i >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=16 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=i >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=32 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=32 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=32 --yield=i >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=32 --yield=i >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=32 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=32 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=32 --yield=i >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=32 --yield=i >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=32 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=32 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=32 --yield=i >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=32 --yield=i >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=32 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=32 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=32 --yield=i >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=32 --yield=i >> lab2_list.csv

	#yield=d
	-./lab2_list --threads=2 --iterations=1 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1 --yield=d >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=1 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=d >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=1 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=d >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=1 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=d >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=2 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=2 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=2 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=2 --yield=d >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=2 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=d >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=2 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=d >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=2 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=d >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=4 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=4 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=4 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=4 --yield=d >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=4 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=d >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=4 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=d >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=4 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=d >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=8 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=8 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=8 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=8 --yield=d >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=8 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=d >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=8 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=d >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=8 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=d >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=16 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=16 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=16 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=16 --yield=d >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=16 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=d >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=16 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=d >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=16 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=d >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=32 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=32 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=32 --yield=d >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=32 --yield=d >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=32 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=32 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=32 --yield=d >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=32 --yield=d >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=32 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=32 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=32 --yield=d >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=32 --yield=d >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=32 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=32 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=32 --yield=d >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=32 --yield=d >> lab2_list.csv

	#yield=il
	-./lab2_list --threads=2 --iterations=1 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1 --yield=il >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=1 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=il >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=1 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=il >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=1 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=il >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=2 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=2 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=2 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=2 --yield=il >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=2 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=il >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=2 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=il >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=2 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=il >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=4 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=4 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=4 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=4 --yield=il >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=4 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=il >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=4 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=il >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=4 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=il >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=8 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=8 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=8 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=8 --yield=il >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=8 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=il >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=8 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=il >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=8 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=il >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=16 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=16 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=16 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=16 --yield=il >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=16 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=il >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=16 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=il >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=16 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=il >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=32 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=32 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=32 --yield=il >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=32 --yield=il >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=32 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=32 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=32 --yield=il >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=32 --yield=il >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=32 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=32 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=32 --yield=il >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=32 --yield=il >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=32 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=32 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=32 --yield=il >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=32 --yield=il >> lab2_list.csv

	#yield=dl
	-./lab2_list --threads=2 --iterations=1 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=1 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=1 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=1 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=1 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=2 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=2 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=2 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=2 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=2 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=2 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=2 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=4 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=4 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=4 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=4 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=4 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=4 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=4 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=8 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=8 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=8 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=8 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=8 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=8 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=8 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=16 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=16 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=16 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=16 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=16 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=16 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=16 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=2 --iterations=32 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=32 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=32 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=2 --iterations=32 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=4 --iterations=32 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=32 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=32 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=4 --iterations=32 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=8 --iterations=32 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=32 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=32 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=8 --iterations=32 --yield=dl >> lab2_list.csv

	-./lab2_list --threads=12 --iterations=32 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=32 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=32 --yield=dl >> lab2_list.csv
	-./lab2_list --threads=12 --iterations=32 --yield=dl >> lab2_list.csv

tests-add:
	-rm -f lab2_add.csv

	./lab2_add --threads=2 --iterations=100 >> lab2_add.csv
	./lab2_add --threads=4 --iterations=100 >> lab2_add.csv
	./lab2_add --threads=8 --iterations=100 >> lab2_add.csv
	./lab2_add --threads=12 --iterations=100 >> lab2_add.csv

	./lab2_add --threads=2 --iterations=1000 >> lab2_add.csv
	./lab2_add --threads=4 --iterations=1000 >> lab2_add.csv
	./lab2_add --threads=8 --iterations=1000 >> lab2_add.csv
	./lab2_add --threads=12 --iterations=1000 >> lab2_add.csv

	./lab2_add --threads=2 --iterations=10000 >> lab2_add.csv
	./lab2_add --threads=4 --iterations=10000 >> lab2_add.csv
	./lab2_add --threads=8 --iterations=10000 >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000 >> lab2_add.csv

	./lab2_add --threads=2 --iterations=100000 >> lab2_add.csv
	./lab2_add --threads=4 --iterations=100000 >> lab2_add.csv
	./lab2_add --threads=8 --iterations=100000 >> lab2_add.csv
	./lab2_add --threads=12 --iterations=100000 >> lab2_add.csv

	./lab2_add --threads=2 --iterations=10 --yield >> lab2_add.csv
	./lab2_add --threads=4 --iterations=10 --yield >> lab2_add.csv
	./lab2_add --threads=8 --iterations=10 --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10 --yield >> lab2_add.csv

	./lab2_add --threads=2 --iterations=20 --yield >> lab2_add.csv
	./lab2_add --threads=4 --iterations=20 --yield >> lab2_add.csv
	./lab2_add --threads=8 --iterations=20 --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=20 --yield >> lab2_add.csv

	./lab2_add --threads=2 --iterations=40 --yield >> lab2_add.csv
	./lab2_add --threads=4 --iterations=40 --yield >> lab2_add.csv
	./lab2_add --threads=8 --iterations=40 --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=40 --yield >> lab2_add.csv

	./lab2_add --threads=2 --iterations=80 --yield >> lab2_add.csv
	./lab2_add --threads=4 --iterations=80 --yield >> lab2_add.csv
	./lab2_add --threads=8 --iterations=80 --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=80 --yield >> lab2_add.csv

	./lab2_add --threads=2 --iterations=100 --yield >> lab2_add.csv
	./lab2_add --threads=4 --iterations=100 --yield >> lab2_add.csv
	./lab2_add --threads=8 --iterations=100 --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=100 --yield >> lab2_add.csv

	./lab2_add --threads=2 --iterations=1000 --yield >> lab2_add.csv
	./lab2_add --threads=4 --iterations=1000 --yield >> lab2_add.csv
	./lab2_add --threads=8 --iterations=1000 --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=1000 --yield >> lab2_add.csv

	./lab2_add --threads=2 --iterations=10000 --yield >> lab2_add.csv
	./lab2_add --threads=4 --iterations=10000 --yield >> lab2_add.csv
	./lab2_add --threads=8 --iterations=10000 --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000 --yield >> lab2_add.csv

	./lab2_add --threads=2 --iterations=100000 --yield >> lab2_add.csv
	./lab2_add --threads=4 --iterations=100000 --yield >> lab2_add.csv
	./lab2_add --threads=8 --iterations=100000 --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=100000 --yield >> lab2_add.csv

	./lab2_add --threads=1 --iterations=10 >> lab2_add.csv
	./lab2_add --threads=1 --iterations=20 >> lab2_add.csv
	./lab2_add --threads=1 --iterations=40 >> lab2_add.csv
	./lab2_add --threads=1 --iterations=80 >> lab2_add.csv
	./lab2_add --threads=1 --iterations=100 >> lab2_add.csv
	./lab2_add --threads=1 --iterations=1000 >> lab2_add.csv
	./lab2_add --threads=1 --iterations=10000 >> lab2_add.csv
	./lab2_add --threads=1 --iterations=100000 >> lab2_add.csv

	./lab2_add --threads=1 --iterations=10  --yield >> lab2_add.csv
	./lab2_add --threads=1 --iterations=20 --yield >> lab2_add.csv
	./lab2_add --threads=1 --iterations=40 --yield >> lab2_add.csv
	./lab2_add --threads=1 --iterations=80 --yield >> lab2_add.csv
	./lab2_add --threads=1 --iterations=100 --yield >> lab2_add.csv
	./lab2_add --threads=1 --iterations=1000 --yield >> lab2_add.csv
	./lab2_add --threads=1 --iterations=10000 --yield >> lab2_add.csv
	./lab2_add --threads=1 --iterations=100000 --yield >> lab2_add.csv

	./lab2_add --threads=1 --iterations=10000 --sync=m --yield >> lab2_add.csv
	./lab2_add --threads=2 --iterations=10000 --sync=m --yield >> lab2_add.csv
	./lab2_add --threads=4 --iterations=10000 --sync=m --yield >> lab2_add.csv
	./lab2_add --threads=8 --iterations=10000 --sync=m --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000 --sync=m --yield >> lab2_add.csv

	./lab2_add --threads=1 --iterations=10000 --sync=c --yield >> lab2_add.csv
	./lab2_add --threads=2 --iterations=10000 --sync=c --yield >> lab2_add.csv
	./lab2_add --threads=4 --iterations=10000 --sync=c --yield >> lab2_add.csv
	./lab2_add --threads=8 --iterations=10000 --sync=c --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000 --sync=c --yield >> lab2_add.csv

	./lab2_add --threads=1 --iterations=1000 --sync=c --yield >> lab2_add.csv
	./lab2_add --threads=2 --iterations=1000 --sync=s --yield >> lab2_add.csv
	./lab2_add --threads=4 --iterations=1000 --sync=s --yield >> lab2_add.csv
	./lab2_add --threads=8 --iterations=1000 --sync=s --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=1000 --sync=s --yield >> lab2_add.csv

	./lab2_add --threads=1 --iterations=10000 >> lab2_add.csv
	./lab2_add --threads=2 --iterations=10000 >> lab2_add.csv
	./lab2_add --threads=4 --iterations=10000 >> lab2_add.csv
	./lab2_add --threads=8 --iterations=10000 >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000 >> lab2_add.csv

	./lab2_add --threads=1 --iterations=10000 --sync=s >> lab2_add.csv
	./lab2_add --threads=2 --iterations=10000 --sync=s >> lab2_add.csv
	./lab2_add --threads=4 --iterations=10000 --sync=s >> lab2_add.csv
	./lab2_add --threads=8 --iterations=10000 --sync=s >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000 --sync=s >> lab2_add.csv

	./lab2_add --threads=1 --iterations=10000 --sync=m >> lab2_add.csv
	./lab2_add --threads=2 --iterations=10000 --sync=m >> lab2_add.csv
	./lab2_add --threads=4 --iterations=10000 --sync=m >> lab2_add.csv
	./lab2_add --threads=8 --iterations=10000 --sync=m >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000 --sync=m >> lab2_add.csv

	./lab2_add --threads=1 --iterations=10000 --sync=c >> lab2_add.csv
	./lab2_add --threads=2 --iterations=10000 --sync=c >> lab2_add.csv
	./lab2_add --threads=4 --iterations=10000 --sync=c >> lab2_add.csv
	./lab2_add --threads=8 --iterations=10000 --sync=c >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000 --sync=c >> lab2_add.csv

graphs: graphs-add graphs-list
	
graphs-add: lab2_add.gp
	./lab2_add.gp

graphs-list: lab2_list.gp
	./lab2_list.gp

# build the distribution tarball
dist: README lab2_add.c lab2_list.c SortedList.h SortedList.c lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png
	tar -zvcf lab2a-505114980.tar.gz Makefile README lab2_add.c lab2_list.c SortedList.h SortedList.c lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png

# delete all files created by the Makefile, and return the directory to its freshly untared state
clean:
	-rm -f *~ *.tar.gz *.csv *.png *.o lab2_add lab2_list

.PHONY: clean dist graphs build tests