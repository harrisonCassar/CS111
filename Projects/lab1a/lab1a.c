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

void restoreTerminal(struct termios *original);
int handleData(int fdin, int fdout, char* buf, int size, int mode);
void capwait(int pid, struct termios *original);
void sighandler(int signalnum);

struct option long_options[] =
{
	{"shell", no_argument, 0, 's'},
	{"debug", no_argument, 0, 'd'},
	{0, 0, 0, 0} //last element of long_options array must contain all 0's (end)
};

int FL_caughtSIGPIPE = 0;
int FL_shell = 0;
int FL_debug = 0;

const int BUFSIZE = 256;

int main(int argc, char* argv[])
{
	//declare variable to hold return values of system calls (used for error detection) and buffer for read operations
	int ret;
	char buf[BUFSIZE];	
	
	/* Parse options */

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
	
	/* Store and set terminal modes */

	//get original terminal modes ('normal' operation modes)
	struct termios original;
	ret = tcgetattr(0,&original);
	if (ret == -1)
	{
		fprintf(stderr,"Error while getting original terminal modes: %s\n", strerror(errno));
		exit(1);		
	}

	if (FL_debug == 1)
        fprintf(stderr,"Saved original terminal modes.\t\n");

	//make copy of original modes with specified changes
	struct termios modified = original;
	modified.c_iflag = ISTRIP;	/* only lower 7 bits	*/
	modified.c_oflag = 0;		/* no processing	*/
	modified.c_lflag = 0;		/* no processing	*/
	
	//set terminal modes with changes
	ret = tcsetattr(0,TCSANOW,&modified);
	if (ret == -1)
    {
		//NOTE: no need to restore terminal, as failure implies no changes were made
        fprintf(stderr,"Error while setting terminal modes: %s\n", strerror(errno));
        exit(1);
    }

	//check to make sure ALL changes were made
	struct termios modified_applied;
	ret = tcgetattr(0,&modified_applied);
	if (ret == -1)
	{
		fprintf(stderr,"Error while getting terminal modes: %s\r\n", strerror(errno));
        restoreTerminal(&original);
		exit(1);
	}

	if (modified_applied.c_iflag != ISTRIP || modified_applied.c_oflag != 0 || modified_applied.c_lflag != 0)
	{
		fprintf(stderr,"Error while setting terminal modes: Not all changes to put terminal in 'non-canonical input mode without echo' were made successfully.\n");
		restoreTerminal(&original);
		exit(1);	
	}

	if (FL_debug == 1)
        fprintf(stderr,"Set all specified changes to terminal modes...\r\nOperating in 'character-at-a-time, no echo' mode.\r\n");

	/* Operating in "character-at-a-time, no echo" mode. Input/Output Operations:  */

	if (FL_shell == 1)
	{
		//set up communication pipes
		int forward_pipefd[2];
		int backward_pipefd[2];

		ret = pipe(forward_pipefd);
		if (ret == -1)
		{
			fprintf(stderr, "Error while attempting to create pipes for communication between main and shell process: %s\r\n",strerror(errno));
			restoreTerminal(&original);
			exit(1);
		}
		
		ret = pipe(backward_pipefd);
		if (ret == -1)
		{
			fprintf(stderr, "Error while attempting to create pipes for communication between main and shell process: %s\r\n",strerror(errno));
			restoreTerminal(&original);
			exit(1);
		}

		if (FL_debug == 1)
        	fprintf(stderr,"Set up 2 communication pipes.\r\n");

		//fork processes
		int pid;
		pid = fork();
		if (pid == -1)
		{
			fprintf(stderr, "Error while attmepting fork of main process: %s\r\n",strerror(errno));
			restoreTerminal(&original);
			exit(1);	
		}	
		
		if (FL_debug == 1)
            fprintf(stderr,"Forked processes.\r\nPerforming input/output redirection in child process...\r\n");

		if (pid == 0) //child process
		{
			//input redirection
			ret = close(forward_pipefd[1]);
			if (ret == -1)
			{
				fprintf(stderr, "Error: %s; ", strerror(errno));
				switch (errno)
				{
					case EINTR:
						fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed because call was interrupted by a signal.\r\n",forward_pipefd[1]);
						break;
					case EIO:
						fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed because an I/O error occured.\r\n",forward_pipefd[1]);
						break;
					default:
						fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed with an unrecognized error.\r\n", forward_pipefd[1]);
						break;
				}		
				
				restoreTerminal(&original);	
				exit(1);
			}

			ret = close(0);
			if (ret == -1)
            {
                fprintf(stderr, "Error: %s; ", strerror(errno));
                switch (errno)
                {
                    case EINTR:
                        fprintf(stderr,"Attempted closing of stdin fd failed because call was interrupted by a signal.\r\n");
                        break;
                    case EIO:
                        fprintf(stderr,"Attempted closing of stdin fd failed because an I/O error occured.\r\n");
            			break; 
			       default:
                        fprintf(stderr,"Attempted closing of stdin fd failed with an unrecognized error.\r\n");
                		break;
				}
				
				restoreTerminal(&original);
				exit(1);
            }
		
			ret = dup(forward_pipefd[0]);
			if (ret == -1)
			{
				fprintf(stderr, "Error: %s; ", strerror(errno));
				switch (errno)
                {
                    case EBADF:
                        fprintf(stderr,"Attempted dup of forward pipe's read end specified by fd '%d' failed because the newfd is out of the allowed range for file descriptors.\r\n", forward_pipefd[0]);
                        break;
                    case EMFILE:
                        fprintf(stderr,"Attempted dup of forward pipe's read end specified by fd '%d' failed because the per-process limit on the number of open file descriptors has been reached.\r\n", forward_pipefd[0]);
                        break;
                   	default:
                        fprintf(stderr,"Attempted dup of forward pipe's read end specified by fd '%d' failed with an unrecognized error.\r\n", forward_pipefd[0]);
                        break;
                }
				
				restoreTerminal(&original);
				exit(1);
			}
			
			ret = close(forward_pipefd[0]);
			if (ret == -1)
            {
                fprintf(stderr, "Error: %s; ", strerror(errno));
                switch (errno)
                {
                    case EINTR:
                        fprintf(stderr,"Attempted closing of forward pipe's read end specified by fd '%d' failed because call was interrupted by a signal.\r\n",forward_pipefd[0]);
                        break;
                    case EIO:
                        fprintf(stderr,"Attempted closing of forward pipe's read end specified by fd '%d' failed because an I/O error occured.\r\n",forward_pipefd[0]);
                        break;
                    default:
                        fprintf(stderr,"Attempted closing of forward pipe's read end specified by fd '%d' failed with an unrecognized error.\r\n", forward_pipefd[0]);
                        break;
                }

				restoreTerminal(&original);
				exit(1);
            }
	
			//output redirection
			ret = close(backward_pipefd[0]);
			if (ret == -1)
            {
                fprintf(stderr, "Error: %s; ", strerror(errno));
                switch (errno)
                {
                    case EINTR:
                        fprintf(stderr,"Attempted closing of backward pipe's read end specified by fd '%d' failed because call was interrupted by a signal.\r\n",backward_pipefd[0]);
                        break;
                    case EIO:
                        fprintf(stderr,"Attempted closing of backward pipe's read end specified by fd '%d' failed because an I/O error occured.\r\n",backward_pipefd[0]);
                        break;
                    default:
                        fprintf(stderr,"Attempted closing of backward pipe's read end specified by fd '%d' failed with an unrecognized error.\r\n", backward_pipefd[0]);
                        break;
                }

                restoreTerminal(&original);
                exit(1);
            }
			
			ret = close(1);
			if (ret == -1)
            {
                fprintf(stderr, "Error: %s; ", strerror(errno));
                switch (errno)
                {
                    case EINTR:
                        fprintf(stderr,"Attempted closing of stdout fd failed because call was interrupted by a signal.\r\n");
                        break;
                    case EIO:
                        fprintf(stderr,"Attempted closing of stdout fd failed because an I/O error occured.\r\n");
                        break;
                    default:
                        fprintf(stderr,"Attempted closing of stdout fd failed with an unrecognized error.\r\n");
                        break;
                }

                restoreTerminal(&original);
                exit(1);
            }

			ret = dup(backward_pipefd[1]);
			if (ret == -1)
            {
                fprintf(stderr, "Error: %s; ", strerror(errno));
                switch (errno)
                {
                    case EBADF:
                        fprintf(stderr,"Attempted dup of backward pipe's write end specified by fd '%d' failed because the newfd is out of the allowed range for file descriptors.\r\n", backward_pipefd[1]);
                        break;
                    case EMFILE:
                        fprintf(stderr,"Attempted dup of backward pipe's write end specified by fd '%d' failed because the per-process limit on the number of open file descriptors has been reached.\r\n", backward_pipefd[1]);
                        break;
                    default:
                        fprintf(stderr,"Attempted dup of backward pipe's write end specified by fd '%d' failed with an unrecognized error.\r\n", backward_pipefd[1]);
                        break;
                }

                restoreTerminal(&original);
                exit(1);
            }	

			ret = close(backward_pipefd[1]);
			if (ret == -1)
            {
                fprintf(stderr, "Error: %s; ", strerror(errno));
                switch (errno)
                {
                    case EINTR:
                        fprintf(stderr,"Attempted closing of backward pipe's write end specified by fd '%d' failed because call was interrupted by a signal.\r\n",backward_pipefd[1]);
                        break;
                    case EIO:
                        fprintf(stderr,"Attempted closing of backward pipe's write end specified by fd '%d' failed because an I/O error occured.\r\n",backward_pipefd[1]);
                        break;
                    default:
                        fprintf(stderr,"Attempted closing of backward pipe's write end specified by fd '%d' failed with an unrecognized error.\r\n", backward_pipefd[1]);
                        break;
                }

                restoreTerminal(&original);
                exit(1);
            }			
			
			if (FL_debug == 1)
                fprintf(stderr,"Performed input/output redirection in child process.\r\nCalling execl(3) in child process...\r\n");
	
			//exec process as shell
			ret = execl("/bin/bash","sh", (char*) NULL);			
			if (ret == -1)
			{
				fprintf(stderr, "Error while attempting to call execl(3) on child process intended to become shell process: %s\r\n", strerror(errno));
				restoreTerminal(&original);
				exit(1);	
			}	
		}
		else //parent process
		{
			ret = close(forward_pipefd[0]);
			if (ret == -1)
			{
				fprintf(stderr, "Error: %s; ", strerror(errno));
				switch (errno)
				{
					case EBADF:
						fprintf(stderr,"Attempted closing of forward pipe's read end specified by fd '%d' failed because the file descriptor is not a valid open file descriptor.\r\n", forward_pipefd[0]);
						break;
					case EIO:
                        fprintf(stderr,"Attempted closing of forward pipe's read end specified by fd '%d' failed because an I/O error occured.\r\n",forward_pipefd[0]);
                        break;
                    default:
                        fprintf(stderr,"Attempted closing of forward pipe's read end specified by fd '%d' failed with an unrecognized error.\r\n", forward_pipefd[0]);
                        break;		
				}

				restoreTerminal(&original);
				exit(1);
			}

			ret = close(backward_pipefd[1]);			
			if (ret == -1)
			{
				fprintf(stderr, "Error: %s; ", strerror(errno));
                switch (errno)
                {
                    case EBADF:
                        fprintf(stderr,"Attempted closing of backward pipe's write end specified by fd '%d' failed because the file descriptor is not a valid open file descriptor.\r\n", backward_pipefd[1]);
                        break;
                    case EIO:
                        fprintf(stderr,"Attempted closing of backward pipe's write end specified by fd '%d' failed because an I/O error occured.\r\n",backward_pipefd[1]);
                        break;
                    default:
                        fprintf(stderr,"Attempted closing of backward pipe's write end specified by fd '%d' failed with an unrecognized error.\r\n", backward_pipefd[1]);
                        break;
                }

				restoreTerminal(&original);
				exit(1);
			}			
		
			if (FL_debug == 1)
                fprintf(stderr,"Closed unneeded ends of communication pipes with child shell process.\r\n");		

			//init data structures for poll(2)
			struct pollfd fds[2];
			fds[0].fd = 0;
			fds[1].fd = backward_pipefd[0];

			fds[0].events = POLLIN | POLLHUP | POLLERR;
			fds[1].events = POLLIN | POLLHUP | POLLERR;

			int FL_takeKeyboardInput = 1;			
			int data_ret;			

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
				
				restoreTerminal(&original);
				exit(1);	
			}
			
			if (FL_debug == 1)
                fprintf(stderr,"Registered signal handler for SIGPIPE signal.\r\n");

			int quitready = 0;

			for (;;)
			{
				ret = poll(fds,(unsigned long) 2, 0);
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

					restoreTerminal(&original);
					exit(1);
				}	
	
				//check and read available data streams
				if (ret > 0)
				{
					if (FL_takeKeyboardInput && ((fds[0].revents & POLLIN) == POLLIN))
					{
						if (FL_debug == 1)
                			fprintf(stderr,"Detected data on terminal input stream. Handling data...\r\n");

						data_ret = handleData(fds[0].fd,forward_pipefd[1],buf,BUFSIZE,0);
						
						if (FL_debug == 1)
                			fprintf(stderr,"Return value: %d\r\n",data_ret);	

						if (data_ret == -1)
						{
							restoreTerminal(&original);
							exit(1);
						}
						else if (data_ret == 1)
						{
							//receive EOF from terminal, close pipe to shell, but cont processing input from shell
							if (FL_debug == 1)
                				fprintf(stderr,"Received EOF from terminal.\r\n");

							FL_takeKeyboardInput = 0;
							ret = close(forward_pipefd[1]);
							if (ret == -1)
							{
								fprintf(stderr, "Error: %s; ", strerror(errno));
                				switch (errno)
                				{
                    				case EBADF:
                        				fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed because the file descriptor is not a valid open file descriptor.\r\n", forward_pipefd[1]);
                        				break;
                    				case EIO:
                        				fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed because an I/O error occured.\r\n",forward_pipefd[1]);
                        				break;
                    				default:
                        				fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed with an unrecognized error.\r\n", forward_pipefd[1]);
                        				break;
                				}

								restoreTerminal(&original);
								exit(1);
							}
						}
						else if (data_ret == 2)
						{
							//send SIGINT signal to child process
							if (FL_debug == 1)
                				fprintf(stderr,"Received interrupt from terminal. Sending SIGINT to child process...\r\n");

							ret = kill(pid,SIGINT);
							if (ret == -1)
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
							}
							
							FL_takeKeyboardInput = 0;

							if (FL_debug == 1)
                                fprintf(stderr,"Sent interrupt to child process.\r\n");
						
							//capwait(pid,&original);
                            //restoreTerminal(&original);
							//exit(1);
	
							ret = close(forward_pipefd[1]);
							if (ret == -1)
                            {
                                fprintf(stderr, "Error: %s; ", strerror(errno));
                                switch (errno)
                                {
                                    case EBADF:
                                        fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed because the file descriptor is not a valid open file descriptor.\r\n", forward_pipefd[1]);
                                        break;
                                    case EIO:
                                        fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed because an I/O error occured.\r\n",forward_pipefd[1]);
                                        break;
                                    default:
                                        fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed with an unrecognized error.\r\n", forward_pipefd[1]);
                                        break;
                                }

                                restoreTerminal(&original);
                                exit(1);
                            }
						}
						else if (data_ret == 3) //caught SIGPIPE
						{
							if (FL_debug == 1)
                				fprintf(stderr,"Caught SIGPIPE on write to pipe to child shell process. Attempting to exit with code 0...\r\n");

							ret = close(forward_pipefd[1]);
							if (ret == -1)
                            {
                                fprintf(stderr, "Error: %s; ", strerror(errno));
                                switch (errno)
                                {
                                    case EBADF:
                                        fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed because the file descriptor is not a valid open file descriptor.\r\n", forward_pipefd[1]);
                                        break;
                                    case EIO:
                                        fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed because an I/O error occured.\r\n",forward_pipefd[1]);
                                        break;
                                    default:
                                        fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed with an unrecognized error.\r\n", forward_pipefd[1]);
                                        break;
                                }

                                restoreTerminal(&original);
                                exit(1);
                            }
						
							quitready = 1;	
						}
					}
					
					if ((fds[1].revents & POLLIN) == POLLIN)
					{
						if (FL_debug == 1)
                			fprintf(stderr,"Detected data on input stream from child shell process. Handling data...\r\n");
						
						data_ret = handleData(fds[1].fd,1,buf,BUFSIZE,1);
						
						if (FL_debug == 1)
                			fprintf(stderr,"Return value: %d\r\n",data_ret);

						if (data_ret == -1)
						{
							restoreTerminal(&original);
							exit(1);
						}
						else if (data_ret == 1)
						{
							//EOF read; collect shell's exit status and report to stderr
							if (FL_debug == 1)
                				fprintf(stderr,"EOF read from child shell process. Attempting exit with code 0...\r\n");
							
							capwait(pid,&original);
							restoreTerminal(&original);
							exit(0);
						}
					}

					//check for errors on poll streams (POLLHUP, POLLERR)
					if ((fds[0].revents & POLLHUP) == POLLHUP)
					{
						//terminal has hungup
						if (FL_debug == 1)
                			fprintf(stderr,"Terminal reader has hungup.\r\n");

						FL_takeKeyboardInput = 0;
                        ret = close(forward_pipefd[1]);
                        if (ret == -1)
                        {
							fprintf(stderr, "Error: %s; ", strerror(errno));
                            switch (errno)
                            {
                            	case EBADF:
                                	fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed because the file descriptor is not a valid open file descriptor.\r\n", forward_pipefd[1]);
                                    break;
                                case EIO:
                                	fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed because an I/O error occured.\r\n",forward_pipefd[1]);
                                    break;
                                default:
                                	fprintf(stderr,"Attempted closing of forward pipe's write end specified by fd '%d' failed with an unrecognized error.\r\n", forward_pipefd[1]);
                                    break;
                            }

                        	restoreTerminal(&original);
                       		exit(1);
                      	}	
						
						quitready = 1;	
					}
					
					if ((fds[1].revents & POLLHUP) == POLLHUP)
					{
						//shell has hungup; collect shell's exit status and report to stderr
						if (FL_debug == 1)
                       		fprintf(stderr,"Shell reader has hungup.\r\n");

						quitready = 1;
					}

					if ((fds[0].revents & POLLERR) == POLLERR)
                    {
						if (FL_debug == 1)
							fprintf(stderr,"POLLERR condition reached on fd specified by '%d' that indicates stdin to the main process\r\n",fds[0].fd);
						
                    	quitready = 1;
					}

                    if ((fds[1].revents & POLLERR) == POLLERR)
                    {
						if (FL_debug == 1)
							fprintf(stderr,"POLLERR condition reached on fd specified by '%d' that indicates the read end of the pipe to the shell\r\n",fds[1].fd);
                    
						quitready = 1;	
                    }
					
					if (quitready == 1)
					{
						capwait(pid,&original);
                        restoreTerminal(&original);
                        exit(0);
					}	
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
		{
			int data_ret;
			data_ret = handleData(0,1,buf,BUFSIZE,2);
			if (data_ret == -1) //error reached
			{
				restoreTerminal(&original);
				exit(1);
			}

			else if (data_ret == 1) //escape sequence found
			{
				if (FL_debug == 1)
					fprintf(stderr,"\r\nEscape sequence encountered. Exiting with code 0...\r\n");
				
				restoreTerminal(&original);
				exit(0);
			}

			/*else if (data_ret == 2) //interrupt character found
			{
				if (FL_debug == 1)
					fprintf(stderr,"\r\nInterrupt character encountered. Exiting with code 1...\r\n");
				
				restoreTerminal(&original);
				exit(1);
			}*/ 
		}
	}		
}

