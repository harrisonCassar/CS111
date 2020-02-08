//NAME: Harrison Cassar
//EMAIL: Harrison.Cassar@gmail.com
//ID: 505114980

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <poll.h>
#include <sys/wait.h>
#include <zlib.h>

/* Globals */

#define BUFSIZE 256

//Flags
int FL_debug = 0;
int FL_specifiedPort = 0;
int FL_caughtSIGPIPE = 0;
int FL_forwardPipeClosed = 0;
int FL_quitReady = 0;
int FL_compress = 0;

//Others
char* raw_port;
int port;
int socketfd;
int clientconnection_socketfd;
int pid;

int forward_pipefd[2];
int backward_pipefd[2];

z_stream defstream;
z_stream infstream;

struct option long_options[] =
{
	{"port", required_argument, 0, 'p'},
	{"debug", no_argument, 0, 'd'},
	{"compress", no_argument, 0, 'c'},
	{0, 0, 0, 0} //last element of long_options array must contain all 0's (end)
};

/* Functions */
void capwait(int child);
void sighandler(int signalnum);
void parseOptions(int argc, char** argv);
int protected_socket(int socket_family, int socket_type, int protocol);
void protected_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
void protected_listen(int sockfd, int backlog);
int protected_accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
void protected_pipe(int* pipefd);
void protected_close(int fd);
void protected_dup(int fd);
int protected_poll(struct pollfd *fds, nfds_t nfds, int timeout);
int protected_read(int fd, char* buf, size_t size);
int protected_write(int fd, const char* buf, size_t size);
void protected_exec();
void handleShellInput();
void handleSocketInput();
void processToShell(char* buf, int size);
void cleanup();

