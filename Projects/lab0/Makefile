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
default: lab0.c
	$(CC) $(CFLAGS) -o lab0 lab0.c

# runs a quick smoke-test on whether or not the program seems to work, supports the required arguments, and properly reports success or failure
check:
	./smoketest

# delete all files created by the Makefile, and return the directory to its freshly untared state
clean:
	rm lab0 *.o *~ *.tar.gz

# build the distribution tarball
dist:
	tar -zvcf lab0-505114980.tar.gz Makefile lab0.c README smoketest backtrace.png breakpoint.png

.PHONY: clean default check dist
