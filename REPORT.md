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

We envisioned *Data blocks* as all the remaining space on our disk. Our 
*Superblock* stores pertinent information about our disk. Our *File Allocation 
Table* stores 2-byte entries per data block and links them if they are part 
of the same file. Our *Root directory* stores our 128 file descriptors. 
Lastly, our *File System* struct allows us to access our other structs.

### Phase 1: Mounting/Unmounting
We used the following data structures/functions to implement Phase 1:

* struct __attribute__ ((__packed__)) filedescriptor
* static struct fsys \* fs
* struct fsys \* fs_malloc()
* int fs_mount(const char \*diskname)
* int fs_umount(void)
* int fs_getfreefat()
* int fs_getfreefiles()
* int fs_info(void)

// Joel (in this section i usually write about each function/struct)

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
*filedescriptor* struct. 

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
* int fs_addblock(int fd, size_t offset)
* int fs_write(int fd, void \*buf, size_t count)
* int fs_read(int fd, void \*buf, size_t count)

// Joel (in this section i usually write about each function/struct)

## Additional Code
### Makefile Development
We build a static library libfs.a and create executables of disk.c and fs.c in 
our libfs folder Makefile. We didn't need to modify the Makefile in our test 
folder.

### Testing File
#### test_fs.c
// Joel (something quick)