int main(int argc, char* argv[])
{
	parseOptions(argc,argv);

	//init (de)compression streams
	if (FL_compress == 1)
	{
		defstream.zalloc = Z_NULL;
	    defstream.zfree = Z_NULL;
	    defstream.opaque = Z_NULL;

	    int ret = deflateInit(&defstream, Z_DEFAULT_COMPRESSION);
	    if (ret != Z_OK)
	    {
	    	fprintf(stderr, "Error while attempting to initialize deflate stream.\n");
	    	exit(1);
	    }

	    infstream.zalloc = Z_NULL;
	    infstream.zfree = Z_NULL;
	    infstream.opaque = Z_NULL;

	    ret = inflateInit(&infstream);
	    if (ret != Z_OK)
	    {
	    	fprintf(stderr, "Error while attempting to initialize inflate stream.\n");
	    	exit(1);
	    }
	}

	//set up socket
	socketfd = protected_socket(AF_INET,SOCK_STREAM, 0);

	atexit(cleanup);

	//bind socket with port
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

	protected_bind(socketfd,(struct sockaddr *) &addr, sizeof(addr));

	if (FL_debug == 1)
		fprintf(stderr,"Successfully bound server socket %d.\r\n", socketfd);

	//listen for connections
	protected_listen(socketfd,5);

	if (FL_debug == 1)
		fprintf(stderr,"Successfully set server socket %d to listen.\r\n", socketfd);

	//accept connection with client
	socklen_t addrlen = sizeof(addr);
	clientconnection_socketfd = protected_accept(socketfd,(struct sockaddr *) &addr, &addrlen);

	if (FL_debug == 1)
		fprintf(stderr,"Successfully established connection with client with a socketfd of %d for the connection.\r\n", clientconnection_socketfd);

	/* Connection made successfully with client. Proceed to begin working with a shell child process... */

	//set up sighandler for SIGPIPE
	if (signal(SIGPIPE,sighandler) == SIG_ERR)
	{
		fprintf(stderr, "Error: %s; ", strerror(errno));
		switch (errno)
		{
			case EINVAL:
				fprintf(stderr,"Attempted call of signal(2) with an invalid signal number 'SIGPIPE'\r\n");
				break;
			default:
				fprintf(stderr,"Attempted call of signal(2) with signal number 'SIGPIPE' failed with an unrecognized error.\r\n");
				break;
		}
			
		exit(1);	
	}

	if (FL_debug == 1)
		fprintf(stderr,"Registered signal handler for SIGPIPE signal.\r\n");

	//set up communication pipes
	protected_pipe(forward_pipefd);
	protected_pipe(backward_pipefd);

	if (FL_debug == 1)
    	fprintf(stderr,"Set up 2 communication pipes. fds: %d, %d, %d, %d\r\n",forward_pipefd[0],forward_pipefd[1],backward_pipefd[0],backward_pipefd[1]);

	//fork processes
	pid = fork();
	if (pid == -1)
	{
		fprintf(stderr, "Error while attempting fork of main process: %s\r\n",strerror(errno));
		exit(1);
	}

	if (FL_debug == 1)
        fprintf(stderr,"Forked processes.\r\nPerforming input/output redirection in child process...\r\n");

    if (pid == 0) //child process
	{
		//input redirection
		protected_close(forward_pipefd[1]);
		protected_close(0);
		protected_dup(forward_pipefd[0]);
		protected_close(forward_pipefd[0]);

		//output redirection
		protected_close(backward_pipefd[0]);
		protected_close(1);
		protected_dup(backward_pipefd[1]);
		protected_close(backward_pipefd[1]);

		if (FL_debug == 1)
            fprintf(stderr,"Performed input/output redirection in child process.\r\nCalling execl(3) in child process...\r\n");
	
        //exec process as shell
        protected_exec();
	}
	else //parent process
	{
		//close unneeded ends of communication pipes
		protected_close(forward_pipefd[0]);
		protected_close(backward_pipefd[1]);
		
		if (FL_debug == 1)
        	fprintf(stderr,"Closed unneeded ends of communication pipes with child shell process.\r\n");		

        //init data structures for poll(2)
        struct pollfd infds[2];
		infds[0].fd = clientconnection_socketfd;
		infds[1].fd = backward_pipefd[0];

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
					handleSocketInput(); //process input from socket

				if ((infds[1].revents & POLLIN) == POLLIN)
	        		handleShellInput(); //process input from shell

	        	//check for POLLHUP events
				if ((infds[0].revents & POLLHUP) == POLLHUP)
				{
					//socket has hungup
					if (FL_debug == 1)
	        			fprintf(stderr,"Socket reader has hungup.\r\n");
				
	        		if (FL_forwardPipeClosed == 0)
					{
						protected_close(forward_pipefd[1]);
						FL_forwardPipeClosed = 1;
					}

	        		//FL_quitReady = 1;
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
						fprintf(stderr,"POLLERR condition reached on fd specified by '%d' that indicates socket input to the main process\r\n",infds[0].fd);

					FL_quitReady = 1;
				}

	            if ((infds[1].revents & POLLERR) == POLLERR)
	            {
					if (FL_debug == 1)
						fprintf(stderr,"POLLERR condition reached on fd specified by '%d' that indicates the read end of the pipe to the shell\r\n",infds[1].fd);
	            	
	            	FL_quitReady = 1;
	            }
			}

			if (FL_quitReady == 1 || FL_caughtSIGPIPE == 1)
			{
				capwait(pid);
	            exit(0);
			}
		}
	}
}

void capwait(int child)
{
	int status;
    if (waitpid(child,&status,0) == -1)
    {
    	fprintf(stderr, "Error: %s; ", strerror(errno));
        switch (errno)
        {
        	case ECHILD:
            	fprintf(stderr, "Attempted call of waitpid(2) with process specified by the pid '%d' failed because the process does not exist.\r\n", child);
                break;
            case EINTR:
            	fprintf(stderr, "Attempted call of waitpid(2) with process specified by the pid '%d' failed because WNOHANG was not set and an unblocked signal or a SIGCHLD was caught.\r\n", child);
                break;
            default:
            	fprintf(stderr, "Attempted call of waitpid(2) with process specified by the pid '%d' failed with an unrecognized error.\r\n", child);
                break;
        }

    	exit(1);
    }

    fprintf(stderr,"SHELL EXIT SIGNAL=%d STATUS=%d\r\n",status & 0x007f,(status & 0xff0) >> 8); //status&0x007f,status&0xff00 >> 8);
}

