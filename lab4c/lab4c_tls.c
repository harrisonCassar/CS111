//NAME: Harrison Cassar
//EMAIL: Harrison.Cassar@gmail.com
//ID: 505114980

//libraries
#include "mraa/aio.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <math.h>

//pin mappings
#define PIN_temperature 1
#define TCP_SERVER_PORT 18000
#define ID_SIZE 9
#define BUFSIZE 256

//auxiliary functions
void parseOptions(int argc, char** argv);
void prepareExit();
int protected_poll(struct pollfd *fds, nfds_t nfds, int timeout);
int protected_open(const char* filepath, int flags, mode_t mode);
void protected_close(int fd);
int protected_read(int fd, char* buf, size_t size);
int protected_write(int fd, const char* buf, size_t size);
int protected_SSL_read(SSL* ssl, void* buf, int num);
int protected_SSL_write(SSL* ssl, void* buf, int num);
int protected_socket(int socket_family, int socket_type, int protocol);
void handleSocketInput();
void logOutput(const char* buf);

//globals
//const char TCP_SERVER_HOST[] = "lever.cs.ucla.edu";

double pollPeriod = 1; //Seconds
char tempScale = 'f'; //Fahrenheit = f, Celsius = c
char* id;
char idbuf[BUFSIZE];
char* host = NULL;
char* logfile = NULL;
int fd_logfile = 1;
int socketfd = 0;
int port = TCP_SERVER_PORT;

int FL_quitReady = 0;
int FL_run = 1;
int FL_stopped = 0;

int FL_fileOpened = 0;
int FL_sslCreated = 0;
int FL_tempSensorInit = 0;

mraa_aio_context temperature_aio;
mraa_result_t status = MRAA_SUCCESS;
char commandbuf[BUFSIZE] = "";
int curindex = 0;

const SSL_METHOD* mth;
SSL_CTX* ctx;
SSL* ssl;

struct option long_options[] =
{
    {"id", required_argument, 0, 'i'},
    {"host", required_argument, 0, 'h'},
	{"period", required_argument, 0, 'p'},
	{"scale", required_argument, 0, 's'},
	{"log", required_argument, 0, 'l'},
	{0, 0, 0, 0} //last element of long_options array must contain all 0's (end)
};

