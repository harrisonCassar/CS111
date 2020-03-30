#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int FL_log = 1;
int fd_logfile;

int main()
{
	if (FL_log)
		fd_logfile = open("logfile.txt",O_TRUNC|O_CREAT|O_WRONLY,O_RDWR);

	if (write(fd_logfile,"HELLO\n",6) == -1)
	{
		fprintf(stderr,"ERROR!\n");
	}

	return 0;
}