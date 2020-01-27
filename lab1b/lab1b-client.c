//NAME: Harrison Cassar
//EMAIL: Harrison.Cassar@gmail.com
//ID: 505114980

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <poll.h>

/* Globals */

#define BUFSIZE 256

//Flags
int FL_debug = 0;
int FL_specifiedPort = 0;
int FL_quitReady = 0;

//Others
char* raw_port;
int port;
int socketfd;

struct termios original;

struct option long_options[] =
{
	{"port", required_argument, 0, 'p'},
	{"debug", no_argument, 0, 'd'},
	{0, 0, 0, 0} //last element of long_options array must contain all 0's (end)
};

/* Functions */
void parseOptions(int argc, char** argv);
void saveTerminal();
void setTerminal();
void restoreTerminal();
int protected_read(int fd, char* buf, size_t size);
int protected_write(int fd, const char* buf, size_t size);
void handleTerminalInput();
void handleSocketInput();
int protected_socket(int socket_family, int socket_type, int protocol);
int protected_poll(struct pollfd *fds, nfds_t nfds, int timeout);

int main(int argc, char* argv[])
{
	parseOptions(argc,argv);

	//save original terminal modes for restoring at exit, and set modified modes
	saveTerminal();
	setTerminal();

	/* Operating in "character-at-a-time, no echo" mode. Input/Output Operations:  */

	//setup connection to server
	socketfd = protected_socket(AF_INET,SOCK_STREAM, 0);

	struct hostent* tmp = gethostbyname("localhost");
	if (tmp == NULL)
	{
		//error, check h_errno
		fprintf(stderr,"Error when calling gethostbyname().\r\n");
		exit(1);
	}

	struct sockaddr_in addr;
	memcpy(&addr.sin_addr,tmp->h_addr_list[0],tmp->h_length);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	bzero(&addr.sin_addr.s_addr,sizeof(addr.sin_addr.s_addr)); 

	if (connect(socketfd, (struct sockaddr *) &addr,sizeof(addr)) == -1)
	{
		//error, check errno
		fprintf(stderr,"Error when attempting to connect client to server: %s\r\n", strerror(errno));

		exit(1);
	}

	if (FL_debug == 1)
		fprintf(stderr,"Successfully established connection to host.\r\n");

	/* Connection successful to host. Begin continually sending input from keyboard to socket while echoing to display, and input from the socket to the display */

	//init data structures for poll(2)
	struct pollfd infds[2];
	infds[0].fd = 0;
	infds[1].fd = socketfd;

	infds[0].events = POLLIN | POLLHUP | POLLERR;
	infds[1].events = POLLIN | POLLHUP | POLLERR;

	int ret;

	for (;;)
	{
		ret = protected_poll(infds,(unsigned long) 2, 0);

		//check and read available data streams
		if (ret > 0)
		{
			//check for POLLIN events
			if ((infds[0].revents & POLLIN) == POLLIN)
				handleTerminalInput(); //process input from terminal

			if ((infds[1].revents & POLLIN) == POLLIN)
        		handleSocketInput(); //process input from socket

        	//check for POLLHUP events
			if ((infds[0].revents & POLLHUP) == POLLHUP)
			{
				//terminal has hungup
				if (FL_debug == 1)
        			fprintf(stderr,"Terminal reader has hungup.\r\n");
			
        		FL_quitReady = 1;
			}

			if ((infds[1].revents & POLLHUP) == POLLHUP)
			{
				//shell has hungup; collect shell's exit status and report to stderr
				if (FL_debug == 1)
               		fprintf(stderr,"Shell reader has hungup.\r\n");
				
               	FL_quitReady = 1;
			}

			//check for POLLERR events
			if ((infds[0].revents & POLLERR) == POLLERR)
            {
				if (FL_debug == 1)
					fprintf(stderr,"POLLERR condition reached on fd specified by '%d' that indicates stdin to the main process\r\n",infds[0].fd);

				FL_quitReady = 1;
			}

            if ((infds[1].revents & POLLERR) == POLLERR)
            {
				if (FL_debug == 1)
					fprintf(stderr,"POLLERR condition reached on fd specified by '%d' that indicates the read end of the pipe to the shell\r\n",infds[1].fd);
            	
            	FL_quitReady = 1;
            }
		}

		if (FL_quitReady == 1)
            exit(0);
	}
}

