#NAME: Harrison Cassar
#EMAIL: Harrison.Cassar@gmail.com
#ID: 505114980

#import libraries
import csv
import sys

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
		self.dir_ptrs = [int(report[12]),int(report[13]),int(report[14]),int(report[15]),int(report[16]),int(report[17]),int(report[18]),int(report[19]),int(report[20]),int(report[21]),int(report[22]),int(report[23])] #add in a buffer for the 12 direct block ptrs
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
def contains_inode(list_of_inodes,num):
	for inode in list_of_inodes:
		if (inode.inode_num == num):
			return True
	return False

def contains_reference(list_of_block_references,num):
	if (num in list_of_block_references):
		return True
	return False

#instantiate "global" data structures
sb = -1
gdt = -1
free_blocks_nums = []
free_inodes_nums = []
directories = {}
allocated_inodes = []
referenced = {}
indirect_blocks = {}
recorded_link_counter = {}
parent_record = {}

#check command-line arguments
if (len(sys.argv) != 2):
	sys.stderr.write("Incorrect number of arguments specified. usage: lab3b report_csv_filename" + "\n")
	exit(1)

try:
	report_file = open(sys.argv[1], 'r')
except IOError:
	sys.stderr.write("ERROR: Cannot open file." + "\n")
	exit(1)

inconsistencyFound = False

#parse CSV file
output = csv.reader(report_file)
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
		if (int(r[1]) not in directories.keys()):
			directories[int(r[1])] = []
		directories[int(r[1])].append(Dirent(r))
	elif (r[0] == 'INODE'):
		allocated_inodes.append(Inode(r))
	elif (r[0] == 'INDIRECT'):
		#add/append indirect block report to data structure
		if (int(r[1]) not in indirect_blocks.keys()):
			indirect_blocks[int(r[1])] = []

		indirect_blocks[int(r[1])].append(Indirect(r))

		#check for duplicates when adding indirect blocks to referenced data structure
		if (int(r[5]) in referenced):
			existingblocktype = ""
			blocktype = ""

			if (referenced[int(r[5])][2] == 0):
				existingblocktype = ""
			elif (referenced[int(r[5])][2] == 1):
				existingblocktype = "INDIRECT "
			elif (referenced[int(r[5])][2] == 2):
				existingblocktype = "DOUBLE INDIRECT "
			elif (referenced[int(r[5])][2] == 3):
				existingblocktype = "TRIPLE INDIRECT "

			if (int(r[2]) == 0):
				blocktype = ""
			elif (int(r[2]) == 1):
				blocktype = "INDIRECT "
			elif (int(r[2]) == 2):
				blocktype = "DOUBLE INDIRECT "
			elif (int(r[2]) == 3):
				blocktype = "TRIPLE INDIRECT "

			existingblocktype += "BLOCK "
			blocktype += "BLOCK "

			print("DUPLICATE " + existingblocktype + str(r[5]) + " IN INODE " + str(referenced[int(r[5])][0]) + " AT OFFSET " + str(referenced[int(r[5])][1]))
			print("DUPLICATE " + blocktype + str(r[5]) + " IN INODE " + str(r[1]) + " AT OFFSET " + str(r[3]))
			inconsistencyFound = True
		else:
			referenced[int(r[5])] = [int(r[1]),int(r[3]),int(r[2])]
				
#inode allocation audits
for i in range(1,sb.inodes_count+1): #allocated_inodes
	onFreeList_inode = bool(i in free_inodes_nums)
	isAllocated = bool(contains_inode(allocated_inodes,i))
	if (onFreeList_inode and isAllocated):
		print("ALLOCATED INODE " + str(i) + " ON FREELIST")
		inconsistencyFound = True
	elif ((not onFreeList_inode) and (not isAllocated)):
		if (i > 0 and i < 11): #we are not asked to consider the inodes reserved for the system (1 to 10) as an error if they are not on free list/not allocated
			continue
		print("UNALLOCATED INODE " + str(i) + " NOT ON FREELIST")
		inconsistencyFound = True

