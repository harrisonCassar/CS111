# NAME: Harrison Cassar
# EMAIL: Harrison.Cassar@gmail.com
# ID: 505114980

#compiler
CC = gcc

#compiler flags:
# -g    adds debugging information to the executable file
# -Wall turns on most, but not all, compiler warnings
# -Wextra enables some extra warning flags not enabled by -Wall
# -lmraa links the mraa library, -lm links the math library
CFLAGS = -Wall -Wextra -g
CLIBRARIES = -lmraa -lm

# default build
default:
	$(CC) $(CFLAGS) $(CLIBRARIES) -o lab4c_tcp lab4c_tcp.c 

# build the distribution tarball
dist:
	tar -zvcf lab4c-505114980.tar.gz Makefile README lab4c_tcp.c lab4c_tls.c

# delete all files created by the Makefile, and return the directory to its freshly untared state
clean:
	rm -f *~ *.tar.gz lab4c_tcp lab4c_tls

.PHONY: clean dist