void parseOptions(int argc, char** argv)
{
	int c; //stores returned character from getopt call
	int option_index; //stores index of found option

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
				fprintf(stderr, "Specified option '%s' not recognized.\nusage: %s --port=port# [--debug]\n\t--port=port#: specify the port number for communication over the network\n\t--debug: enable copious logging (to stderr) of every processed character and event\n\n", argv[optind-1], argv[0]);
				exit(1);
			case 'p':
				FL_specifiedPort = 1;
				raw_port = optarg;
				port = (int) strtol(raw_port,NULL,10);
				break;
			case 'd':
				FL_debug = 1;
				break;
		}
	}

	if (FL_specifiedPort != 1)
	{
		fprintf(stderr,"Option '--port=port#' is required.\nusage: %s --port=port# [--debug]\n\t--port=port#: specify the port number for communication over the network\n\t--debug: enable copious logging (to stderr) of every processed character and event\n\n", argv[0]);
		exit(1);
	}

	if (port == 0 && strcmp(raw_port,"0") != 0)
	{
		fprintf(stderr, "Specified port '%s' is inconvertable to a numeric value.\n", raw_port);
		exit(1);
	}

	if (port < 0)
	{
		fprintf(stderr,"Specified port '%d' cannot be negative. Please pick a different port.\n", port);
		exit(1);
	}

	if (port <= 1024)
	{
		fprintf(stderr,"Specified port '%d' is reserved. Please pick a different port.\n", port);
		exit(1);
	}

	if (FL_debug == 1)
	{
		fprintf(stderr,"Running in debug mode.\n");
		fprintf(stderr,"Specified port is: %d\n", port);
	}

	//check if non-option arguments were specified (not supported)
	if (argc-optind != 0)
	{
		fprintf(stderr, "Non-option argument '%s' is not supported.\nusage: %s [--shell] [--debug]\n\t--shell: pass input/output between the terminal and shell\n\t--debug: enable copious logging (to stderr) of every processed character and event\n\n", argv[optind], argv[0]);
		exit(1);
	}
}

void saveTerminal()
{
	//get original terminal modes ('normal' operation modes)
	if (tcgetattr(0,&original) == -1)
	{
		fprintf(stderr,"Error while getting original terminal modes: %s\n", strerror(errno));
		exit(1);		
	}

	if (FL_debug == 1)
        fprintf(stderr,"Saved original terminal modes.\t\n");
}

void setTerminal()
{
	//make copy of original modes with specified changes
	struct termios modified = original;
	modified.c_iflag = ISTRIP;	/* only lower 7 bits	*/
	modified.c_oflag = 0;		/* no processing	*/
	modified.c_lflag = 0;		/* no processing	*/
	
	//set terminal modes with changes
	if (tcsetattr(0,TCSANOW,&modified) == -1)
    {
		//NOTE: no need to restore terminal, as failure implies no changes were made
        fprintf(stderr,"Error while setting terminal modes: %s\n", strerror(errno));
        exit(1);
    }

    //modifications made, so whenever exit, must restore terminal modes
    atexit(restoreTerminal);

	//check to make sure ALL changes were made
	struct termios modified_applied;
	if (tcgetattr(0,&modified_applied) == -1)
	{
		fprintf(stderr,"Error while getting terminal modes: %s\r\n", strerror(errno));
		exit(1);
	}

	if (modified_applied.c_iflag != ISTRIP || modified_applied.c_oflag != 0 || modified_applied.c_lflag != 0)
	{
		fprintf(stderr,"Error while setting terminal modes: Not all changes to put terminal in 'non-canonical input mode without echo' were made successfully.\n");
		exit(1);	
	}

	if (FL_debug == 1)
        fprintf(stderr,"Set all specified changes to terminal modes...\r\nOperating in 'character-at-a-time, no echo' mode.\r\n");
}

