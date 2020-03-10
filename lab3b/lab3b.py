#import libraries
import csv

#define auxiliary data structures
class Superblock:
	def __init__(self, report):
		self.blocks_count = int(report[1])
		self.inodes_count = int(report[2])
		self.block_size = int(report[3])
		self.inode_size = int(report[4])
		self.blocks_per_group = int(report[5])
		self.inodes_per_group = int(report[6])
		self.first_non_reserved_inode = int(report[7])

class Group:
	def __init__(self, report):
		self.group_num = int(report[1])
		self.num_blocks_in_group = int(report[2])
		self.num_inodes_in_group = int(report[3])
		self.num_free_blocks = int(report[4])
		self.num_free_inodes = int(report[5])
		self.block_bitmap = int(report[6])
		self.inode_bitmap = int(report[7])
		self.inode_table = int(report[8])
	
"""
class Bfree:
	def __init__(self, report):
		self.block_num = report[0]

class Ifree:
	def __init__(self, report):
		self.inode_num = report[0]
"""

class Inode:
	def __init__(self, report):
		self.inode_num = int(report[1])
		self.file_type = report[2]
		self.mode = int(report[3])
		self.owner = int(report[4])
		self.group = int(report[5])
		self.link_count = int(report[6])
		self.inode_change_time = report[7] #change time of inode
		self.inode_modification_time = report[8] #modification time of file
		self.inode_access_time = report[9]
		self.file_size = int(report[10])
		self.num_of_blocks = int(report[11])
		self.list = [int(report[12]),int(report[13]),int(report[14]),int(report[15]),int(report[16]),int(report[17]),int(report[18]),int(report[19]),int(report[20]),int(report[21]),int(report[22]),int(report[23])] #add in a buffer for the 12 direct block ptrs
		self.ind_ptr = int(report[24])
		self.dind_ptr = int(report[25])
		self.tind_ptr = int(report[26])

class Dirent:
	def __init__(self, report):
		self.parent_inode_num = int(report[1])
		self.logical_byte_offset = int(report[2])
		self.inode_num = int(report[3])
		self.entry_length = int(report[4])
		self.name_length = int(report[5])
		self.name = report[6]

class Indirect:
	def __init__(self, report):
		self.inode_num = int(report[1])
		self.indirection_level = int(report[2])
		self.logical_block_offset = int(report[3])
		self.block_num_of_containing_reference = int(report[4])
		self.block_num_of_referenced = int(report[5])

#define auxiliary functions
def contains(list_of_inodes,num):
	for inode in list_of_inodes:
		if (inode.inode_num == num):
			return True
	return False

#instantiate "global" data structures
sb = -1
gdt = -1
free_blocks_nums = []
free_inodes_nums = []
directory_entries = []
allocated_inodes = []

with open('samples/P3B-test_15.csv', 'r') as f:
	output = csv.reader(f)
	for r in output:
		if (r[0] == 'SUPERBLOCK'):
			sb = Superblock(r)
		elif (r[0] == 'GROUP'):
			gdt = Group(r)
		elif (r[0] == 'BFREE'):
			free_blocks_nums.append(int(r[1]))
		elif (r[0] == 'IFREE'):
			free_inodes_nums.append(int(r[1]))
		elif (r[0] == 'DIRENT'):
			directory_entries.append(Dirent(r))
		elif (r[0] == 'INODE'):
			allocated_inodes.append(Inode(r))
		elif (r[0] == 'INDIRECT'):
			print(r)

#inode allocation audits
for i in range(11,sb.inodes_count+1): #allocated_inodes
	onFreeList = bool(i in free_inodes_nums)
	isAllocated = bool(contains(allocated_inodes,i))

	if onFreeList and isAllocated:
		print("ALLOCATED INODE " + str(i) + " ON FREELIST")
	elif (not onFreeList) and (not isAllocated):
		print("UNALLOCATED INODE " + str(i) + " NOT ON FREELIST")

#block consistency audits

#inode allocation audits
