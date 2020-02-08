# NAME: Harrison Cassar
# EMAIL: Harrison.Cassar@gmail.com
# ID: 505114980

#compiler
CC = gcc

#compiler flags:
# -g    adds debugging information to the executable file
# -Wall turns on most, but not all, compiler warnings
# -Wextra enables some extra warning flags not enabled by -Wall
CFLAGS = -Wall -Wextra -g
CLIBRARIES = -pthread

# default build all
default: lab2_add.c
	$(CC) $(CFLAGS) $(CLIBRARIES) -o lab2_add lab2_add.c

# build the distribution tarball
dist:
	tar -zvcf lab2a-505114980.tar.gz Makefile README

# delete all files created by the Makefile, and return the directory to its freshly untared state
clean:
	rm -f *~ *.tar.gz

.PHONY: clean dist