void restoreTerminal()
{
	if (tcsetattr(0,TCSANOW,&original) == -1)
	{
		fprintf(stderr,"Error while attempting to restore terminal modes: %s\r\n", strerror(errno));
		exit(1);
	}
}

int protected_read(int fd, char* buf, size_t size)
{
	int bytesRead = read(fd,buf,size);

	//check for error
	if (bytesRead == -1)
	{
		fprintf(stderr,"Error: %s; ", strerror(errno));
        switch (errno)
        {
        	case EINTR:
            	fprintf(stderr,"Attempted read from fd '%d' failed because call was interrupted by a signal before any data was read.\r\n", fd);
            	break;
            case EIO:
            	fprintf(stderr,"Attempted read from fd '%d' failed because of an I/O error.\r\n", fd);
                break;
            default:
            	fprintf(stderr,"Attempted read from fd '%d' failed with an unrecognized error.\r\n", fd);
            	break;
        }
		
		exit(1);
	}

	return bytesRead;
}

int protected_write(int fd, const char* buf, size_t size)
{
	int bytesWritten = write(fd, buf, size);
	
	//check for write error
	if (bytesWritten == -1)
	{
		fprintf(stderr,"Error: %s; ", strerror(errno));
        switch (errno)
        {
        	case EDQUOT:
            	fprintf(stderr,"Attempted write to file specified by fd '%d' failed because user's quote of disk blocks on the filesystem containing stdin has been exhausted.\r\n", fd);
                break;
            case EFBIG:
            	fprintf(stderr,"Attempted write to file specified by fd '%d' failed because an attempt was made to write a file that exceeds the implementation-defined max file size or the process's file size limit, or to write as a position past the max allowed offset.\r\n", fd);
                break;
           	case EINTR:
            	fprintf(stderr,"Attempted write to file specified by fd '%d' failed because call was interrupted by a signal before any data was written.\r\n", fd);
                break;
            case EIO:
            	fprintf(stderr,"Attempted write to file specified by fd '%d' failed with a low-level I/O error while modifying the inode.\r\n", fd);
                break;
           	case EPERM:
            	fprintf(stderr,"Attempted write to file specified by fd '%d' failed because operation was prevented by a file seal.\r\n", fd);
                break;
            default:
            	fprintf(stderr,"Attempted write to file specified by fd '%d' failed with an unrecognized error.\r\n", fd);
                break;
        }

        exit(1);
	}

	return bytesWritten;
}

void handleTerminalInput()
{
	if (FL_debug == 1)
		fprintf(stderr,"Detected data on terminal input stream. Handling data...\r\n");

	int bytesRead;

    char newline[2] = {0x0D,0x0A};
	char escape[4] = {'^','D',0x0D,0x0A};
	char interrupt[4] = {'^','C',0x0D,0x0A};

	char buf[BUFSIZE];	

	//read bytes from terminal input
	bytesRead = protected_read(0,buf,BUFSIZE);	
	
	for (int i = 0; i < bytesRead; i++)
	{
		if (FL_debug == 1)
			fprintf(stderr,"Character read: %d\r\n",buf[i]);

		//check for escape sequence
		if (buf[i] == 0x04)
		{
			protected_write(1,escape,4);
			protected_write(socketfd,buf+i,1);
		}
		
		//check for interrupt character
		else if (buf[i] == 0x03)
		{
			protected_write(1,interrupt,4);
			protected_write(socketfd,buf+i,1);
		}
		
		//process newline (if present) and write to output
		else if (buf[i] == 0x0D || buf[i] == 0x0A)
        {
			protected_write(1,newline,2);
			//protected_write(socketfd,newline+1,1); //taken out, as this processing will be done on server side
			protected_write(socketfd,buf+i,1);
		}
		
		//write any other character normally
		else
		{
			protected_write(1,buf+i,1);
			protected_write(socketfd,buf+i,1);
		}
	}
}

