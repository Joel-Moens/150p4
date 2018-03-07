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
}; //Stores most needed info about disk
struct __attribute__ ((__packed__)) fatblock {
	uint16_t word[2048];
}; //Block that stores 2byte entries per datablock linking them if used in same file
//THIS MAY NEED TO BE A uint16_t pointer that we initialize later?
struct __attribute__ ((__packed__)) rootblock {
	// Contains an entry for each file of fs
	struct filedescriptor entry[128];
}; //Block that stores all 128 fd
struct __attribute__ ((__packed__))  datablock {
	void * memory;
}; //block that stores file data
struct __attribute__ ((__packed__)) fsys {
	struct superblock * super;
	struct fatblock ** fat;
	struct rootblock * root;
	struct datablock ** data;
}; // File system struct, contains a superblock, list of fatblocks, rootblock, and list of datablocks

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
	printf("super signature is: %s, super blocksize is: %d, fatnum is: %d\n", newfs->super->sig, newfs->super->blocksize, newfs->super->fatnum);
	
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
	newfs->data = (struct datablock **) malloc(sizeof(struct datablock *) * newfs->super->datablocknum);
	for(int i = 0; i < newfs->super->datablocknum; i++)
	{
		newfs->data[i] = (struct datablock *) malloc(BLOCK_SIZE);
		block_read(diskindex++, newfs->data[i]);
		//printf("Allocated memory for datablock %d\n", i);
	} // iterate through each datablock and insert into fs
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
	printf("TRYING TO UMOUNT \n");
	int index = 0;
	int dataindex = 0;
	int fatindex = 0;
	size_t diskindex = 0;
	while(index != block_disk_count())
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
		else
		{
			struct datablock * temp = fs->data[dataindex++];
			if(block_write(diskindex++, temp) == -1)
				return -1; // if blockwrite fails 
			free(temp); // Free datablock * one by one
			// printf("Freed datablock #%d \n",dataindex);
			index++;
			continue;
 		} // Write the data blocks 

	}
	free(fs->data);
	//printf("Freed data list \n");
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
	//Find the first empty entry in root
	//struct filedescriptor * given = fs_findemptyfd();
	//Return -1 if no empty fd found
	// given->name = filename
	// given->size = 0;
	// given->startindex = FAT_EOC;

	return 0;	/* TODO: Phase 2 */
}

int fs_delete(const char *filename)
{
	//struct filedescriptor * given = fs_findfd(filename)
	//strncmp 
	//if given->size == 0
	//then given->startindex should equal FAT_EOC
	//if so given-> = '\0' no problem
	//if given->size > 0
	//if given->startindex != FAT_EOC
	//if given->startindex < 2048 we are in fat[0][startindex]
	//if given->startindex > 2047 we are in fat[1][startindex-2048]
	//if given->startindex > 4095 we are in fat[2][startindex-4096]
	//if given->startindex > 6143 we are in fat[3][startindex-6144]
	// SET x to the fat block index SET y to the fat word index
	//While the current fatword != FAT_EOC we clean datablock by assigning NULL and fat word is set to 0 
	//clean fs->data[given->startindex], find next datablock and clean fs->fat[x][y]
	//WE HAVE TO CLEAN ALL DATABLOCKS AND THE FAT
	//Start by using given startindex to find the fatword
	return 0;	/* TODO: Phase 2 */
}

int fs_ls(void)
{
	//for(int i = 0; i<128; i++)
	//struct filedescripto * given = fs->root->entry[i];
	//if(given->name[0] != '\0')
	//	print given->
	return 0;	/* TODO: Phase 2 */
}

int fs_open(const char *filename)
{
	return 0;	/* TODO: Phase 3 */
}

int fs_close(int fd)
{
	return 0;	/* TODO: Phase 3 */
}

int fs_stat(int fd)
{
	return 0;	/* TODO: Phase 3 */
}

int fs_lseek(int fd, size_t offset)
{
	return 0;	/* TODO: Phase 3 */
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

