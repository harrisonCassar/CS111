//NAME: Harrison Cassar
//EMAIL: Harrison.Cassar@gmail.com
//ID: 505114980

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void causeSegfault();
void sighandler(int signalnum);

struct option long_options[] =
{
	{"input", required_argument, 0, 'i'},
	{"output", required_argument, 0, 'o'},
	{"segfault", no_argument, 0, 's'},
	{"catch", no_argument, 0, 'c'},
	{0, 0, 0, 0} //last element of long_options array must contain all 0's (end)
};

int main(int argc, char* argv[])
{
	int c; //stores returned character from getopt call
	int option_index; //stores index of found option
	
	//declare flags for option parsing	
	int FL_input = 0;
	int FL_output = 0;
	int FL_segfault = 0;
	int FL_catch = 0;
 	
	//declare holder variables for option arguments
	char* input_filepath = NULL;
	char* output_filepath = NULL;
	
	//indicate getopt_long to not auto-output error messages
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
				fprintf(stderr,"Specified option \"%s\" not recognized.\nusage: %s [--input=filepath] [--output=filepath] [--segfault] [--catch]\n\t--input=filename: use the specified file as standard input\n\t--output=filename: create the specified file and use it as standard output\n\t--segfault: force a segmentation fault\n\t--catch: catch segmentation fault forced by --segfault\n\n", argv[optind-1], argv[0]);
				exit(1);
			case 'i':
				FL_input = 1;
				input_filepath = optarg;
				break;			
			case 'o':
				FL_output = 1;
				output_filepath = optarg;
				break;
			case 's':
				FL_segfault = 1;
				break;		
			case 'c':
				FL_catch = 1;
				break;
		}
	}

	//check if non-option arguments were specified (not supported) 
	if (argc-optind != 0)
	{
		fprintf(stderr,"Non-option argument '%s' is not supported.\nusage: %s [--input=filepath] [--output=filepath] [--segfault] [--catch]\n\t--input=filename: use the specified file as standard input\n\t--output=filename: create the specified file and use it as standard output\n\t--segfault: force a segmentation fault\n\t--catch: catch segmentation fault forced by --segfault\n\n", argv[optind],argv[0]);
		exit(1);	
	}	
	
	//redirect input
	if (FL_input)
	{	
		int fd_input = open(input_filepath, O_RDONLY);
		if (fd_input == -1)
		{
			//error has occured, check errno
			switch (errno)
			{
				case EACCES:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because of inability to request access to file, or search permission denied for one of the directories in the file's path, or file did not exist yet and write access is not allowed in parent directory.\n", input_filepath);
					break;
				case EFAULT:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because file's path points to outside your accessible address space.\n", input_filepath);
					break;
				case EINTR:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because open call interrupted by a signal handler while blocked waiting to complete an open of a slow device.\n", input_filepath);
					break;
				case ELOOP:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because too many symbolic links were encountered in resolving the file's path.\n", input_filepath);
					break;
				case EMFILE:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because the per-process limit on the number of open file descriptors has been reached.\n", input_filepath);
					break;
				case ENFILE:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because the system-wide limit on the total number of open files has been reached.\n", input_filepath);
					break;
				case ENAMETOOLONG:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because file's path was too long.\n", input_filepath);
					break;
				case ENODEV:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because file's path refers to a device-special file and no corresponding device exists.\n", input_filepath);
					break;
				case ENOENT:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because file does not exist or a directory component in the file's path does not exist or is a dangling symbolic link.\n", input_filepath);
					break;
				case ENOMEM:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because either the file is a FIFO, but memory for the FIFO buffer can't be allocated because the per-user hard limit on memory allocation for pipes has been reached and the caller is not privileged or insufficient kernel memory was available.\n", input_filepath);
					break;
				case ENOTDIR:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because a component used as a directory in the file's path is not a directory.\n", input_filepath);
					break;
				case ENXIO:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because the file is a device-special file and no corresponding device exists or file is a UNIX domain socket.\n", input_filepath);
					break;
				case EOVERFLOW:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because file's path refers to a regular file that is too large to be opened.\n", input_filepath);
					break;
				case EPERM:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed because the operation was prevented by a file seal.\n", input_filepath);
					break;
				default:
					fprintf(stderr,"Attempted open of specified file '%s' with option '--input' failed with an unrecognized error.\n", input_filepath);
					break;
			}

			exit(2);
		}

		//redirect file to stdin
		close(0);
		dup(fd_input);
		close(fd_input);	
	}

	//redirect output
	if (FL_output)
	{
		int fd_output = open(output_filepath, O_WRONLY|O_CREAT|O_TRUNC, 0666);
		if (fd_output == -1)
		{
			//error has occured, check errno
			switch (errno)
            		{	
                case EACCES:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because of inability to request access to file, or search permission denied for one of the directories in the file's path, or file did not exist yet and write access is not allowed in parent directory.\n", output_filepath);
					break;
                case EFAULT:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because file's path points to outside your accessible address space.\n", output_filepath);
					break;
                case EINTR:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because open call interrupted by a signal handler while blocked waiting to complete an open of a slow device.\n", output_filepath);
					break;
                case ELOOP:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because too many symbolic links were encountered in resolving the file's path.\n", output_filepath);
					break;
                case EMFILE:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because the per-process limit on the number of open file descriptors has been reached.\n", output_filepath);
					break;
                case ENFILE:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because the system-wide limit on the total number of open files has been reached.\n", output_filepath);
					break;
                case ENAMETOOLONG:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because file's path was too long.\n", output_filepath);
					break;
                case ENODEV:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because file's path refers to a device-special file and no corresponding device exists.\n", output_filepath);
					break;
                case ENOENT:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because a directory component in the file's path does not exist or is a dangling symbolic link.\n", output_filepath);
					break;
                case ENOMEM:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because either the file is a FIFO, but memory for the FIFO buffer can't be allocated because the per-user hard limit on memory allocation for pipes has been reached and the caller is not privileged or insufficient kernel memory was available.\n", output_filepath);
					break;
                case ENOTDIR:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because a component used as a directory in the file's path is not a directory.\n", output_filepath);
					break;
                case ENXIO:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because the file is a device-special file and no corresponding device exists or file is a UNIX domain socket.\n", output_filepath);
					break;
                case EOVERFLOW:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because file's path refers to a regular file that is too large to be opened.\n", output_filepath);
					break;
                case EPERM:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed because the operation was prevented by a file seal.\n", output_filepath);
					break;
                default:
                    fprintf(stderr,"Attempted open of specified file '%s' with option '--output' failed with an unrecognized error.\n", output_filepath);
					break;
            }

			exit(3);
		}

		//redirect stdout to file
		close(1);
		dup(fd_output);
		close(fd_output);
	}

	//register SIGSEGV handler
	if (FL_catch)
		signal(SIGSEGV,sighandler);

	//cause segfault
	if (FL_segfault)
		causeSegfault();

	//character for use by read/write operations
	char k;
	int numbytes;

	for(;;)
	{	
		//read byte from input
		numbytes = read(0,&k,1);
		
		//check for error
		if (numbytes == -1)
		{
			switch (errno)
			{
				case EINTR:
					fprintf(stderr,"Attempted read of specified file '%s' with option '--input' failed because call was interrupted by a signal before any data was read.\n", input_filepath);
					break;
				case EIO:
					fprintf(stderr,"Attempted read of specified file '%s' with option '--input' failed because of an I/O error.\n", input_filepath);
					break;
				default:
					fprintf(stderr,"Attempted read of specified file '%s' with option '--input' failed with an unrecognized error.\n", input_filepath);
					break;
			}

			exit(10);
		}

		//check for EOF
		if (numbytes == 0)
			break;

		//write byte to output
		numbytes = write(1,&k,1);
		
		//check for error
		if (numbytes == -1)
		{
			switch (errno)
			{
				case EDQUOT:
					fprintf(stderr,"Attempted write of specified file '%s' with option '--output' failed because user's quote of disk blocks on the filesystem containing stdin has been exhausted.\n", output_filepath);
					break;
				case EFBIG:
					fprintf(stderr,"Attempted write of specified file '%s' with option '--output' failed because an attempt was made to write a file that exceeds the implementation-defined max file size or the process's file size limit, or to write as a position past the max allowed offset.\n", output_filepath);
					break;
				case EINTR:
                    fprintf(stderr,"Attempted write of specified file '%s' with option '--output' failed because call was interrupted by a signal before any data was written.\n", output_filepath);
					break;
				case EIO:
					fprintf(stderr,"Attempted write of specified file '%s' with option '--output' failed with a low-level I/O error while modifying the inode.\n", output_filepath);
					break;
				case EPERM:
					fprintf(stderr,"Attempted write of specified file '%s' with option '--output' failed because operation was prevented by a file seal.\n", output_filepath);
					break;
				default:
					fprintf(stderr,"Attempted write of specified file '%s' with option '--output' failed with an unrecognized error.\n", output_filepath);	
					break;
			}

			exit(11);
		}
	}
	
	exit(0);
}

void causeSegfault()
{
	char* ptr = NULL;
	*ptr = 0;
}

void sighandler(int signalnum)
{
	fprintf(stderr,"Segmentation fault of signal number %d caught. Exiting process with return code 4...\n", signalnum);
	exit(4);	
}
