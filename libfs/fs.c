#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "disk.h"
#include "fs.h" 
#define FAT_EOC 65535
//uint8_t
//uint16_t
//uint32_t
//uint64_t
struct __attribute__ ((__packed__)) filedescriptor {
	char name[16];
	uint32_t size;
	uint16_t startindex;
	char padding[10];
}; //File descriptor stores meta data for each file in root
struct __attribute__ ((__packed__)) superblock  {
	char sig[8]; // Must be "ECS150FS"
	uint16_t blocksize;
	uint16_t rootindex;
	uint16_t dataindex;
	uint16_t datablocknum;
	uint8_t fatnum;
	char padding[4079];
}; //Stores most needed info about disk's
struct openfile {
	FILE * pointer;
	size_t offset;
	int file_d;
}; // pointer to actual file, offset where we are reading from file;
struct __attribute__ ((__packed__)) fatblock {
	uint16_t word[2048];
}; //Block that stores 2byte entries per datablock linking them if used in same file
//THIS MAY NEED TO BE A uint16_t pointer that we initialize later?
struct __attribute__ ((__packed__)) rootblock {
	// Contains an entry for each file of fs
	struct filedescriptor entry[128];
}; //Block that stores all 128 fd
struct __attribute__ ((__packed__)) fsys {
	struct superblock * super;
	struct fatblock ** fat;
	struct rootblock * root;
	struct openfile ** files;
	int ofnum;
}; // File system struct, contains a superblock, list of fatblocks, rootblock, and list of openfiles and openfile num

//Pointer to the filesystem we will malloc when we mount a disk
static struct fsys * fs;
//fs was going to be our filesystem that we use for this entire project, but trying to test out each component to see what the problem is
/* TODO: Phase 1 */
struct fsys * fs_malloc()
{
	// Malloc for a pointer that will read from disk
	struct fsys * newfs = (struct fsys *) malloc(sizeof(struct fsys));
	newfs->super = (struct superblock *) malloc(sizeof(struct superblock));
	// Malloc a temp fsys pointer for the filesystem 
	block_read(0, newfs->super);
	// Malloc the superblock pointer for the superblock point to buffer
	
	char signature[8] = "ECS150FS";
	if(strncmp(newfs->super->sig, signature, 8) != 0)
	{
		printf("Error Signature does not equal ECS150FS \n");
		return NULL;
	}
	if((newfs->super->fatnum + 2 + newfs->super->datablocknum) != newfs->super->blocksize)
	{
		printf("Error Block Size and Fatnum + Root + Super + Data Block Size are not equal. \n");
		return NULL;
	}
	
	uint16_t blockindex = 1;
	size_t diskindex = 1;
	//Malloc for the fatblocks
	newfs->fat = (struct fatblock **) malloc(sizeof(struct fatblock *) * newfs->super->fatnum);
	for(uint16_t i=0; i<newfs->super->fatnum; i++)
	{
		newfs->fat[i] = (struct fatblock *) malloc(sizeof(struct fatblock));
		//printf("Allocated memory for fatblock %d\n", i);
	}
	while(blockindex < newfs->super->rootindex)
	{
		//printf("Trying to insert fatblock %d rootindex is %d \n", blockindex-1,newfs->super->rootindex);
		block_read(diskindex, newfs->fat[blockindex-1]);
		//printf("If buffer were casted to a fatblock the first entry would be: %d\n", newfs->fat[blockindex-1]->word[0]);
		diskindex++;
		//newfs->fat[index-1] = (struct fatblock *) malloc(sizeof(struct fatblock));
		blockindex++;
	} // iterate through each fat and insert into fs
	newfs->root = (struct rootblock *) malloc(sizeof(struct rootblock));
	block_read(blockindex++, newfs->root);
	newfs->files = (struct openfile **) malloc(sizeof(struct openfile*)*32);
	newfs->ofnum = 0;
	return newfs;
}


int fs_mount(const char *diskname)
{
	/* TODO: Phase 1 */
	if(block_disk_open(diskname) == -1)
	{
		return -1;
		// Cannot be opened or is already open
	}
	else
	{
		printf("TRYING TO MOUNT \n");
		
		//Make a superblock *
		fs = fs_malloc();
		if(fs == NULL)
		{
			return -1;
		}
		//Make a fatblock *
		//Make a rootblock *
		return 0;
	}
}