void sighandler(int signalnum)
{
	FL_caughtSIGPIPE = 1;

	if (FL_debug == 1)
		fprintf(stderr,"Caught signal '%d'.\r\n",signalnum);
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
			case 'c':
				FL_compress = 1;
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

	if (FL_debug == 1) {
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

//NEED BETTER ERROR CHECKING
void protected_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
	if (bind(sockfd,addr,addrlen) == -1)
	{
		fprintf(stderr,"Error when attempting to bind socket to address: %s", strerror(errno));

		exit(1);
	}
}

void protected_listen(int sockfd, int backlog)
{
	if (listen(sockfd,backlog) == -1)
	{
		fprintf(stderr,"Error when attempting to listen on socket of fd %d with backlog of %d. Error: %s", sockfd, backlog, strerror(errno));

		exit(1);
	}
}

int protected_accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
	int ret = accept(sockfd,addr,addrlen);
	if (ret == -1)
	{
		fprintf(stderr,"Error when attempting to accept a connection on socket of fd %d. Error: %s", sockfd, strerror(errno));

		exit(1);
	}

	return ret;
}

void protected_pipe(int* pipefd)
{
	if (pipe(pipefd) == -1)
	{
		fprintf(stderr, "Error while attempting to create pipes for communication between main and shell process: %s\r\n",strerror(errno));
		exit(1);
	}
}

void protected_close(int fd)
{
	if (close(fd) == -1)
	{
		fprintf(stderr, "Error: %s; ", strerror(errno));
		switch (errno)
		{
			case EINTR:
				fprintf(stderr,"Attempted closing of fd '%d' failed because call was interrupted by a signal.\r\n",fd);
				break;
			case EIO:
				fprintf(stderr,"Attempted closing of fd '%d' failed because an I/O error occured.\r\n",fd);
				break;
			default:
				fprintf(stderr,"Attempted closing of fd '%d' failed with an unrecognized error.\r\n",fd);
				break;
		}		

		exit(1);
	}
}

void protected_dup(int fd)
{
	if (dup(fd) == -1)
	{
		fprintf(stderr, "Error: %s; ", strerror(errno));
		switch (errno)
        {
            case EBADF:
                fprintf(stderr,"Attempted dup of fd '%d' failed because the newfd is out of the allowed range for file descriptors.\r\n", fd);
                break;
            case EMFILE:
                fprintf(stderr,"Attempted dup of fd '%d' failed because the per-process limit on the number of open file descriptors has been reached.\r\n", fd);
                break;
           	default:
                fprintf(stderr,"Attempted dup of fd '%d' failed with an unrecognized error.\r\n", fd);
                break;
        }
		
		exit(1);
	}
}

void protected_exec()
{
	if (execl("/bin/bash","sh", (char*) NULL) == -1)
	{
		fprintf(stderr, "Error while attempting to call execl(3) on child process intended to become shell process: %s\r\n", strerror(errno));
		exit(1);	
	}	
}

void handleShellInput()
{
	if (FL_debug == 1)
		fprintf(stderr,"Detected data on input stream from child shell process. Handling data...\r\n");
						
	int bytesRead;

    //char newline[2] = {0x0D,0x0A};
	//char escape[4] = {'^','D',0x0D,0x0A};

	char buf[BUFSIZE];	

	//read bytes from shell input
	bytesRead = protected_read(backward_pipefd[0],buf,BUFSIZE);	
	
	//check for 0x04 in bytes
	for (int i = 0; i < bytesRead; i++)
	{
		if (buf[i] == 0x04)
			FL_quitReady = 1;
	}

	//send compressed/uncompressed shell output to socket
	if (FL_compress == 1)
	{
		char tmp[BUFSIZE];

		//compress bytes and send to socket
		defstream.avail_in = (uInt) bytesRead;
	    defstream.next_in = (Bytef *) buf;

	    while (defstream.avail_in != 0)
	    {	
	    	defstream.avail_out = (uInt) BUFSIZE;
	    	defstream.next_out = (Bytef *) tmp;
	    	
	    	if (FL_debug == 1)
	    		fprintf(stderr, "Compression loop iteration.\n");

	    	int def_ret = deflate(&defstream, Z_SYNC_FLUSH);
			if (def_ret != Z_OK)
	    	{
	    		fprintf(stderr, "Error while attempting to deflate terminal input.\n");
	    		exit(1);
	    	}

	    	//send off to socket
	    	if (defstream.avail_out == 0)
	    		protected_write(clientconnection_socketfd,tmp,BUFSIZE);
	    	else
	    		protected_write(clientconnection_socketfd,tmp,BUFSIZE-defstream.avail_out);
	    }
	}
	else //no compression before sending
		protected_write(clientconnection_socketfd,buf,bytesRead);
}

