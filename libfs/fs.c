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
struct __attribute__ ((__packed__)) fsys {
	struct superblock * super;
	struct fatblock * fat[4];
	struct rootblock * root;
	struct datablock * data[8192];
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

static struct fsys * fs;
//fs was going to be our filesystem that we use for this entire project, but trying to test out each component to see what the problem is
/* TODO: Phase 1 */
int fs_malloc()
{
	// Allocate for each component
	void * buffer = (void *) malloc(sizeof(void));
	struct fsys * newfs = (struct fsys *) malloc(sizeof(struct fsys));
	block_read(0, buffer);
	newfs->super = (struct superblock *) malloc(sizeof(struct superblock));
	newfs->super = buffer;
	printf("super signature is: %s, super blocksize is: %d \n", newfs->super->sig, newfs->super->blocksize);
	
	char signature[8] = "ECS150FS";
	if(strcmp(newfs->super->sig, signature) != 0)
	{
		printf("Error Signature does not equal ECS150FS \n");
		//return NULL;
	}
	if((newfs->super->fatnum + 2 + newfs->super->datablocknum) != newfs->super->blocksize)
	{
		printf("Error Block Size and Fatnum + Root + Super + Data Block Size are not equal. \n");
		return -1;
	}
	
	uint16_t index = 1;
	newfs->fat[0] = (struct fatblock *) malloc(sizeof(struct fatblock) * newfs->super->fatnum);
	while(index < newfs->super->rootindex)
	{
		printf("Trying to insert fatblock %d rootindex is %d \n", index-1,newfs->super->rootindex);
		block_read(index, buffer);
		newfs->fat[index-1] = buffer;
		index++;
		printf("Trying to insert fatblock %d rootindex is %d \n", index-1,newfs->super->rootindex);
	} // iterate through each fat and insert into fs
	block_read(index++, (void *) newfs->root);
	return 0;
	for(int i = 0; i < newfs->super->datablocknum; i++)
	{
		block_read(index++, newfs->data[i]);
	} // iterate through each datablock and insert into fs
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
	while(fs->fat[index] != 0)
	{
		index++;
	}
	return (fs->super->datablocknum - index);
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
	//printf("super->sig is %s \n", fs->super->sig);
	// printf("FS Info: \n total_blk_count=%d \n fat_blk_count=%d \n rdir_blk=%d \n data_blk=%d\n data_blk_count=%d\n fat_free_ratio=%d/%d\n rdir_free_ratio=%d/%d\n",
	// 	fs->super->blocksize,
	// 	fs->super->fatnum,
	// 	fs->super->rootindex,
	// 	fs->super->dataindex,
	// 	fs->super->datablocknum,
	// 	fs_getfreefat(),fs->super->datablocknum,
	// 	fs_getfreefiles(), 128);
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