void restoreTerminal(struct termios *original)
{
	int ret = tcsetattr(0,TCSANOW,original);
	if (ret < 0)
	{
		fprintf(stderr,"Error while attempting to restore terminal modes: %s\r\n", strerror(errno));
		exit(1);
	}	
}

void sighandler(int signalnum)
{
	FL_caughtSIGPIPE = 1;

	if (FL_debug == 1)
		fprintf(stderr,"Caught signal '%d'.\r\n",signalnum);
}

void capwait(int pid, struct termios *original)
{
	int status;
	int ret;
	ret = waitpid(pid,&status,0);
    if (ret == -1)
    {
    	fprintf(stderr, "Error: %s; ", strerror(errno));
        switch (errno)
        {
        	case ECHILD:
            	fprintf(stderr, "Attempted call of waitpid(2) with process specified by the pid '%d' failed because the process does not exist.\r\n", pid);
                break;
            case EINTR:
            	fprintf(stderr, "Attempted call of waitpid(2) with process specified by the pid '%d' failed because WNOHANG was not set and an unblocked signal or a SIGCHLD was caught.\r\n", pid);
                break;
            default:
            	fprintf(stderr, "Attempted call of waitpid(2) with process specified by the pid '%d' failed with an unrecognized error.\r\n", pid);
                break;
        }

		restoreTerminal(original);
    	exit(1);
    }

    fprintf(stderr,"SHELL EXIT SIGNAL=%d STATUS=%d\r\n",status & 0x007f,(status & 0xff0) >> 8);//status&0x007f,status&0xff00 >> 8);
}

