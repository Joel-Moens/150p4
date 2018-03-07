#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "disk.h"
#include "fs.h" 
#define FAT_EOC 0xFFFF;
//uint8_t
//uint16_t
//uint32_t
//uint64_t
struct fsys {
	struct superblock * super;
	struct fatblock ** fat;
	struct rootblock * root;
	struct datablock ** data;
}; // File system struct, contains a superblock, list of fatblocks, rootblock, and list of datablocks
struct __attribute__ ((__packed__)) filedescriptor {
	char name[16];
	uint64_t size;
	uint32_t startindex;
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


struct rootblock * root;
struct superblock * super;
struct fatblock ** fat;
struct datablock ** data;
//fs was going to be our filesystem that we use for this entire project, but trying to test out each component to see what the problem is
/* TODO: Phase 1 */
int fs_malloc()
{
	// Allocate for each component
	super = (struct superblock *) malloc(sizeof(struct superblock));
	printf("super signature is: %s, super blocksize is: %d \n", super->sig, super->blocksize);
	void * address = (void * ) malloc(sizeof(void));
	block_read(0, address);
	super = (struct superblock *)address;

	// CURRENTLY STUCK HERE trying to allocated fatblock **
	// I need a fatblock** because I'm making an array of fatblock *
	// This next printf works, showing that block_read api and type casting works fine
	printf("super signature is: %s, super blocksize is: %d \n", super->sig, super->blocksize);

	//Tried googling this on stackoverflow and says to use valgrind, I don't know how to get that to test the code though
	fat = (struct fatblock **) malloc((int)super->fatnum * sizeof(struct fatblock*) );
	printf("super signature is: %s, super blocksize is: %d \n", super->sig, super->blocksize);
	root = (struct rootblock *) malloc(sizeof(struct rootblock));
	data = (struct datablock **) malloc(sizeof(struct datablock) * super->datablocknum);

	printf("super signature is: %s, super blocksize is: %d \n", super->sig, super->blocksize);
	char signature[8] = "ECS150FS";
	if(strcmp(super->sig, signature) != 0)
	{
		printf("Error Signature does not equal ECS150FS \n");
		//return NULL;
	}
	if((super->fatnum + 2 + super->datablocknum) != super->blocksize)
	{
		printf("Error Block Size and Fatnum + Root + Super + Data Block Size are not equal. \n");
		return -1;
	}
	uint16_t index = 1;
	while(index < super->rootindex)
	{
		block_read(index, address);
		printf("Trying to insert fatblock %d\n", index-1);
		fat[index-1] = (struct fatblock *) address;
		index++;	
	}
	block_read(index++, address);
	root = (struct rootblock *)address;
	for(int i = 0; i < super->datablocknum; i++)
	{
		block_read(index++, address);
		data[i] = (struct datablock *) address;
	}
	return 0;
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
		if(fs_malloc() == -1)
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
	// make null all associations between file system and drive
	// block_disk_close()
	return 0;	
}
int fs_getfreefat()
{
	int index = 0;
	while(fat[index] != 0)
	{
		index++;
	}
	return (super->datablocknum - index);
}
int fs_getfreefiles()
{
	int index = 0;
	while(root->entry[index].name[0] != '\0')
	{
		index++;
	}
	return (128 - index);
}
int fs_info(void)
{
	printf("super->sig is %s \n", super->sig);
	printf("FS Info: \n total_blk_count=%d \n fat_blk_count=%d \n rdir_blk=%d \n data_blk=%d\n data_blk_count=%d\n fat_free_ratio=%d/%d\n rdir_free_ratio=%d/%d\n",
		super->blocksize,
		super->fatnum,
		super->rootindex,
		super->dataindex,
		super->datablocknum,
		fs_getfreefat(),super->datablocknum,
		fs_getfreefiles(), 128);
	return 0;	/* TODO: Phase 1 */
}

int fs_create(const char *filename)
{
	return 0;	/* TODO: Phase 2 */
}

int fs_delete(const char *filename)
{
	return 0;	/* TODO: Phase 2 */
}

int fs_ls(void)
{
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