void handleSocketInput()
{
	if (FL_debug == 1)
		fprintf(stderr,"Server detected data on terminal input stream. Handling data...\r\n");

	int bytesRead;

	char buf[BUFSIZE];	

	//read bytes from terminal input
	bytesRead = protected_read(clientconnection_socketfd,buf,BUFSIZE);	
	
	//decompress bytes before sending to shell
	if (FL_compress == 1)
	{
		char tmp[BUFSIZE];

		//prepare for inflation
		infstream.avail_in = (uInt) bytesRead; // size of input
		infstream.next_in = (Bytef *) buf; // input char array

		while (infstream.avail_in != 0)
	    {
	    	infstream.avail_out = (uInt) BUFSIZE;
	    	infstream.next_out = (Bytef *) tmp;

	    	if (FL_debug == 1)
    			fprintf(stderr, "Decompression loop iteration.\n");

	    	int inf_ret = inflate(&infstream, Z_SYNC_FLUSH);
			if (inf_ret != Z_OK)
	    	{
    			fprintf(stderr, "Error while attempting to inflate input from socket.\n");
    			exit(1);
    		}

    		int bytesToProcess;
	    	
	    	if (infstream.avail_out == 0)
	    		bytesToProcess = BUFSIZE;
	    	else
	    		bytesToProcess = BUFSIZE-infstream.avail_out;

	    	//process inflated bytes
			processToShell(tmp,bytesToProcess);
		}
	}
	else //no need to decompress before sending to shell
	{
		processToShell(buf,bytesRead);
	}
}

void processToShell(char* buf, int size)
{
	char newline[2] = {0x0D,0x0A};

	for (int i = 0; i < size; i++)
	{
		if (FL_debug == 1)
			fprintf(stderr,"Server character read from socket: %d\r\n",buf[i]);

		//check for escape sequence
		if (buf[i] == 0x04)
		{
			if (FL_forwardPipeClosed == 0)
			{
				protected_close(forward_pipefd[1]);
				FL_forwardPipeClosed = 1;
			}
		}
		
		//check for interrupt
		else if (buf[i] == 0x03)
		{
			if (kill(pid,SIGINT) == -1)
			{
				fprintf(stderr, "Error: %s; ", strerror(errno));
				switch (errno)
                {
                    case EINVAL:
                        fprintf(stderr,"Attempted call of kill(2) of process with pid '%d' failed because an invalid signal 'SIGINT' was specified.\r\n", pid);
                        break;
                    case EPERM:
                        fprintf(stderr,"Attempted call of kill(2) of process with pid '%d' failed because the calling process does not have the permission to send the signal to the target process.\r\n",pid);
                        break;
					case ESRCH:
						fprintf(stderr,"Attempted call of kill(2) of process with pid '%d' failed because the target process does not exist.\r\n", pid);
						break;
                    default:
                        fprintf(stderr,"Attempted call of kill(2) of process with pid '%d' failed with an unrecognized error.\r\n", pid);
                        break;
                }

                exit(1);
			}
							
			if (FL_debug == 1)
				fprintf(stderr,"Sent interrupt to child process.\r\n");

			//potentially close pipe too
		}
		
		//process newline (if present) and write to output
		else if (buf[i] == 0x0D || buf[i] == 0x0A)
			protected_write(forward_pipefd[1],newline+1,1);
		
		//write any other character normally
		else
			protected_write(forward_pipefd[1],buf+i,1);
	}
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

//BETTER ERROR DESCRIPTIONS
void cleanup()
{
	//end (de)compression streams
	if (FL_compress == 1)
	{
		deflateEnd(&defstream);
    	inflateEnd(&infstream);
	}

	if (shutdown(clientconnection_socketfd,SHUT_RDWR) == -1)
	{
		fprintf(stderr, "Error while attempting shutdown of socket with fd %d: %s; ", socketfd, strerror(errno));

		exit(1);
	}
}