#directory consistency audits
for inode in allocated_inodes: #generate record of parents for all inodes of directories
	if (inode.file_type != 'd'):
		continue

	for entry in directories[inode.inode_num]:
		if (entry.name != "'.'" and entry.name != "'..'"):
			parent_record[entry.inode_num] = inode.inode_num

for i in directories: #generate recorded link counts, and check for other errors
	directory_entries = directories[i]

	for entry in directory_entries:

		#increment link counter for inode of the file referred to by the entry
		if (entry.inode_num not in recorded_link_counter.keys()):
			recorded_link_counter[entry.inode_num] = 1
		else:
			recorded_link_counter[entry.inode_num] += 1

		#check for reference to invalid/unallocated inode
		isAllocated = bool(contains_inode(allocated_inodes,entry.inode_num))
		if (entry.inode_num < 1 or entry.inode_num > sb.inodes_count):
			print("DIRECTORY INODE " + str(entry.parent_inode_num) + " NAME " + entry.name + " INVALID INODE " + str(entry.inode_num))
			inconsistencyFound = True
		elif (not isAllocated):
			print("DIRECTORY INODE " + str(entry.parent_inode_num) + " NAME " + entry.name + " UNALLOCATED INODE " + str(entry.inode_num))
			inconsistencyFound = True

		#check if entry is '.' or '..', and check if they refer to themselves/parent (with consideration of root directory entry)
		if (entry.name == "'.'" and entry.inode_num != entry.parent_inode_num):
			print("DIRECTORY INODE " + str(entry.parent_inode_num) + " NAME " + entry.name + " LINK TO INODE " + str(entry.inode_num) + " SHOULD BE " + str(entry.parent_inode_num))
			inconsistencyFound = True
		elif (entry.name == "'..'"):
			if (entry.parent_inode_num == 2 and entry.inode_num != entry.parent_inode_num): #check for root
				print("DIRECTORY INODE " + str(entry.parent_inode_num) + " NAME " + entry.name + " LINK TO INODE " + str(entry.inode_num) + " SHOULD BE " + str(entry.parent_inode_num))
				inconsistencyFound = True
			elif (entry.parent_inode_num != 2 and parent_record[entry.parent_inode_num] != entry.inode_num):
				print("DIRECTORY INODE " + str(entry.parent_inode_num) + " NAME " + entry.name + " LINK TO INODE " + str(entry.inode_num) + " SHOULD BE " + parent_record[entry.parent_inode_num])
				inconsistencyFound = True

for inode in allocated_inodes: #check link counts for lack of matching
	if (inode.inode_num not in recorded_link_counter.keys()):
		print("INODE " + str(inode.inode_num) + " HAS 0 LINKS BUT LINKCOUNT IS " + str(inode.link_count))
		inconsistencyFound = True
	elif (recorded_link_counter[inode.inode_num] != inode.link_count):
		print("INODE " + str(inode.inode_num) + " HAS " + str(recorded_link_counter[inode.inode_num]) + " LINKS BUT LINKCOUNT IS " + str(inode.link_count))
		inconsistencyFound = True

#block consistency audits
min_block_num = gdt.inode_table + (sb.inodes_per_group*sb.inode_size/sb.block_size)
max_block_num = sb.blocks_count - 1

