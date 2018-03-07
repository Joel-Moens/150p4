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

};
struct filedescriptor {
	char name[16];
	uint64_t size;
	uint32_t startindex;
	char padding[10];
};
struct superblock {
	char sig[8]; // Must be "ECS150FS"
	uint16_t blocksize;
	uint16_t rootindex;
	uint16_t dataindex;
	uint16_t datablocknum;
	uint8_t fatnum;
	char padding[4079];
};
struct __attribute__ ((__packed__)) fatblock {
	// Keeps track of both the free data blocks and mapping between files 
	// and the data blocks holding their content
	uint16_t word[2048];
};
struct __attribute__ ((__packed__)) rootblock {
	// Contains an entry for each file of fs
	struct filedescriptor entry[128];
};
struct __attribute__ ((__packed__))  datablock {
	void * memory;

};
struct __attribute__ ((__packed__))  datalist {
	void * memory;

};
struct __attribute__ ((__packed__))  fatlist {
	void * memory;
};

struct fsys * fs;
/* TODO: Phase 1 */
struct fsys* fs_malloc()
{
	struct fsys * temp = (void *) malloc(sizeof(void)*4);
	temp->super = (struct superblock *) malloc(sizeof(struct superblock));
	temp->fat = (struct fatblock **) malloc(sizeof(struct fatblock *) * temp->super->fatnum);
	temp->root = (struct rootblock *) malloc(sizeof(struct rootblock));
	temp->data = (struct datablock **) malloc(sizeof(struct datablock *) * temp->super->datablocknum);
	void * address = (void * ) malloc(sizeof(void));
	block_read(0, address);
	temp->super = (struct superblock *)address;

	printf("temp->super signature is: %s, temp->super blocksize is: %d \n", temp->super->sig, temp->super->blocksize);
	if(strcmp(temp->super->sig, "ECS150FS") != 0)
	{
		printf("Error Signature does not equal ECS150FS \n");
		//return NULL;
	}
	if((temp->super->fatnum + 2 + temp->super->datablocknum) != temp->super->blocksize)
	{
		printf("Error Block Size and Fatnum + Root + Super + Data Block Size are not equal. \n");
		return NULL;
	}
	uint16_t index = 1;
	while(index < temp->super->rootindex)
	{
		block_read(index, address);
		printf("Trying to insert fatblock %d\n", index-1);
		temp->fat[index-1] = (struct fatblock *) address;
		index++;	
	}
	block_read(index++, address);
	temp->root = (struct rootblock *)address;
	for(int i = 0; i < temp->super->datablocknum; i++)
	{
		block_read(index++, address);
		temp->data[i] = (struct datablock *) address;
	}
	return temp;
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
	printf("fs->super->sig is %s \n", fs->super->sig);
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