int handleData(int fdin, int fdout, char* buf, int size, int mode)
{
	//mode parameter indicates data handling mode, where 0 indicates coming from terminal, 1 indicates from shell child process, 2 indicates just terminal (no passing to shell)
	const int INTR = 2; 
	const int ESC = 1;
	const int DONE = 0;
	const int ERR = -1;
	const int PIPE = 3;
	int bytesRead;
	int bytesWritten;
	int retval;
	int quit = 0;

    char newline[2] = {0x0D,0x0A};
	char escape[4] = {'^','D',0x0D,0x0A};
	char interrupt[4] = {'^','C',0x0D,0x0A};	

	//read bytes from input
	bytesRead = read(fdin,buf,size);

	//check for error
	if (bytesRead == -1)
	{
		fprintf(stderr,"Error: %s; ", strerror(errno));
        switch (errno)
        {
        	case EINTR:
            	fprintf(stderr,"Attempted read of file specified by fdin '%d' failed because call was interrupted by a signal before any data was read.\r\n", fdin);
            	break;
            case EIO:
            	fprintf(stderr,"Attempted read of file specified by fdin '%d' failed because of an I/O error.\r\n", fdin);
                break;
            default:
            	fprintf(stderr,"Attempted read of file specified by fdin '%d' failed with an unrecognized error.\r\n", fdin);
            	break;
        }
		
		return ERR;
	}		
	
	for (int i = 0; i < bytesRead; i++)
	{
		if (FL_debug == 1)
			fprintf(stderr,"Character read: %d\r\n",buf[i]);

		//check for escape sequence
		if (buf[i] == 0x04)
		{
			bytesWritten = write(1,escape,4);
			retval = ESC;
			quit = 1;
		}
		
		//check for interrupt
		else if (buf[i] == 0x03)
		{
			bytesWritten = write(1,interrupt,4);
			retval = INTR;
			quit = 1;
		}
		
		//process newline (if present) and write to output
		else if (buf[i] == 0x0D || buf[i] == 0x0A)
        {
			if (mode == 0) //coming from terminal, passing to shell
				bytesWritten = write(fdout,newline+1,1);
			else //mode is 1, coming from shell
				bytesWritten = write(fdout,newline,2);

			retval = DONE;
        }
		
		//write any other character normally
		else
		{
 	        bytesWritten = write(fdout,buf+i,1);
			retval = DONE;
		}

		//check for write error
		if (bytesWritten == -1)
		{
			fprintf(stderr,"Error: %s; ", strerror(errno));
            switch (errno)
            {
            	case EDQUOT:
                	fprintf(stderr,"Attempted write to file specified by fdout '%d' failed because user's quote of disk blocks on the filesystem containing stdin has been exhausted.\r\n", fdout);
                    break;
                case EFBIG:
                	fprintf(stderr,"Attempted write to file specified by fdout '%d' failed because an attempt was made to write a file that exceeds the implementation-defined max file size or the process's file size limit, or to write as a position past the max allowed offset.\r\n", fdout);
                    break;
               	case EINTR:
                	fprintf(stderr,"Attempted write to file specified by fdout '%d' failed because call was interrupted by a signal before any data was written.\r\n", fdout); //THIS CAN BE SIGNAL SOURCE FOR SIGPIPE
                    break;
                case EIO:
                	fprintf(stderr,"Attempted write to file specified by fdout '%d' failed with a low-level I/O error while modifying the inode.\r\n", fdout);
                    break;
               	case EPERM:
                	fprintf(stderr,"Attempted write to file specified by fdout '%d' failed because operation was prevented by a file seal.\r\n", fdout);
                    break;
                default:
                	fprintf(stderr,"Attempted write to file specified by fdout '%d' failed with an unrecognized error.\r\n", fdout);
                    break;
            }
			
			//if SIGPIPE signal caught, then instead return PIPE
			if (FL_caughtSIGPIPE)
			{
				FL_caughtSIGPIPE = 0;
				return PIPE;			
			}			

			return ERR;
		}

		if (quit == 1)
			break;

		//also print to screen if coming from terminal
		if (mode == 0)
		{
			if (buf[i] == 0x0D || buf[i] == 0x0A)
				bytesWritten = write(1,newline,2);					
 			else
				bytesWritten = write(1,buf+i,1);
			
			if (bytesWritten == -1)
        	{
            	fprintf(stderr,"Error: %s; ", strerror(errno));
            	switch (errno)
            	{
                	case EDQUOT:
                    	fprintf(stderr,"Attempted write to file specified by fdout '%d' failed because user's quote of disk blocks on the filesystem containing stdin has been exhausted.\r\n", fdout);
                    	break;
                	case EFBIG:
                    	fprintf(stderr,"Attempted write to file specified by fdout '%d' failed because an attempt was made to write a file that exceeds the implementation-defined max file size or the process's file size limit, or to write as a position past the max allowed offset.\r\n", fdout);
                    	break;
                	case EINTR:
                    	fprintf(stderr,"Attempted write to file specified by fdout '%d' failed because call was interrupted by a signal before any data was written.\r\n", fdout); //THIS CAN BE SIGNAL SOURCE FOR SIGPIPE
                    	break;
                	case EIO:
                    	fprintf(stderr,"Attempted write to file specified by fdout '%d' failed with a low-level I/O error while modifying the inode.\r\n", fdout);
                    	break;
                	case EPERM:
                    	fprintf(stderr,"Attempted write to file specified by fdout '%d' failed because operation was prevented by a file seal.\r\n", fdout);
                    	break;
                	default:
                    	fprintf(stderr,"Attempted write to file specified by fdout '%d' failed with an unrecognized error.\r\n", fdout);
                    	break;
            	}

            	//if SIGPIPE signal caught, then instead return PIPE
            	if (FL_caughtSIGPIPE)
            	{
            		FL_caughtSIGPIPE = 0;
            		return PIPE;
            	}
            	
				return ERR;
            }
		}
	}

	return retval;
}