for i in range(0,len(allocated_inodes)):
	inode = allocated_inodes[i]
	
	#check direct blocks
	for j in range(0,len(inode.dir_ptrs)):
		dir_ptr = inode.dir_ptrs[j]
		if (dir_ptr < 0 or dir_ptr > max_block_num):
			print("INVALID BLOCK " + str(dir_ptr) + " IN INODE " + str(inode.inode_num) + " AT OFFSET " + str(j))
			inconsistencyFound = True
		elif (dir_ptr != 0 and dir_ptr < min_block_num):
			print("RESERVED BLOCK " + str(dir_ptr) + " IN INODE " + str(inode.inode_num) + " AT OFFSET " + str(j))
			inconsistencyFound = True
		elif (dir_ptr != 0): #add to referenced list/dictionary, checking if duplicate; output if so (key is block num, value is list of 3 items: block inode #, offset value, indirection level)
			if (dir_ptr in referenced):
				blocktype = ""
				if (referenced[dir_ptr][2] == 0):
					blocktype = ""
				elif (referenced[dir_ptr][2] == 1):
					blocktype = "INDIRECT "
				elif (referenced[dir_ptr][2] == 2):
					blocktype = "DOUBLE INDIRECT "
				elif (referenced[dir_ptr][2] == 3):
					blocktype = "TRIPLE INDIRECT "

				blocktype += "BLOCK "
				print("DUPLICATE " + blocktype + str(dir_ptr) + " IN INODE " + str(referenced[dir_ptr][0]) + " AT OFFSET " + str(referenced[dir_ptr][1]))
				print("DUPLICATE BLOCK " + str(dir_ptr) + " IN INODE " + str(inode.inode_num) + " AT OFFSET " + str(j))
				inconsistencyFound = True
			else:
				referenced[dir_ptr] = [inode.inode_num,j,0]
			
	#check indirect block, and its respective direct block ptrs
	if (inode.ind_ptr < 0 or inode.ind_ptr > max_block_num):
		print("INVALID INDIRECT BLOCK " + str(inode.ind_ptr) + " IN INODE " + str(inode.inode_num) + " AT OFFSET " + str(len(inode.dir_ptrs)))
		inconsistencyFound = True
	elif (inode.ind_ptr != 0 and inode.ind_ptr < min_block_num):
		print("RESERVED INDIRECT BLOCK " + str(inode.ind_ptr) + " IN INODE " + str(inode.inode_num) + " AT OFFSET " + str(len(inode.dir_ptrs)))
		inconsistencyFound = True
	elif (inode.ind_ptr != 0):
		if (inode.ind_ptr in referenced):
			blocktype = ""
			if (referenced[inode.ind_ptr][2] == 0):
				blocktype = ""
			elif (referenced[inode.ind_ptr][2] == 1):
				blocktype = "INDIRECT "
			elif (referenced[inode.ind_ptr][2] == 2):
				blocktype = "DOUBLE INDIRECT "
			elif (referenced[inode.ind_ptr][2] == 3):
				blocktype = "TRIPLE INDIRECT "

			blocktype += "BLOCK "
			print("DUPLICATE " + blocktype + str(inode.ind_ptr) + " IN INODE " + str(referenced[inode.ind_ptr][0]) + " AT OFFSET " + str(referenced[inode.ind_ptr][1]))
			print("DUPLICATE INDIRECT BLOCK " + str(inode.ind_ptr) + " IN INODE " + str(inode.inode_num) + " AT OFFSET " + str(len(inode.dir_ptrs)))
			inconsistencyFound = True
		else:
			referenced[inode.ind_ptr] = [inode.inode_num,len(inode.dir_ptrs),1]

	#for k in range(0,sb.block_size/4): 
	#for all referenced indirect blocks connected to inode m.. actually dont have to check for duplicates again, as did so on addition in CSV parsing

	#check double indirect block, its respective single indirect blocks, and their respective direct block ptrs
	if (inode.dind_ptr < 0 or inode.dind_ptr > max_block_num):
		print("INVALID DOUBLE INDIRECT BLOCK " + str(inode.dind_ptr) + " IN INODE " + str(inode.inode_num) + " AT OFFSET " + str(len(inode.dir_ptrs) + sb.block_size/4))
		inconsistencyFound = True
	elif (inode.dind_ptr != 0 and inode.dind_ptr < min_block_num):
		print("RESERVED DOUBLE INDIRECT BLOCK " + str(inode.dind_ptr) + " IN INODE " + str(inode.inode_num) + " AT OFFSET " + str(len(inode.dir_ptrs) + sb.block_size/4))
		inconsistencyFound = True
	elif (inode.dind_ptr != 0):
		if (inode.dind_ptr in referenced):
			blocktype = ""
			if (referenced[inode.dind_ptr][2] == 0):
				blocktype = ""
			elif (referenced[inode.dind_ptr][2] == 1):
				blocktype = "INDIRECT "
			elif (referenced[inode.dind_ptr][2] == 2):
				blocktype = "DOUBLE INDIRECT "
			elif (referenced[inode.dind_ptr][2] == 3):
				blocktype = "TRIPLE INDIRECT "

			blocktype += "BLOCK "
			print("DUPLICATE " + blocktype + str(inode.dind_ptr) + " IN INODE " + str(referenced[inode.dind_ptr][0]) + " AT OFFSET " + str(referenced[inode.dind_ptr][1]))
			print("DUPLICATE DOUBLE INDIRECT BLOCK " + str(inode.dind_ptr) + " IN INODE " + str(inode.inode_num) + " AT OFFSET " + str(len(inode.dir_ptrs) + sb.block_size/4))
			inconsistencyFound = True
		else:
			referenced[inode.dind_ptr] = [inode.inode_num,len(inode.dir_ptrs) + sb.block_size/4,2]
	
	#for all referenced indirect blocks connected to inode m.. actually dont have to check for duplicates again, as did so on addition in CSV parsing

	#check triple indirect block, its respective double indirect blocks, their respective single indirect blocks, and their respective direct blocks
	if (inode.tind_ptr < 0 or inode.tind_ptr > max_block_num):
		print("INVALID TRIPLE INDIRECT BLOCK " + str(inode.tind_ptr) + " IN INODE " + str(inode.inode_num) + " AT OFFSET " + str(len(inode.dir_ptrs) + ((sb.block_size/4)+1)*(sb.block_size/4)))
		inconsistencyFound = True
	elif (inode.tind_ptr != 0 and inode.tind_ptr < min_block_num):
		print("RESERVED TRIPLE INDIRECT BLOCK " + str(inode.tind_ptr) + " IN INODE " + str(inode.inode_num) + " AT OFFSET " + str(len(inode.dir_ptrs) + ((sb.block_size/4)+1)*(sb.block_size/4)))
		inconsistencyFound = True
	elif (inode.tind_ptr != 0):
		if (inode.tind_ptr in referenced):
			blocktype = ""
			if (referenced[inode.tind_ptr][2] == 0):
				blocktype = ""
			elif (referenced[inode.tind_ptr][2] == 1):
				blocktype = "INDIRECT "
			elif (referenced[inode.tind_ptr][2] == 2):
				blocktype = "DOUBLE INDIRECT "
			elif (referenced[inode.tind_ptr][2] == 3):
				blocktype = "TRIPLE INDIRECT "

			blocktype += "BLOCK "
			print("DUPLICATE " + blocktype + str(inode.tind_ptr) + " IN INODE " + str(referenced[inode.tind_ptr][0]) + " AT OFFSET " + str(referenced[inode.tind_ptr][1]))
			print("DUPLICATE TRIPLE INDIRECT BLOCK " + str(inode.tind_ptr) + " IN INODE " + str(inode.inode_num) + " AT OFFSET " + str(len(inode.dir_ptrs) + ((sb.block_size/4)+1)*(sb.block_size/4)))
			inconsistencyFound = True
		else:
			referenced[inode.tind_ptr] = [inode.inode_num,len(inode.dir_ptrs) + ((sb.block_size/4)+1)*(sb.block_size/4),3]
	
	#for all referenced indirect blocks connected to inode m.. actually dont have to check for duplicates again, as did so on addition in CSV parsing

#at this point, have scanned all blocks referenced, and all should have been added to referenced list
#check for other block errors (unreferenced but not on free list, or referenced but on free list)
for i in range(gdt.inode_table+((sb.inode_size*sb.inodes_per_group)/sb.block_size),sb.blocks_count): #num of blocks
	onFreeList_block = bool(i in free_blocks_nums)
	isReferenced = bool(contains_reference(referenced,i))

	if (onFreeList_block and isReferenced):
		print("ALLOCATED BLOCK " + str(i) + " ON FREELIST")
		inconsistencyFound = True
	elif ((not onFreeList_block) and (not isReferenced)):
		print("UNREFERENCED BLOCK " + str(i))
		inconsistencyFound = True


#prepare for exit
report_file.close()

if (inconsistencyFound):
	exit(2)
else:
	exit(0)