int fs_umount(void)
{
	/* TODO: Phase 1 */
	printf("TRYING TO UNMOUNT \n");
	int index = 0;
	int fatindex = 0;
	size_t diskindex = 0;
	while(index != fs->super->rootindex+1)
	{
		if(index == 0)
		{
			if(block_write(diskindex++, fs->super) == -1)
				return -1; // if blockwrite fails 
			index++;
			continue;
			// Do not free super until the end
		} // Write super into first block of disk
		if(index < fs->super->rootindex)
		{
			struct fatblock * temp = fs->fat[fatindex++];
			if(block_write(diskindex++, temp) == -1)
				return -1; // if blockwrite fails 
			free(temp); // Free the fatblock * one by one
			// printf("Freed fatblock #%d \n",fatindex);
			index++;
			continue;
		} // Write the fat blocks 
		if(index == fs->super->rootindex)
		{
			free(fs->fat); // Free the fatblock **
			// printf("Free fat list \n");
			if(block_write(diskindex++, fs->root) == -1)
				return -1; // if blockwrite fails 
			free(fs->root); // Free the rootblock *
			// printf("Freed rootblock \n");
			index++;
			continue;
		} // Write the root block into disk and free

	}
	free(fs->super);
	//printf("Freed superblock \n");
	free(fs);
	//printf("Freed filesystem \n");
	return 0;	
}
int fs_getfreefat()
{
	int blockindex = 0;
	int wordindex = 0;
	while((fs->fat[blockindex])->word[wordindex] != 0)
	{
		
		if(wordindex + (blockindex * 2048) > fs->super->datablocknum)
		{
			return 0;
			//We are passed the last known fat word
		}
		printf("fat[%d][%d] is : %d FAT_EOC is : %d \n", blockindex, wordindex, (fs->fat[blockindex])->word[wordindex], FAT_EOC);
		if(wordindex < 2048)
		{
			wordindex++;			
		}
		else
		{
			wordindex = 0;
			blockindex++;
		}

	}
	return (fs->super->datablocknum - (wordindex + (blockindex * 2048)));
}
int fs_getfreefiles()
{
	int index = 0;
	while(fs->root->entry[index].name[0] != '\0')
	{
		index++;
	}
	return (128 - index);
}


// helper function to find first empty entry in root
int fs_findemptyfd()
{
	for (int i = 0; i < 128; i++)
	{
		// return index of where we have an empty entry
		if (fs->root->entry[i].name[0] == '\0')
			return i;
	}	
	return -1;
}

// helper function to find matching filename in root
int fs_findfd(const char *filename)
{
	for (int i = 0; i < 128; i++)
	{
		// return index of where we have a matching filename
		if (strncmp(filename,fs->root->entry[i].name,16))
			return i;
	}	
	return -1;
}

int fs_info(void)
{
	printf("super->sig is %s \n", fs->super->sig);
  	printf("FS Info: \n total_blk_count=%d \n fat_blk_count=%d \n rdir_blk=%d \n data_blk=%d\n data_blk_count=%d\n fat_free_ratio=%d/%d\n rdir_free_ratio=%d/%d\n",
	 	fs->super->blocksize,
	 	fs->super->fatnum,
	 	fs->super->rootindex,
	 	fs->super->dataindex,
	 	fs->super->datablocknum,
	 	fs_getfreefat(),fs->super->datablocknum,
	 	fs_getfreefiles(), 128);

	return 0;	/* TODO: Phase 1 */
}
int fs_create(const char *filename)
{
	int status = fs_findemptyfd();
	// if we can't find an empty entry in root, fail
	if (status == -1)
		return status;
	// if our filename is too large or isn't NULL-terminated or already exists in root, fail
	if ((sizeof(*filename) > 16) || (filename[-1] != '\0') || (fs_findfd(filename) != -1)) 
		return -1;
	// adds file into file system 
	struct filedescriptor * given = &(fs->root->entry[status]);
	strcpy(given->name,filename);
	given->size = 0;
	given->startindex = FAT_EOC;
	return 0;	/* TODO: Phase 2 */
}

