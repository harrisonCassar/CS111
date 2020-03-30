# NAME: Harrison Cassar
# EMAIL: Harrison.Cassar@gmail.com
# ID: 505114980

#compiler
CC = gcc

#compiler flags:
# -g    adds debugging information to the executable file
#  # -Wall turns on most, but not all, compiler warnings
# -Wextra enables some extra warning flags not enabled by -Wall
CFLAGS = -Wall -Wextra -g

# build lab0 executable
default: lab1a.c
	$(CC) $(CFLAGS) -o lab1a lab1a.c

# delete all files created by the Makefile, and return the directory to its freshly untared state
clean:
	rm -f lab1a *.o *~ *.tar.gz

# build the distribution tarball
dist:
	tar -zvcf lab1a-505114980.tar.gz Makefile lab1a.c README

.PHONY: clean default dist
