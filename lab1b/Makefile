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
LIBRARIES = -lz

# build lab0 executable
default: client server

client: lab1b-client.c
	$(CC) $(CFLAGS) -o lab1b-client lab1b-client.c $(LIBRARIES)

server: lab1b-server.c
	$(CC) $(CFLAGS) -o lab1b-server lab1b-server.c $(LIBRARIES)

# delete all files created by the Makefile, and return the directory to its freshly untared state
clean:
	rm -f lab1b-client lab1b-server *.o *~ *.tar.gz

# build the distribution tarball
dist:
	tar -zvcf lab1b-505114980.tar.gz Makefile lab1b-client.c lab1b-server.c README

.PHONY: clean default dist client server
