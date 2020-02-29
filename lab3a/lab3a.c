//NAME: Harrison Cassar
//EMAIL: Harrison.Cassar@gmail.com
//ID: 505114980

//headers
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ext2_fs.h"

//macros
#define BOOT_SIZE 1024
#define BLOCK_SIZE 1024

//auxiliary functions
char* getFilepathFromArguments(int argc, char** argv);
int protected_pread(int fd, void* buf, size_t size, off_t offset);
int protected_open(const char* filepath, int flags);
void protected_close(int fd);

int main(int argc, char** argv)
{
	//get filepath and open file
	char* fspath = getFilepathFromArguments(argc,argv);
	int fs = protected_open(fspath,O_RDONLY);

	/* SUPERBLOCK Summary: 
	
	SUPERBLOCK
	total number of blocks (decimal)
	total number of i-nodes (decimal)
	block size (in bytes, decimal)
	i-node size (in bytes, decimal)
	blocks per group (decimal)
	i-nodes per group (decimal)
	first non-reserved i-node (decimal)
	*/
	
	struct ext2_super_block sb;
	protected_pread(fs,&sb,sizeof(struct ext2_super_block),BOOT_SIZE);
	fprintf(stdout,"SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",sb.s_blocks_count,sb.s_inodes_count,1024 << sb.s_log_block_size,sb.s_inode_size,sb.s_blocks_per_group,sb.s_inodes_per_group,sb.s_first_ino);

	/* GROUP Summary:

	GROUP
	group number (decimal, starting from zero)
	total number of blocks in this group (decimal)
	total number of i-nodes in this group (decimal)
	number of free blocks (decimal)
	number of free i-nodes (decimal)
	block number of free block bitmap for this group (decimal)
	block number of free i-node bitmap for this group (decimal)
	block number of first block of i-nodes in this group (decimal)

	ASSUMPTION: Given the assumption that the only input to this exectuable (per the spec) will be filesystems of one block group.
	Therefore, # blocks in the group is just equal to the number of blocks of the whole system (from SUPERBLOCK).
	*/

	struct ext2_group_desc gdt;
	protected_pread(fs,&gdt,sizeof(struct ext2_group_desc),BOOT_SIZE+BLOCK_SIZE);
	fprintf(stdout,"GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n",0,sb.s_blocks_count,sb.s_inodes_count,gdt.bg_free_blocks_count,gdt.bg_free_inodes_count,gdt.bg_block_bitmap,gdt.bg_inode_bitmap,gdt.bg_inode_table);

	/* FREE BLOCK ENTRIES Summary:
	Scan the free block bitmap for the group. For each free block, produce a new-line terminated line, with two comma-separated fields (with no white space).

	BFREE
	number of the free block (decimal)
	*/

	unsigned char buf;
	unsigned char mask;
	unsigned int blockcount = 0;
	int blockbitmap_offset = gdt.bg_block_bitmap*BLOCK_SIZE;

	//read one byte from FREE BLOCK BITMAP at a time, iterate through bits of that byte
	for (int i = 0; i < BLOCK_SIZE; i++)
	{
		mask = 1;
		protected_pread(fs,&buf,1,blockbitmap_offset+i);
		
		//iterate through bits of read byte, checking if block represented by bit is free
		for (int j = 0; j < 8; j++)
		{
			if (blockcount == sb.s_blocks_count)
				break;

			if ((buf & mask) == 0)
				fprintf(stdout,"BFREE,%d\n",blockcount+1);

			mask = mask << 1;
			blockcount++;
		}

		if (blockcount == sb.s_blocks_count)
			break;
	}

	/* FREE INODE ENTRIES Summary:
	Scan the free I-node bitmap for each group. For each free I-node, produce a new-line terminated line, with two comma-separated fields (with no white space).

	IFREE
	number of the free I-node (decimal)
	*/

	unsigned int inodecount = 0;
	int inodebitmap_offset = gdt.bg_inode_bitmap*BLOCK_SIZE;

	//read one byte from FREE BLOCK BITMAP at a time, iterate through bits of that byte
	for (int i = 0; i < BLOCK_SIZE; i++)
	{
		mask = 1;
		protected_pread(fs,&buf,1,inodebitmap_offset+i);
		
		//iterate through bits of read byte, checking if block represented by bit is free
		for (int j = 0; j < 8; j++)
		{
			if (inodecount == sb.s_inodes_count)
				break;

			if ((buf & mask) == 0)
				fprintf(stdout,"IFREE,%d\n",inodecount+1);

			mask = mask << 1;
			inodecount++;
		}

		if (inodecount == sb.s_inodes_count)
			break;
	}

	/* INODE Summary:
	Scan the I-nodes for each group. For each allocated (non-zero mode and non-zero link count) I-node, produce a new-line terminated line, with up to 27 comma-separated fields (with no white space). The first twelve fields are i-node attributes:	

	*/

	exit(0);
}