int fs_delete(const char *filename)
{
	// we will have to add check for file being currently open, return -1 if so
	int status = fs_findfd(filename);
	// if we can't find the filename in root, fail
	if (status == -1)
		return status;
	// if our filename is too large or isn't NULL-terminated, fail 
	if ((sizeof(*filename) > 16) || (filename[-1] != '\0')) 
		return -1;
	// removes file from file system
	struct filedescriptor * given = &(fs->root->entry[status]);
	if ((given->size == 0) && (given->startindex == FAT_EOC))
	{
		memset(&given->name[0],0,sizeof(given->name));
		return 0;
	}
	// we need to find out where in FAT our file info is stored
	void *delete_ptr = malloc(4096);
	int x = 0, y = 0;
	int fatblock = given->startindex % 2048;
	x = fatblock;
	y = given->startindex - (2048*fatblock);
	memset(&given->name[0],0,sizeof(given->name));
	given->size = 0;
	given->startindex = FAT_EOC;
	// looping through disk and FAT to clear data
	while ((fs->fat[x])->word[y] != FAT_EOC)
	{
		int old_y = y;
		int new_y = (fs->fat[x])->word[y];
		(fs->fat[x])->word[y] = 0;
		block_write(old_y + fs->super->dataindex,delete_ptr);
		y = new_y;
	} // While the current word in the chain doesn't equal FAT_EOC empty the data block associated with word
	// one last block_write to clean FAT_EOC root/disk
	(fs->fat[x])->word[y] = 0;
	block_write(y + fs->super->dataindex,delete_ptr);
	free(delete_ptr);
	return 0;	/* TODO: Phase 2 */
}

int fs_ls(void)
{
	// need to check if no underlying virtual disk was opened, return -1 if so
	// also need to match test_ref's output
	for (int i = 0; i < 128; i++)
	{
		if (fs->root->entry[i].name[0] != '\0')
			printf("%s",fs->root->entry[i].name);
	}	
	return 0;	/* TODO: Phase 2 */
}

// helper function to find first empty file in files
int fs_findemptyfile()
{
	for (int i = 0; i < 32; i++)
	{
		// return index of where we have an empty entry
		if (!(fs->files[i]))
			return i;
	}	
	return -1;
}

// helper function to find matching file descriptor in files
int fs_findfilefd(int fd)
{
	for (int i = 0; i < 32; i++)
	{
		// check if file has been malloc'ed
		if (fs->files[i])
		{
			// return index of where we have match
			if ((fs->files[i])->file_d == fd)
				return i;
		}
	}	
	return -1;
}

int fs_open(const char *filename)
{
	// already 32 open files cannot open another
	if(fs->ofnum == 32)
		return -1;
	int status = fs_findemptyfile();
	// could not find an empty file, ERROR
	if (status == -1)
		return status;
	fs->files[status] = (struct openfile *) malloc(sizeof(struct openfile));
	(fs->files[status])->pointer = fopen(filename, "r+");
	(fs->files[status])->offset = 0;
	(fs->files[status])->file_d = fileno((fs->files[status])->pointer);
	// if file can't be opened
	if((fs->files[fs->ofnum])->pointer == NULL)
		return -1;
	fs->ofnum++;

	return 0;	/* TODO: Phase 3 */
}

int fs_close(int fd)
{
	// no files open, cannot close any
	if(fs->ofnum == 0)
		return -1;
	int status = fs_findfilefd(fd);
	// could not find file descriptor, ERROR
	if (status == -1)
		return status;
	fclose((fs->files[status])->pointer);
	(fs->files[status])->pointer = NULL;
	free(fs->files[status]);
	fs->ofnum--;
	
	return 0;	/* TODO: Phase 3 */
}

int fs_stat(int fd)
{
	for (int i = 0; i < 32; i++)
	{
		// check if file has been malloc'ed
		if (fs->files[i])
		{
			// we found a match
			if ((fs->files[i])->file_d == fd)
				return sizeof((fs->files[i])->pointer);
		}
	}	
	return -1;	/* TODO: Phase 3 */
}

int fs_lseek(int fd, size_t offset)
{
	for (int i = 0; i < 32; i++)
	{
		// check if file has been malloc'ed
		if (fs->files[i])
		{
			// check if offset is out of bounds
			if (sizeof(offset) > sizeof((fs->files[i])->pointer))
				return -1;
			// we found a match
			else if ((fs->files[i])->file_d == fd)
			{
				(fs->files[i])->offset = offset;
				return 0;
			}
		}
	}	
	return -1;	/* TODO: Phase 3 */
}

int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
	return 0;	
}

int fs_read(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
	return 0;
}

