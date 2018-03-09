# ECS 150: Project 4 "File System"
### Partners: Joel Moens, Akshay Kumar
#### Due Date: Friday, March 9th, 2018

-------------------------------------------------------------------------------

## Program Implementation
### Phase 0: ECS150-FS Specification
We used the following data structures to implement Phase 0:

* struct __attribute__ ((__packed__)) superblock
* struct __attribute__ ((__packed__)) fatblock
* struct __attribute__ ((__packed__)) rootblock
* struct __attribute__ ((__packed__)) fsys

Superblock struct *superblock* stores pertinent information about our disk, it
is the table of contents for our file system. File Allocation Table is a
list of struct *fatblock* and it stores 2-byte *word* per data block which link
to one another if they are part of the same file. Our *Root directory* struct 
*rootblock* stores our 128 file descriptors (Table of Contents for files on
disk). Lastly, our *File System* struct fsys combines a *superblock*, list of
*fatblocks*, and *rootblock*. We ensure that no extra memory is allocated in
these structs by assigning them the __attribute__ ((__packed__)) 

### Phase 1: Mounting/Unmounting
We used the following data structures/functions to implement Phase 1:

* struct __attribute__ ((__packed__)) filedescriptor
* append usedata and 
* static struct fsys \* fs
* struct fsys \* fs_malloc()
* int fs_mount(const char \*diskname)
* int fs_umount(void)
* int fs_info(void)

In order to easily access a virtual disk that we would like to mount, we need 
to store only the first few blocks into our filesystem. We declare a global 
static fsys * fs that is used for the rest of the program. When we call 
*fs_mount* we open the given disk and proceed to store the *root*, *super*, and
*fat* list inside fs with *fs_malloc*. We read the superblock first because it
informs us how large the drive is, the number of fatblocks needed, as well as
other useful information that we use to malloc and read the correct blocks 
into fs. If any of the given disk api fails while allocating for fs, we return 
NULL, which returns -1 in fs_mount, denoting a failure to mount. For
*fs_umount* we write the updated fatblocks and updated rootblock back into
the virtual drive, close the disk, and free all memory used. *fs_info* simply 
returns certain pieces of information from the super block and filesystem.
We added an int *usedata* variable in our filesystem that keeps track of how
many fat words are in use, including the first word which is always FAT_EOC.
We also added an int *entries* that keeps track of the number of files on disk.
These two ints are used for printing free ratios for fat and root



### Phase 2: File Creation/Deletion
We used the following functions to implement Phase 2:

* int fs_findemptyfd()
* int fs_findfd(const char \*filename)
* int fs_create(const char \*filename)
* int fs_delete(const char \*filename)
* int fs_ls(void)

Our *fs_findemptyfd* loops through the length of our *Root directory*, 
searches for an empty entry by checking for a starting null terminating 
character, and returns the index of found empty entry. If we cannot find an 
empty entry, we return an error. Our *fs_findfd* also loops through the 
length of our *Root directory*, searches for a matching entry by checking for 
an equal char array to the argument *filename*, and returns the index of found
matching entry. If we cannot find a matching entry, we return an error. Our    
*fs_create* first finds an empty entry in the *Root directory* to write the 
argument *filename* to. We error check to make sure our *filename* is not too 
large, is null terminated, and does not already exist in the *Root directory*. 
We then add information about the *filename* (*startindex*, *size*, and        
*name*, which are held in our *filedescriptor* struct that the *rootblock* 
struct has access too). Our *fs_delete* first finds a matching entry in the    
*Root directory* to our argument *filename*. We error check to make sure our 
*filename* is not too large and is null terminated. We then search for where 
in our *File Allocation Table* our file is located. This could be multiple 
locations. We then jump around our *File Allocation Table*, setting visited  
locations to 0. We go to our disk space where we allocate *Data blocks* and 
write over blocks of size *BLOCK_SIZE* with a void pointer. Lastly, our *fs_ls*
loops through the length of our *Root directory* and prints entries with a 
non-empty char array, which is held in our *name* variable in the
*filedescriptor* struct. Inside *fs_create* and *fs_delete* we also inc and dec
*fs->entries* respectively.