int main(int argc, char** argv)
{
    atexit(prepareExit);

	//parse options
	parseOptions(argc,argv);

    //open specified logfile
    fd_logfile = protected_open(logfile,O_TRUNC|O_CREAT|O_WRONLY,00777);
    FL_fileOpened = 1;

    //initalize SSL and MRAA
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
	//mraa_init();

    mth = TLSv1_client_method();
    if (mth == NULL)
    {
        fprintf(stderr, "Failed to create TLS client method.\n");
    	exit(2);
    }

    ctx = SSL_CTX_new(mth);
    if (ctx == NULL)
    {
        fprintf(stderr, "Failed to create new SSL context.\n");
    	exit(2);
    }

    ssl = SSL_new(ctx);
    if (ssl == NULL)
    {
        fprintf(stderr, "Failed to create new SSL structure.\n");
    	exit(2);
    }
    FL_sslCreated = 1;

    //initializes connection to temperature sensor (AIO)
	temperature_aio = mraa_aio_init(PIN_temperature);
    if (temperature_aio == NULL)
    {
    	fprintf(stderr, "Failed to initialize AIO %d for temperature sensor.\n", PIN_temperature);
    	exit(2);
    }
    FL_tempSensorInit = 1;

	//setup connection to server
	socketfd = protected_socket(AF_INET,SOCK_STREAM, 0);

	struct hostent* tmp = gethostbyname(host);
	if (tmp == NULL)
	{
		//error, check h_errno
		fprintf(stderr, "Error when finding the host.\n");
		exit(1);
	}

	struct sockaddr_in addr;
	
	addr.sin_family = AF_INET;
	bcopy((char*)tmp->h_addr,(char*)&addr.sin_addr.s_addr,tmp->h_length);
	addr.sin_port = htons(port);

	if (connect(socketfd, (struct sockaddr*) &addr,sizeof(addr)) == -1)
	{
		//error, check errno
		fprintf(stderr, "Error when attempting to connect client to server: %s\n", strerror(errno));

		exit(2);
	}

	/* Connection successful to host. */

    //connect SSL to created socket
    if (SSL_set_fd(ssl,socketfd) == 0)
    {
        fprintf(stderr,"Error when setting SSL to socketfd.\n");
		exit(2);
    }

    //connect SSL to server (handshake)
    if (SSL_connect(ssl) <= 0)
    {
        fprintf(stderr,"Error when connecting SSL to server.\n");
		exit(2);
    }

	//send (and log) an ID terminated with a newline
	id[ID_SIZE] = '\0';
	sprintf(idbuf,"ID=%s\n",id);
	protected_SSL_write(ssl,idbuf,ID_SIZE+4);
	protected_write(fd_logfile,idbuf,ID_SIZE+4);

	//send (and log) newline-terminated temperature reports over socket
	//process (and log) newline-terminated commands received over the connection
	
	//init data structures for input handling section of code
    int ret;
    time_t currenttime;
    time_t lasttime;
    struct tm* current;
    char outputbuf[BUFSIZE];

    time(&lasttime);
    lasttime = currenttime;

	struct pollfd infds[1];
	infds[0].fd = socketfd;
	infds[0].events = POLLIN | POLLHUP | POLLERR;

	//continuously poll for input from socket and temperature sensor (until socket sends OFF signal)
    while (FL_run)
    {
    	if (!FL_stopped)
    	{
	    	//sample temperature sensor if pollPeriod has elapsed
	   		time(&currenttime);
			
			if (difftime(currenttime,lasttime) >= pollPeriod)
			{
				lasttime = currenttime;
				current = localtime(&currenttime);

				//read from temperature sensor
				int raw_temperature = mraa_aio_read(temperature_aio);
				if (raw_temperature == -1)
				{
					fprintf(stderr, "Failed to read from temperature sensor.\n");
					exit(2);
				}

				//converts temperature reading into a temperature (scale controlled with --scale)
				double temp = 1023.0/((double)raw_temperature) - 1.0;
			    temp *= 100000.0;

			    double temperature = 1.0 / (log(temp/100000.0)/4275 + 1/298.15) - 273.15; // convert to temperature via datasheet

				if (tempScale == 'f')
					temperature = temperature * 9/5 + 32;

				//log timestamp and processed temperature reading
				char hour[3] = "";
				char min[3] = "";
				char sec[3] = "";

				if (current->tm_hour < 10)
					sprintf(hour,"0%d",current->tm_hour);
				else
					sprintf(hour,"%d",current->tm_hour);
				
				if (current->tm_min < 10)
					sprintf(min,"0%d",current->tm_min);
				else
					sprintf(min,"%d",current->tm_min);
				
				if (current->tm_sec < 10)
					sprintf(sec,"0%d",current->tm_sec);
				else
					sprintf(sec,"%d",current->tm_sec);

				sprintf(outputbuf,"%s:%s:%s %.1f\n", hour, min, sec, temperature);
				protected_SSL_write(ssl,outputbuf,strlen(outputbuf));

				logOutput(outputbuf);
			}
		}
		
		//check and read available data from socket
		ret = protected_poll(infds,(unsigned long) 1, 0);
		
		if (ret > 0)
		{
			//check for POLLIN events
			if ((infds[0].revents & POLLIN) == POLLIN)
				handleSocketInput(); //process input from terminal

        	//check for POLLHUP or POLLERR events
			if ((infds[0].revents & (POLLHUP | POLLERR)))
			{
				fprintf(stderr, "POLLHUP or POLLERR detected on socket input stream.\n");
				break;
			}
		}
    }

    //exited loop means SHUTDOWN is detected (from socket)
    //record time
	time(&currenttime);
	current = localtime(&currenttime);
	
	//write SHUTDOWN report to and append to logfile
	char hour[3] = "";
	char min[3] = "";
	char sec[3] = "";

	if (current->tm_hour < 10)
		sprintf(hour,"0%d",current->tm_hour);
	else
		sprintf(hour,"%d",current->tm_hour);
	
	if (current->tm_min < 10)
		sprintf(min,"0%d",current->tm_min);
	else
		sprintf(min,"%d",current->tm_min);
	
	if (current->tm_sec < 10)
		sprintf(sec,"0%d",current->tm_sec);
	else
		sprintf(sec,"%d",current->tm_sec);

	sprintf(outputbuf,"%s:%s:%s SHUTDOWN\n", hour, min, sec);

	logOutput(outputbuf);
	
	exit(0);
}

