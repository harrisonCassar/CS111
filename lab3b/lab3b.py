import csv

class Superblock:
	def __init__(self):
		self.blocks_count = -1
		self.inodes_count = -1
		self.block_size = -1
		self.inode_size = -1
		self.blocks_per_group = -1
		self.inodes_per_group = -1
		self.first_non_reserved_inode = -1

class Group:
	def __init__(self):
		self.group_num = -1
		self.num_blocks_in_group = -1
		self.num_inodes_in_group = -1
		self.num_free_blocks = -1
		self.num_free_inodes = -1
		self.block_bitmap = -1
		self.inode_bitmap = -1
		self.inode_table = -1
	
class Bfree:
	def __init__(self):
		self.block_num = -1

class Ifree:
	def __init__(self):
		self.inode_num = -1

class Dirent:
	def __init__(self):
		self.parent_inode_num = -1
		self.logical_byte_offset = -1
		self.inode_num = -1
		self.entry_length = -1
		self.name_length = -1
		self.name = -1

class Inode:
	def __init__(self):
		self.inode_num = -1
		self.file_type = -1
		self.mode = -1
		self.owner = -1
		self.group = -1
		self.link_count = -1
		self.inode_change_time = -1 #change time of inode
		self.inode_modification_time = -1 #modification time of file
		self.inode_access_time = -1
		self.file_size = -1
		self.num_of_blocks = -1
		#add in essentially buffers for the 12 direct block ptrs
		self.ind_ptr = -1
		self.dind_ptr = -1
		self.tind_ptr = -1

with open('samples/P3B-test_1.csv', 'r') as f:
    reader = csv.reader(f)
    for row in reader:
        print(row)