### Phase 3: File Descriptor Operations
We used the following data structures/functions to implement Phase 3:

* struct openfile
* int fs_findemptyfile()
* int fs_findfilefd(int fd)
* int fs_open(const char \*filename)
* int fs_close(int fd)
* int fs_stat(int fd)
* int fs_lseek(int fd, size_t offset)

Our *openfile* struct allows us to keep: a char array *name* that holds the 
filename, a pointer *pointer* which lets us find the stream which we can open 
or close a file with, a variable *offset* that lets us know what our offset 
is, which determines where in the file we read/write too, and a variable       
*file_d* that holds the file descriptor that allows us to find a specific file.
We also added an integer *ofnum* to our *File System* struct so we can keep 
track of how many open files are currently available. Our *fs_findemptyfile* 
is very similar to *fs_findemptyfd*, except it searches our *files* array that
contains *openfile* structs for an unintialized file. Our *fs_findfilefd* is 
very similar to *fs_findfd*, except it searches *files* for a matching file 
descriptor and returns the index of a match. We return fails for both 
functions if we cannot find what we are looking for. As long as there are less 
than 32 open files, we initialize an *openfile* struct and assign values to 
the struct's data members. We lastly increment our count of *ofnum* in our 
*File System* struct. Our *fs_close* finds a file to close, closes it, then 
frees that location in our *files* array so we do not consume uncessary 
memory. We lastly decrement our count of *ofnum* in our *File System* struct. 
Our *fs_stat* loops through the length of our *files* array and returns the 
size of our desired file by searching for a matching file descriptor. Our      
*fs_lseek* does the same as *fs_stat*, but updates our *offset* to the desired 
value.

### Phase 4: File Reading/Writing
We used the following functions to implement Phase 4:

* int fs_findblockindex(int entryindex, int blockindex)
* int fs_firstemptyfat()
* int fs_addblock(int entryindex, int blockindex)
* int fs_write(int fd, void \*buf, size_t count)
* int fs_read(int fd, void \*buf, size_t count)

*fs_read* searches for the openfile using given *fd*, returns the name which 
is then searched for in the root entries. We then mod openfile's offset by 
BLOCK_SIZE which gives us how many data blocks we must move past to find the 
start of our read (blockindex). We then pass the root entryindex and file's 
blockindex to *fs_findblockindex* which follows the fat chain starting from 
root entry's *startindex* until it reaches the correct blockindex and returns
the correct wordindex for the indexed datablock for the file. Then, we read
the correct datablock into a temporary *blockbuf* and copy into the correct 
position of the full buffer we are given. We continue finding the next data
block with *fs_findblockindex* until we have read all of count or reached EOC.
*fs_write* is pretty similar to *fs_read*, but we needed extra functionality.
We created an *fs_addblock* which returns the correct datablock index similar
to *fs_findblockindex*, however, the method also adds appends data to the file
if needed with the use of our helper *fs_firstemptyfat* which finds the first
unused datablock index in the fat. For both *fs_write* and *fs_read* we had
to deal with a base case that assumed an offset in the openfile. The offset
affects *byteread*, *count*, and memory we copy from block into/from buf. 
The base case and general case for both methods have two cases: whether we
have read/written enough, or we if we must continue reading/writing. This 
is dependent on count-offset for base and on count for general.

## Additional Code
### Makefile Development
We build a static library libfs.a and create executables of disk.c and fs.c in 
our libfs folder Makefile. We didn't need to modify the Makefile in our test 
folder.

### Testing File
#### test_fs.c

This test file was given to us to use and it's purpose is to use the api we 
created in fs.c by using the disk created by the given disk api. It was tough
to test phase 2,3, and 4 without them all being finished, because test_fs.c 
uses all three methods developed in these phases for one api call (e.g add/rm).