void parseOptions(int argc, char** argv)
{
	int c; //stores returned character from getopt call
	int option_index; //stores index of found option
	char* raw_period;
	char* raw_scale;
	int len;

    int FL_id = 0;
    int FL_host = 0;
    int FL_log = 0;
    int FL_port = 0;

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
				fprintf(stderr, "Specified option not recognized.\nusage: %s --id=9-digit-# --host=name-or-address --log=filename port [--period=#] [--scale=[FC]]\n\t--id: specifies the id number to associate with the client during connection to the server\n\t--host: specifies the host name or address of the server to connect to\n\t--log: log all inputted commands and reading reports to the specified logfile.\n\tport: specifies server's port to connect to\n\t--period: specifies the time interval in-between temperature sensor readings.\n\t--scale: specifies temperature reading scale (F for Fahrenheit [default], C for Celsius).\n\n", argv[0]);
				exit(1);
                break;
            case 'i':
                FL_id = 1;
                if (strlen(optarg) != ID_SIZE)
                {
                    fprintf(stderr, "Specified ID is not a 9-digit number.\n");
				    exit(1);
                }

				id = optarg;

				int i = 0;
                for (; i < ID_SIZE; i++)
                {
                    if (!isdigit(optarg[i]))
                    {
                        fprintf(stderr, "Specified ID must contain only valid numerical digits: 0 to 9.");
				        exit(1);
                    }
                }
                break;
            case 'h':
                FL_host = 1;
                host = optarg;
                break;
			case 'l':
				FL_log = 1;
				logfile = optarg;
				break;
			case 'p':
				raw_period = optarg;
				pollPeriod = (double) strtol(raw_period,NULL,10);
				break;
			case 's':
				raw_scale = optarg;
				len = strlen(raw_scale);
				if (len != 1)
				{
					fprintf(stderr,"Specified scale '%s' is too many characters. Valid scales: F (Fahrenheit) or C (Celsius).\n", raw_scale);
					exit(1);
				}

				switch (raw_scale[0])
				{
					default:
						fprintf(stderr,"Specified scale '%s' is unrecognized. Valid scales: F (Fahrenheit) or C (Celsius).\n", raw_scale);
						exit(1);
					case 'c':
					case 'C':
						tempScale = 'c';
						break;
					case 'f':
					case 'F':
						tempScale = 'f';
						break;
				}

				break;
		}
	}

    //now check for port (extra option, "non-switch parameter")
    if (optind < argc)
    {
        FL_port = 1;
        port = atoi(argv[optind]);
    }

    //check for argument errors
    if (!FL_id || !FL_host || !FL_log || !FL_port)
    {
        fprintf(stderr,"Did not pass in all of the mandatory arguments.\n\nusage: %s --id=9-digit-# --host=name-or-address --log=filename port [--period=#] [--scale=[FC]]\n\t--id: specifies the id number to associate with the client during connection to the server\n\t--host: specifies the host name or address of the server to connect to\n\t--log: log all inputted commands and reading reports to the specified logfile.\n\tport: specifies server's port to connect to\n\t--period: specifies the time interval in-between temperature sensor readings.\n\t--scale: specifies temperature reading scale (F for Fahrenheit [default], C for Celsius).\n\n", argv[0]);
		exit(1);
    }

	if (pollPeriod < 0)
	{
		fprintf(stderr,"Specified period '%e' cannot be negative. Please pick a different period.\n", pollPeriod);
		exit(1);
	}

	if (pollPeriod == 0 && strcmp(raw_period,"0") != 0)
	{
		fprintf(stderr, "Specified period '%s' is inconvertable to a numeric value.\n", raw_period);
		exit(1);
	}

	//check if extra unexpected arguments were specified beyond 1 for the port (not supported)
	if ((argc-1)-optind != 0)
	{
		fprintf(stderr, "Extra unexpected option not recognized.\nusage: %s --id=9-digit-# --host=name-or-address --log=filename port [--period=#] [--scale=[FC]]\n\t--id: specifies the id number to associate with the client during connection to the server\n\t--host: specifies the host name or address of the server to connect to\n\t--log: log all inputted commands and reading reports to the specified logfile.\n\tport: specifies server's port to connect to\n\t--period: specifies the time interval in-between temperature sensor readings.\n\t--scale: specifies temperature reading scale (F for Fahrenheit [default], C for Celsius).\n\n", argv[0]);
		exit(1);
	}
}

void prepareExit()
{
	
    if (FL_tempSensorInit)
    {
       status = mraa_aio_close(temperature_aio);
        if (status != MRAA_SUCCESS)
        {
            fprintf(stderr, "Unable to close AIO for temperature sensor.\n");
            mraa_result_print(status);
        } 
    }
    
	//mraa_deinit();

	if (FL_fileOpened)
        protected_close(fd_logfile);
    
    if (FL_sslCreated)
    {
        SSL_shutdown(ssl);
        SSL_free(ssl);
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

		exit(2);
	}

	return ret;
}

