#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "disk.h"
#include "fs.h"
//uint8_t
//uint16_t
//uint32_t
//uint64_t
struct fsys {
	struct superblock * super;
	struct fatblock * fat;

}
struct filedescriptor {
	char name[16];
	uint64_t size;
	uint32_t startindex;
	char[10] padding;

}

struct superblock {
	char sig[8]; // Must be "ECS150FS"
	uint16_t blocksize;
	uint16_t rootindex;
	uint16_t dataindex;
	uint16_t datablocknum;
	uint8_t fatnum;
	char padding[4079];
}
struct __attribute__ ((__packed__)) fatblock {
	// Keeps track of both the free data blocks and mapping between files 
	// and the data blocks holding their content
	uint16_t word[2048];
}
struct __attribute__ ((__packed__)) rootblock {
	// Contains an entry for each file of fs
	struct filedescriptor[128] entry;
}
struct __attribute__ ((__packed__))  datablock {
	void * memory;

}
struct __attribute__ ((__packed__))  datalist {
	void * memory;

}
struct __attribute__ ((__packed__))  fatlist {
	void * memory;

}

struct fsys * filesystem;

/* TODO: Phase 1 */
struct fsys* fs_malloc()
{
	fsys * temp = (struct fsys*) malloc((sizeof(struct fsys)));
	void * address;
	block_read(0, address);
	temp->super = (struct superblock *)address;

	uint16_t index = 1;
	temp->fat = (struct fatblock*) malloc(sizeof(fatblock) * temp->super->fatnum);
	while(index < temp->super->rootindex)
	{
		block_read(index, address);
		temp->fat[index-1] = (struct fatblock *) address;
		index++;	
	}
	block_read(index++, address);
	temp->root = (struct rootblock *)address;
	temp->data = (struct datablock*) malloc(sizeof(datablock) * temp->super->datablocknum);
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
		
		//Make a superblock *
		filesystem = fs_malloc();
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
}

int fs_info(void)
{
	/* TODO: Phase 1 */
}

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
}

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
}

int fs_ls(void)
{
	/* TODO: Phase 2 */
}

int fs_open(const char *filename)
{
	/* TODO: Phase 3 */
}

int fs_close(int fd)
{
	/* TODO: Phase 3 */
}

int fs_stat(int fd)
{
	/* TODO: Phase 3 */
}

int fs_lseek(int fd, size_t offset)
{
	/* TODO: Phase 3 */
}

int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
}

int fs_read(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
}