char* getFilepathFromArguments(int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr,"Incorrect number of arguments. Must only specify filepath to filesystem.\n");
		exit(1);
	}

	return argv[1];
}

int protected_pread(int fd, void* buf, size_t size, off_t offset)
{
	int bytesRead = pread(fd,buf,size,offset);

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

int protected_open(const char* filepath, int flags)
{
	int fd = open(filepath, flags);
	if (fd == -1)
	{
		//error has occured, check errno
		switch (errno)
		{
			case EACCES:
				fprintf(stderr,"Attempted open of specified file '%s' failed because of inability to request access to file, or search permission denied for one of the directories in the file's path, or file did not exist yet and write access is not allowed in parent directory.\n", filepath);
				break;
			case EFAULT:
				fprintf(stderr,"Attempted open of specified file '%s' failed because file's path points to outside your accessible address space.\n", filepath);
				break;
			case EINTR:
				fprintf(stderr,"Attempted open of specified file '%s' failed because open call interrupted by a signal handler while blocked waiting to complete an open of a slow device.\n", filepath);
				break;
			case ELOOP:
				fprintf(stderr,"Attempted open of specified file '%s' failed because too many symbolic links were encountered in resolving the file's path.\n", filepath);
				break;
			case EMFILE:
				fprintf(stderr,"Attempted open of specified file '%s' failed because the per-process limit on the number of open file descriptors has been reached.\n", filepath);
				break;
			case ENFILE:
				fprintf(stderr,"Attempted open of specified file '%s' failed because the system-wide limit on the total number of open files has been reached.\n", filepath);
				break;
			case ENAMETOOLONG:
				fprintf(stderr,"Attempted open of specified file '%s' failed because file's path was too long.\n", filepath);
				break;
			case ENODEV:
				fprintf(stderr,"Attempted open of specified file '%s' failed because file's path refers to a device-special file and no corresponding device exists.\n", filepath);
				break;
			case ENOENT:
				fprintf(stderr,"Attempted open of specified file '%s' failed because file does not exist or a directory component in the file's path does not exist or is a dangling symbolic link.\n", filepath);
				break;
			case ENOMEM:
				fprintf(stderr,"Attempted open of specified file '%s' failed because either the file is a FIFO, but memory for the FIFO buffer can't be allocated because the per-user hard limit on memory allocation for pipes has been reached and the caller is not privileged or insufficient kernel memory was available.\n", filepath);
				break;
			case ENOTDIR:
				fprintf(stderr,"Attempted open of specified file '%s' failed because a component used as a directory in the file's path is not a directory.\n", filepath);
				break;
			case ENXIO:
				fprintf(stderr,"Attempted open of specified file '%s' failed because the file is a device-special file and no corresponding device exists or file is a UNIX domain socket.\n", filepath);
				break;
			case EOVERFLOW:
				fprintf(stderr,"Attempted open of specified file '%s' failed because file's path refers to a regular file that is too large to be opened.\n", filepath);
				break;
			case EPERM:
				fprintf(stderr,"Attempted open of specified file '%s' failed because the operation was prevented by a file seal.\n", filepath);
				break;
			default:
				fprintf(stderr,"Attempted open of specified file '%s' failed with an unrecognized error.\n", filepath);
				break;
		}

		exit(1);
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
		
		exit(1);
	}
}