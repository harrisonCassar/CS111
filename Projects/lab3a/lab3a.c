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
#include <time.h>
#include "ext2_fs.h"

//macros
#define BOOT_SIZE 1024
#define BLOCK_SIZE 1024
#define EXT2_S_IFREG 0x8000
#define EXT2_S_IFDIR 0x4000
#define EXT2_S_IFLNK 0xA000

//auxiliary functions
char* getFilepathFromArguments(int argc, char** argv);
int protected_pread(int fd, void* buf, size_t size, off_t offset);
int protected_open(const char* filepath, int flags);
void protected_close(int fd);
void formatTime(char* buf, int size, time_t time);

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

	INODE
	inode number (decimal)
	file type ('f' for file, 'd' for directory, 's' for symbolic link, '?" for anything else)
	mode (low order 12-bits, octal ... suggested format "%o")
	owner (decimal)
	group (decimal)
	link count (decimal)
	time of last I-node change (mm/dd/yy hh:mm:ss, GMT) //NOTE: According to sanity test, ", GMT" should not be included in the output (confirmed by TA, despite the spec)
	modification time (mm/dd/yy hh:mm:ss, GMT) //NOTE: According to sanity test, ", GMT" should not be included in the output (confirmed by TA, despite the spec)
	time of last access (mm/dd/yy hh:mm:ss, GMT) //NOTE: According to sanity test, ", GMT" should not be included in the output (confirmed by TA, despite the spec)
	file size (decimal)
	number of (512 byte) blocks of disk space (decimal) taken up by this file
	(next 15 are block addresses; decimal, 12 direct, one indirect, one double indirect, one triple indirect; if file length is less than the size of the block ptrs, 60 bytes, the file will contain zero data blocks, and the name is stored in the space normally occupied by the block ptrs; in this case, do not print out block ptrs)
	
	For directory entries, additionally:
	For each directory I-node, scan every data block. For each valid (non-zero I-node number) directory entry, produce a new-line terminated line, with seven comma-separated fields (no white space).
	
	DIRENT
	parent inode number (decimal) ... the I-node number of the directory that contains this entry
	logical byte offset (decimal) of this entry within the directory
	inode number of the referenced file (decimal)
	entry length (decimal)
	name length (decimal)
	name (string, surrounded by single-quotes). Don't worry about escaping, we promise there will be no single-quotes or commas in any of the file names.
	
	For indirect block references, additionally:
	For each file or directory I-node, scan the single indirect blocks and (recursively) the double and triple indirect blocks. For each non-zero block pointer you find, produce a new-line terminated line with six comma-separated fields (no white space).

	INDIRECT
	I-node number of the owning file (decimal)
	(decimal) level of indirection for the block being scanned ... 1 for single indirect, 2 for double indirect, 3 for triple
	logical block offset (decimal) represented by the referenced block. If the referenced block is a data block, this is the logical block offset of that block within the file. If the referenced block is a single- or double-indirect block, this is the same as the logical offset of the first data block to which it refers.
	block number of the (1, 2, 3) indirect block being scanned (decimal) . . . not the highest level block (in the recursive scan), but the lower level block that contains the block reference reported by this entry.
	block number of the referenced block (decimal)
	*/

	struct ext2_inode inode;
	int inodetable_offset = gdt.bg_inode_table*BLOCK_SIZE;

	for (unsigned int i = 0; i < sb.s_inodes_count; i++)
	{
		protected_pread(fs,&inode,sizeof(struct ext2_inode),inodetable_offset + i*sizeof(struct ext2_inode));

		if (inode.i_mode != 0 && inode.i_links_count != 0)
		{
			//determine file type
			char ftype = '?';
			if (inode.i_mode & EXT2_S_IFREG)
				ftype = 'f';
			else if (inode.i_mode & EXT2_S_IFDIR)
				ftype = 'd';
			else if (inode.i_mode & EXT2_S_IFLNK)
				ftype = 's';

			//construct time outputs
			char buf_inodeChange[25];
			char buf_inodeModification[25];
			char buf_inodeAccess[25];

			formatTime(buf_inodeChange,25,(time_t) inode.i_ctime); //ctime --> inode change
			formatTime(buf_inodeModification,25,(time_t) inode.i_mtime); //mtime --> modification time (of file)
			formatTime(buf_inodeAccess,25,(time_t) inode.i_atime); //atime --> access

			long filesize = (((long) inode.i_dir_acl) << 32) | ((long) inode.i_size);

			fprintf(stdout,"INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%ld,%d",i+1,ftype,inode.i_mode & 0x0FFF,inode.i_uid,inode.i_gid,inode.i_links_count,buf_inodeChange,buf_inodeModification,buf_inodeAccess,filesize,inode.i_blocks);
			
			//check if symbolic link and if no data blocks are occupied by file (size is less than block pointer size, meaning no more printing)
			if (ftype == 's' && filesize < 60)
				continue;

			for (int j = 0; j < EXT2_N_BLOCKS; j++)
				fprintf(stdout,",%d",inode.i_block[j]);

			fprintf(stdout,"\n");

			if (ftype == 'd')
			{
				struct ext2_dir_entry dirent;

				long sumlogicaloffset = 0;

				//direct blocks
				for (int k = 0; k < EXT2_NDIR_BLOCKS; k++)
				{	
					if (inode.i_block[k] != 0)
					{
						for (int block_offset = 0; block_offset < BLOCK_SIZE; )	
						{			
							protected_pread(fs,&dirent,sizeof(struct ext2_dir_entry),inode.i_block[k] * BLOCK_SIZE + block_offset);

							if (dirent.inode != 0)
								fprintf(stdout,"DIRENT,%d,%ld,%d,%d,%d,\'%s\'\n",i+1,(long) k*BLOCK_SIZE+block_offset,dirent.inode,dirent.rec_len,dirent.name_len,dirent.name);
						
							block_offset += dirent.rec_len;
						}
					}
				}

				sumlogicaloffset += EXT2_NDIR_BLOCKS*BLOCK_SIZE;
				
				__u32 block_ptr_buf[BLOCK_SIZE];
				__u32 indblock_ptr_buf[BLOCK_SIZE];
				__u32 dindblock_ptr_buf[BLOCK_SIZE];

				//indirect block
				if (inode.i_block[EXT2_IND_BLOCK] != 0)
				{
					protected_pread(fs,block_ptr_buf,BLOCK_SIZE,inode.i_block[EXT2_IND_BLOCK] * BLOCK_SIZE);
					
					//iterate through block ptrs stored at ind block
					for (int l = 0; l < BLOCK_SIZE/4; l++)
					{
						if (block_ptr_buf[l] != 0)
						{
							for (int block_offset = 0; block_offset < BLOCK_SIZE; )	
							{			
								protected_pread(fs,&dirent,sizeof(struct ext2_dir_entry),block_ptr_buf[l] * BLOCK_SIZE + block_offset);

								if (dirent.inode != 0)
									fprintf(stdout,"DIRENT,%d,%ld,%d,%d,%d,\'%s\'\n",i+1,(long) l*BLOCK_SIZE + sumlogicaloffset + block_offset,dirent.inode,dirent.rec_len,dirent.name_len,dirent.name);
							
								block_offset += dirent.rec_len;
							}
						}
					}
				}

				sumlogicaloffset += 1 * (BLOCK_SIZE/4) * BLOCK_SIZE; 

				//double indirect block
				if (inode.i_block[EXT2_DIND_BLOCK] != 0)
				{
					protected_pread(fs,indblock_ptr_buf,BLOCK_SIZE,inode.i_block[EXT2_DIND_BLOCK] * BLOCK_SIZE);
					
					//iterate through indblock ptrs stored at dind block
					for (int m = 0; m < BLOCK_SIZE/4; m++)
					{
						if (indblock_ptr_buf[m] != 0)
						{
							protected_pread(fs,block_ptr_buf,BLOCK_SIZE,indblock_ptr_buf[m] * BLOCK_SIZE);
							
							//iterate through block ptrs stored at ind block
							for (int n = 0; n < BLOCK_SIZE/4; n++)
							{
								if (block_ptr_buf[n] != 0)
								{
									for (int block_offset = 0; block_offset < BLOCK_SIZE; )	
									{			
										protected_pread(fs,&dirent,sizeof(struct ext2_dir_entry),block_ptr_buf[n] * BLOCK_SIZE + block_offset);

										if (dirent.inode != 0)
											fprintf(stdout,"DIRENT,%d,%ld,%d,%d,%d,\'%s\'\n",i+1,(long) n*BLOCK_SIZE + sumlogicaloffset + m*(BLOCK_SIZE/4)*BLOCK_SIZE + block_offset,dirent.inode,dirent.rec_len,dirent.name_len,dirent.name);
									
										block_offset += dirent.rec_len;
									}
								}
							}
						}
					}
				}

				sumlogicaloffset += 1 * (BLOCK_SIZE/4) * (BLOCK_SIZE/4) * BLOCK_SIZE;

				//triple indirect block
				if (inode.i_block[EXT2_TIND_BLOCK] != 0)
				{
					protected_pread(fs,dindblock_ptr_buf,BLOCK_SIZE,inode.i_block[EXT2_TIND_BLOCK] * BLOCK_SIZE);
					
					//iterate through dindblock ptrs stored at tind block
					for (int p = 0; p < BLOCK_SIZE/4; p++)
					{
						if (dindblock_ptr_buf[p] != 0)
						{
							protected_pread(fs,indblock_ptr_buf,BLOCK_SIZE,dindblock_ptr_buf[p] * BLOCK_SIZE);
							
							//iterate through indblock ptrs stored at dind block
							for (int q = 0; q < BLOCK_SIZE/4; q++)
							{
								if (indblock_ptr_buf[q] != 0)
								{
									protected_pread(fs,block_ptr_buf,BLOCK_SIZE,indblock_ptr_buf[q] * BLOCK_SIZE);
									
									//iterate through block ptrs stored at ind block
									for (int r = 0; r < BLOCK_SIZE/4; r++)
									{
										if (block_ptr_buf[r] != 0)
										{
											for (int block_offset = 0; block_offset < BLOCK_SIZE; )	
											{			
												protected_pread(fs,&dirent,sizeof(struct ext2_dir_entry),block_ptr_buf[r] * BLOCK_SIZE + block_offset);

												if (dirent.inode != 0)
													fprintf(stdout,"DIRENT,%d,%ld,%d,%d,%d,\'%s\'\n",i+1,(long) r*BLOCK_SIZE + sumlogicaloffset + q*(BLOCK_SIZE/4)*BLOCK_SIZE + p*(BLOCK_SIZE/4)*(BLOCK_SIZE/4)*BLOCK_SIZE + block_offset,dirent.inode,dirent.rec_len,dirent.name_len,dirent.name);
											
												block_offset += dirent.rec_len;
											}
										}
									}
								}
							}
						}
					}
				}
			}

			if (ftype == 'd' || ftype == 'f')
			{
				__u32 block_ptr_buf[BLOCK_SIZE];
				__u32 indblock_ptr_buf[BLOCK_SIZE];
				__u32 dindblock_ptr_buf[BLOCK_SIZE];

				//indirect block
				if (inode.i_block[EXT2_IND_BLOCK] != 0)
				{
					protected_pread(fs,block_ptr_buf,BLOCK_SIZE,inode.i_block[EXT2_IND_BLOCK] * BLOCK_SIZE);
					
					//iterate through block ptrs stored at ind block
					for (int t = 0; t < BLOCK_SIZE/4; t++)
					{
						if (block_ptr_buf[t] > 0)
							fprintf(stdout,"INDIRECT,%d,1,%ld,%d,%d\n",i+1,(long) EXT2_NDIR_BLOCKS+t,inode.i_block[EXT2_IND_BLOCK],block_ptr_buf[t]);
					}
				}

				//double indirect block
				if (inode.i_block[EXT2_DIND_BLOCK] != 0)
				{
					protected_pread(fs,indblock_ptr_buf,BLOCK_SIZE,inode.i_block[EXT2_DIND_BLOCK] * BLOCK_SIZE);
					
					//iterate through indblock ptrs stored at dind block
					for (int m = 0; m < BLOCK_SIZE/4; m++)
					{
						if (indblock_ptr_buf[m] > 0)
						{
							fprintf(stdout,"INDIRECT,%d,2,%ld,%d,%d\n",i+1,(long) EXT2_NDIR_BLOCKS+(m+1)*(BLOCK_SIZE/4),inode.i_block[EXT2_DIND_BLOCK],indblock_ptr_buf[m]);

							protected_pread(fs,block_ptr_buf,BLOCK_SIZE,indblock_ptr_buf[m] * BLOCK_SIZE);
							
							//iterate through block ptrs stored at ind block
							for (int t = 0; t < BLOCK_SIZE/4; t++)
							{
								if (block_ptr_buf[t] > 0)
									fprintf(stdout,"INDIRECT,%d,1,%ld,%d,%d\n",i+1,(long) EXT2_NDIR_BLOCKS+(m+1)*(BLOCK_SIZE/4) + t,indblock_ptr_buf[m],block_ptr_buf[t]);
							}
						}
					}
				}

				//triple indirect block
				if (inode.i_block[EXT2_TIND_BLOCK] != 0)
				{
					protected_pread(fs,dindblock_ptr_buf,BLOCK_SIZE,inode.i_block[EXT2_TIND_BLOCK] * BLOCK_SIZE);
					
					//iterate through dindblock ptrs stored at tind block
					for (int p = 0; p < BLOCK_SIZE/4; p++)
					{
						if (dindblock_ptr_buf[p] > 0)
						{
							fprintf(stdout,"INDIRECT,%d,3,%ld,%d,%d\n",i+1,(long) EXT2_NDIR_BLOCKS+(p+1)*(BLOCK_SIZE/4)*(BLOCK_SIZE/4) + BLOCK_SIZE/4,inode.i_block[EXT2_TIND_BLOCK],dindblock_ptr_buf[p]);

							protected_pread(fs,indblock_ptr_buf,BLOCK_SIZE,dindblock_ptr_buf[p] * BLOCK_SIZE);
							
							//iterate through indblock ptrs stored at dind block
							for (int q = 0; q < BLOCK_SIZE/4; q++)
							{
								if (indblock_ptr_buf[q] > 0)
								{
									fprintf(stdout,"INDIRECT,%d,2,%ld,%d,%d\n",i+1,(long) EXT2_NDIR_BLOCKS+(p+1)*(BLOCK_SIZE/4)*(BLOCK_SIZE/4) + (q+1)*BLOCK_SIZE/4,dindblock_ptr_buf[p],indblock_ptr_buf[q]);

									protected_pread(fs,block_ptr_buf,BLOCK_SIZE,indblock_ptr_buf[q] * BLOCK_SIZE);
									
									//iterate through block ptrs stored at ind block
									for (int t = 0; t < BLOCK_SIZE/4; t++)
									{
										if (block_ptr_buf[t] > 0)
											fprintf(stdout,"INDIRECT,%d,1,%ld,%d,%d\n",i+1,(long) EXT2_NDIR_BLOCKS+(p+1)*(BLOCK_SIZE/4)*(BLOCK_SIZE/4) + (q+1)*BLOCK_SIZE/4 + t,indblock_ptr_buf[q],block_ptr_buf[t]);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	exit(0);
}

void formatTime(char* buf, int size, time_t time)
{
	struct tm* tmp = gmtime(&time);
	strftime(buf,size,"%D %T", tmp);
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