void handleSocketInput()
{
	if (FL_debug == 1)
		fprintf(stderr,"Client detected data on socket input stream. Handling data...\r\n");

	int bytesRead;

    char newline[2] = {0x0D,0x0A};
	char escape[4] = {'^','D',0x0D,0x0A};
	//char interrupt[4] = {'^','C',0x0D,0x0A};

	char buf[BUFSIZE];	

	//read bytes from terminal input
	bytesRead = protected_read(socketfd,buf,BUFSIZE);	
	
	if (FL_debug == 1)
		fprintf(stderr,"Client bytesRead from socket: %d\r\n", bytesRead);

	for (int i = 0; i < bytesRead; i++)
	{
		//LOG OPTION NEEDS TO OUTPUT STUFF HERE!

		//check for escape sequence
		if (buf[i] == 0x04)
			protected_write(1,escape,4);
		
		//process newline (if present) and write to output
		else if (buf[i] == 0x0D || buf[i] == 0x0A)
			protected_write(1,newline,2);
		
		//write any other character normally
		else
			protected_write(1,buf+i,1);
	}

	if (bytesRead == 0)
		FL_quitReady = 1;
}

int protected_socket(int socket_family, int socket_type, int protocol)
{
	int ret = socket(socket_family,socket_type,protocol);
	if (ret == -1)
	{
		fprintf(stderr,"Error: %s; ", strerror(errno));
        switch (errno)
        {
        	case EACCES:
            	fprintf(stderr,"Attempted creation of socket failed because permission to create socket of specified type and/or protocol is denied.\r\n");
            	break;
            case EAFNOSUPPORT:
            	fprintf(stderr,"Attempted creation of socket failed because the implementation does not support the specified address family.\r\n");
                break;
            case EINVAL:
            	fprintf(stderr,"Attempted creation of socket failed because unknown protocol, or protocol family not avaiable, or invalid flags in type.\r\n");
            	break;
            case EMFILE:
            	fprintf(stderr,"Attempted creation of socket failed because the per-process limit on the number of open file descriptors has been reached.\r\n");
        		break;
        	case ENFILE:
        		fprintf(stderr,"Attempted creation of socket failed because the system-wide limit on the total number of open files has been reached.\r\n");
        		break;
        	case ENOBUFS:
        	case ENOMEM:
        		fprintf(stderr,"Attempted creation of socket failed because insufficient memory is avaiable. The socket cannot be created until sufficient resources are freed.\r\n");
        		break;
        	case EPROTONOSUPPORT:
        		fprintf(stderr,"Attempted creation of socket failed because the protocol type or the specified portocol is not supported within this domain.\r\n");
        		break;
            default:
            	fprintf(stderr,"Attempted creation of socket failed with an unrecognized error.\r\n");
            	break;
        }
		
		exit(1);
	}

	return ret;
}

int protected_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	int ret = poll(fds,nfds,timeout);
	if (ret == -1)
	{
		fprintf(stderr, "Error: %s; ", strerror(errno));
		switch (errno)
		{
			case EINTR:
				fprintf(stderr,"Attempted call of poll(2) failed because a signal occured before any requested event.\r\n");
				break;
			case EINVAL:
				fprintf(stderr,"Attempted call of poll(2) failed because the nfds parameter's value of 2 exceeds the RLIMIT_NOFILE value.\r\n");
				break;
			case ENOMEM:
				fprintf(stderr,"Attempted call of poll(2) failed because there was no space to allocate file descriptor tables.\r\n");
				break;
			default:
				fprintf(stderr,"Attempted call of poll(2) failed with an unrecognized error.\r\n");
				break;		
		}

		exit(1);
	}

	return ret;
}