int protected_open(const char* filepath, int flags, mode_t mode)
{
	int fd = open(filepath, flags, mode);
	if (fd == -1)
	{
		//error has occured, check errno
		switch (errno)
		{
			case EACCES:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because of inability to request access to file, or search permission denied for one of the directories in the file's path, or file did not exist yet and write access is not allowed in parent directory.\n", filepath);
				break;
			case EFAULT:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because file's path points to outside your accessible address space.\n", filepath);
				break;
			case EINTR:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because open call interrupted by a signal handler while blocked waiting to complete an open of a slow device.\n", filepath);
				break;
			case ELOOP:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because too many symbolic links were encountered in resolving the file's path.\n", filepath);
				break;
			case EMFILE:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because the per-process limit on the number of open file descriptors has been reached.\n", filepath);
				break;
			case ENFILE:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because the system-wide limit on the total number of open files has been reached.\n", filepath);
				break;
			case ENAMETOOLONG:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because file's path was too long.\n", filepath);
				break;
			case ENODEV:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because file's path refers to a device-special file and no corresponding device exists.\n", filepath);
				break;
			case ENOENT:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because file does not exist or a directory component in the file's path does not exist or is a dangling symbolic link.\n", filepath);
				break;
			case ENOMEM:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because either the file is a FIFO, but memory for the FIFO buffer can't be allocated because the per-user hard limit on memory allocation for pipes has been reached and the caller is not privileged or insufficient kernel memory was available.\n", filepath);
				break;
			case ENOTDIR:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because a component used as a directory in the file's path is not a directory.\n", filepath);
				break;
			case ENXIO:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because the file is a device-special file and no corresponding device exists or file is a UNIX domain socket.\n", filepath);
				break;
			case EOVERFLOW:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because file's path refers to a regular file that is too large to be opened.\n", filepath);
				break;
			case EPERM:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed because the operation was prevented by a file seal.\n", filepath);
				break;
			default:
				fprintf(stderr,"Attempted open of specified file '%s' with option '--log' failed with an unrecognized error.\n", filepath);
				break;
		}

		exit(2);
	}

	return fd;
}

void protected_close(int fd)
{
	if (close(fd) == -1)
	{
		fprintf(stderr, "Error: %s; ", strerror(errno));
		switch (errno)
		{
			case EINTR:
				fprintf(stderr,"Attempted closing of fd '%d' failed because call was interrupted by a signal.\r\n", fd);
				break;
			case EIO:
				fprintf(stderr,"Attempted closing of fd '%d' failed because an I/O error occured.\r\n", fd);
				break;
			default:
				fprintf(stderr,"Attempted closing of fd '%d' failed with an unrecognized error.\r\n", fd);
				break;
		}		
		
		exit(2);
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
		
		exit(2);
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
            case EAGAIN:
            	fprintf(stderr,"The file descriptor fd refers to a file other than a socket and has been marked nonblocking (O_NONBLOCK), and the write would block.  See open(2) for further details on the O_NONBLOCK flag.\n");
            	break;
            case EBADF:
            	fprintf(stderr,"fd is not a valid file descriptor or is not open for writing.\n");
            	break;
            default:
            	fprintf(stderr,"Attempted write to file specified by fd '%d' failed with an unrecognized error.\r\n", fd);
                break;
        }

        exit(2);
	}

	return bytesWritten;
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
		
		exit(2);
	}

	return ret;
}

void handleSocketInput()
{
	int bytesRead;

	char buf[BUFSIZE];	

	//read bytes from socket input
	bytesRead = protected_SSL_read(ssl,buf,BUFSIZE);	

	//handle read bytes
	int i;
	for (i = 0; i < bytesRead; i++)
	{
		if (buf[i] != '\n')
		{
			commandbuf[curindex] = buf[i];
			curindex++;
		}
		else
		{
			commandbuf[curindex] = '\0';
			curindex = 0;

			if (strcmp(commandbuf,"OFF") == 0)
			{
				FL_run = 0;

				strcat(commandbuf,"\n");
				logOutput(commandbuf);

				break;
			}
			else if (strncmp(commandbuf,"SCALE=",6) == 0)
			{
				if (strcmp(commandbuf+6,"F") == 0)
					tempScale = 'f';
				else if (strcmp(commandbuf+6,"C") == 0)
					tempScale = 'c';
			}
			else if (strcmp(commandbuf,"STOP") == 0)
				FL_stopped = 1;
			else if (strcmp(commandbuf,"START") == 0)
				FL_stopped = 0;
			else if (strncmp(commandbuf,"PERIOD=",7) == 0)
			{
				double newPeriod = strtod(commandbuf+7,NULL);
				if (newPeriod > 0 || (newPeriod == 0 && strcmp(commandbuf+7,"0") == 0))
					pollPeriod = newPeriod;
			}

			//log all commands (regardless of validity) after processed
			strcat(commandbuf,"\n");
			logOutput(commandbuf);
		}
	}
}

void logOutput(const char* buf)
{
	protected_write(fd_logfile, buf, strlen(buf));
}

int protected_SSL_read(SSL* ssl, void* buf, int num)
{
    int ret = SSL_read(ssl,buf,num);
    if (ret <= 0)
    {
        fprintf(stderr,"Error while attempting to read bytes from SSL.");
        exit(2);
    }

    return ret;
}

int protected_SSL_write(SSL* ssl, void* buf, int num)
{
    int ret = SSL_write(ssl,buf,num);
    if (ret <= 0)
    {
        fprintf(stderr,"Error while attempting to write bytes to SSL.");
        exit(2);
    }

    return ret;
}