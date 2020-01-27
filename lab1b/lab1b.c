//NAME: Harrison Cassar
//EMAIL: Harrison.Cassar@gmail.com
//ID: 505114980

#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>

/* Globals */
#define BUFSIZE 256

int FL_caughtSIGPIPE = 0;
int FL_shell = 0;
int FL_debug = 0;
int FL_forwardPipeClosed = 0;
int pid;
int quitready = 0;

int forward_pipefd[2];
int backward_pipefd[2];

struct termios original;

struct option long_options[] =
{
	{"shell", no_argument, 0, 's'},
	{"debug", no_argument, 0, 'd'},
	{0, 0, 0, 0} //last element of long_options array must contain all 0's (end)
};

/* Functions */
void capwait(int child);
void sighandler(int signalnum);
void parseOptions(int argc, char** argv);
void saveTerminal();
void setTerminal();
void restoreTerminal();
void protected_pipe(int* pipefd);
void protected_close(int fd);
void protected_dup(int fd);
void protected_exec();
int protected_poll(struct pollfd *fds, nfds_t nfds, int timeout);
int protected_read(int fd, char* buf, size_t size);
int protected_write(int fd, const char* buf, size_t size);
void handleTerminalInput();
void handleShellInput();
void handleTerminalInput_WithoutShell();

int main(int argc, char* argv[])
{
	parseOptions(argc,argv);

	//save original terminal modes for restoring at exit, and set modified modes
	saveTerminal();
	setTerminal();

	/* Operating in "character-at-a-time, no echo" mode. Input/Output Operations:  */

	if (FL_shell == 1)
	{
		//--shell option detected
		
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
			infds[0].fd = 0;
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
						handleTerminalInput(); //process input from terminal

					if ((infds[1].revents & POLLIN) == POLLIN)
                		handleShellInput(); //process input from shell

					//check for POLLHUP events
					if ((infds[0].revents & POLLHUP) == POLLHUP)
					{
						//terminal has hungup
						if (FL_debug == 1)
                			fprintf(stderr,"Terminal reader has hungup.\r\n");

                		if (FL_forwardPipeClosed == 0)
						{
							protected_close(forward_pipefd[1]);
							FL_forwardPipeClosed = 1;
						}

                		quitready = 1;
					}

					if ((infds[1].revents & POLLHUP) == POLLHUP)
					{
						//shell has hungup; collect shell's exit status and report to stderr
						if (FL_debug == 1)
                       		fprintf(stderr,"Shell reader has hungup.\r\n");

						quitready = 1;
					}

					//check for POLLERR events
					if ((infds[0].revents & POLLERR) == POLLERR)
                    {
						if (FL_debug == 1)
							fprintf(stderr,"POLLERR condition reached on fd specified by '%d' that indicates stdin to the main process\r\n",infds[0].fd);
						
                    	quitready = 1;
					}

                    if ((infds[1].revents & POLLERR) == POLLERR)
                    {
						if (FL_debug == 1)
							fprintf(stderr,"POLLERR condition reached on fd specified by '%d' that indicates the read end of the pipe to the shell\r\n",infds[1].fd);
                    
						quitready = 1;	
                    }
				}

				if (quitready == 1 || FL_caughtSIGPIPE == 1)
				{
					capwait(pid);
                    exit(0);
				}
			}
		}
	}
	else //no --shell option detected
	{
		if (FL_debug == 1)
        	fprintf(stderr,"No '--shell' option specified. Entering no shell mode...\r\n");		

        //read ASCII input from keyboard, and write to display output
		for (;;)
			handleTerminalInput_WithoutShell();
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
				fprintf(stderr, "Specified option '%s' not recognized.\nusage: %s [--shell] [--debug]\n\t--shell: pass input/output between the terminal and shell\n\t--debug: enable copious logging (to stderr) of every processed character and event\n\n", argv[optind-1], argv[0]);
				exit(1);
			case 's':
				FL_shell = 1;
				break;
			case 'd':
				FL_debug = 1;
				break;
		}
	}

	if (FL_debug == 1)
		fprintf(stderr,"Running in debug mode.\n");

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
			if (FL_forwardPipeClosed == 0)
			{
				protected_close(forward_pipefd[1]);
				FL_forwardPipeClosed = 1;
			}
		}
		
		//check for interrupt
		else if (buf[i] == 0x03)
		{
			protected_write(1,interrupt,4);
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
        {
			protected_write(1,newline,2);
			protected_write(forward_pipefd[1],newline+1,1);
		}
		
		//write any other character normally
		else
		{
			protected_write(1,buf+i,1);
			protected_write(forward_pipefd[1],buf+i,1);
		}
	}
}

void handleShellInput()
{
	if (FL_debug == 1)
		fprintf(stderr,"Detected data on input stream from child shell process. Handling data...\r\n");
						
	int bytesRead;

    char newline[2] = {0x0D,0x0A};
	char escape[4] = {'^','D',0x0D,0x0A};

	char buf[BUFSIZE];	

	//read bytes from terminal input
	bytesRead = protected_read(backward_pipefd[0],buf,BUFSIZE);	
	
	for (int i = 0; i < bytesRead; i++)
	{
		//check for escape sequence
		if (buf[i] == 0x04)
		{
			protected_write(1,escape,4);

			quitready = 1;
		}
		
		//process newline (if present) and write to output
		else if (buf[i] == 0x0D || buf[i] == 0x0A)
			protected_write(1,newline,2);
		
		//write any other character normally
		else
			protected_write(1,buf+i,1);
	}
}

void handleTerminalInput_WithoutShell()
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
			exit(0);
		}
		
		//check for interrupt (only print '^C')
		else if (buf[i] == 0x03)
			protected_write(1,interrupt,4);
		
		//process newline (if present) and write to output
		else if (buf[i] == 0x0D || buf[i] == 0x0A)
			protected_write(1,newline,2);
		
		//write any other character normally
		else
			protected_write(1,buf+i,1);
	}
}