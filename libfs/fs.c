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
	char name[16];
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
	void *delete_ptr = malloc(BLOCK_SIZE);
	int x = 0, y = 0;
	int fatblock = given->startindex % 2048;
	x = fatblock;
	y = given->startindex - (2048*fatblock);
	memset(&given->name[0],0,sizeof(given->name));
	//What are we doing here ^ ^ ^??
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
	strcpy((fs->files[status])->name,filename);
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
int fs_findblockindex(int entryindex, int blockindex)
{
	struct filedescriptor * given = &(fs->root->entry[entryindex]);
	if(blockindex == 0)
		return given->startindex;
	//if blockindex == 0 then we return the index to the first data block
	// otherwise we use the fat chain to find the correct datablock based on blockindex
	int x = 0, y = 0;
	int fatindex = given->startindex % 2048;
	// Find the fatblock to search through
	int wordindex = 0;
	x = fatindex;
	// x equals fat block index
	y = given->startindex - (2048*fatindex);
	// y equals word index in fat block
	for(int i = 0; i < blockindex; i++)
	{
		wordindex = (fs->fat[x])->word[y];
		if(wordindex == FAT_EOC)
		{
			return -1;
		} // the offset is trying to read past allocated datablock
		x = wordindex % 2048;
		y = wordindex - (2048*fatindex);
	} // Move along the chain until the correct data block index is found
	 
	return wordindex;

} // Find the index of the data block corresponding to file offset
int fs_firstemptyfat()
{

	for(int findex = 0; findex < fs->super->fatnum; findex++)
	{
		for(int windex = 0; windex < 2048; windex++)
		{
			if((fs->fat[findex])->word[windex] != 0)
			{
				continue;
			}
			else
			{
				return (findex*2048 + windex); // return the fat array index
			}
		}
	}
	return -1;

}

int fs_addblock(int entryindex, int blockindex)
{
	struct filedescriptor * given = &(fs->root->entry[entryindex]);
	int fatindex;
	int windex; 
	if(given->startindex == FAT_EOC)
	{
		//Search fat for empty word index
		given->startindex = fs_firstemptyfat();
		//Find the fat block we are searching through
		fatindex = given->startindex % 2048;
		windex = given->startindex - (2048*fatindex);
		(fs->fat[fatindex])->word[windex] = FAT_EOC;
		//Assign given->startindex to the empty data block index in the fat array
		//Assign index in the fat array to FAT_EOC
	} // File descriptor is empty
	if(blockindex == 0)
		return given->startindex;
	//if blockindex == 0 then we return the index to the first data block
	// otherwise we use the fat chain to find the correct datablock based on blockindex
	int x = 0, y = 0;
	fatindex = given->startindex % 2048;
	// Find the fatblock to search through
	int wordindex = 0;
	x = fatindex;
	// x equals fat block index
	y = given->startindex - (2048*fatindex);
	// y equals word index in fat block
	for(int i = 0; i < blockindex; i++)
	{
		wordindex = (fs->fat[x])->word[y];
		if(wordindex == FAT_EOC)
		{
			wordindex = fs_firstemptyfat();
			if(wordindex == -1)
			{
				return -1; 
			} // No more empty data blocks
			else
			{
				(fs->fat[x])->word[y] = wordindex;
			} // Fat word linked to next empty fat index
		} // If the next word in the chain is FAT_EOC we need to keep adding
		x = wordindex % 2048;
		y = wordindex - (2048*fatindex);
	} // Move along the chain until the correct data block index is found
	 
	return wordindex;
}

int fs_write(int fd, void *buf, size_t count)
{
	/* 
	TODO: Phase 4
	Yo Akshay this is going to be super similar to fs_read, 
	the one thing is we have to look at the given filedescriptor in root 
	and see if there is a datablock associated with the file
	if there isn't we have to add a block to the file and set it to given->startindex
	we then set the associated fat word for this data block equal to FAT_EOC

	if root has a given->startindex, but we are writing passed allocated space
	then we have to add a datablock, but append to our current fat list and make this datablock word = FAT_EOC
	 */
	int ofindex = fs_findfilefd(fd);
	int entryindex = fs_findfd(fs->files[ofindex]->name);
	int blockindex = (fs->files[ofindex])->offset % BLOCK_SIZE;
	int offset = (fs->files[ofindex])->offset - (blockindex * BLOCK_SIZE); // Offset after being inside the right data block
	int dataindex = fs_addblock(entryindex, blockindex);
	struct filedescriptor * given = &(fs->root->entry[entryindex]);
	// 
	// number of bytes written can be smaller than count if we run out of size
	return 0;	
}

int fs_read(int fd, void *buf, size_t count)
{
	// /* TODO: Phase 4 */
	size_t byteread = 0;
	int ofindex = fs_findfilefd(fd);
	// Find the open file with fd
	if(ofindex == -1)
	{
		return -1;
	} // fd is invalid
	int entryindex = fs_findfd((fs->files[ofindex])->name); // Find the entry in root using the openfile name
	int blockindex = (fs->files[ofindex])->offset % BLOCK_SIZE; // mod the openfile offset to datablock size to find which file datablock to start at
	int offset = (fs->files[ofindex])->offset - (blockindex * BLOCK_SIZE); // Offset after being inside the right data block
	int dataindex = fs_findblockindex(entryindex, blockindex);
	// find the first datablock in file to read from by using openfile offset
	if(dataindex == -1)
	{
		return byteread;
	} //reached FAT_EOC right away with the offset
	int block2read = (count + offset) % BLOCK_SIZE;
	int readindex = 0;
	char * readbuf = (char *) malloc(sizeof(char) * count);
	char * blockbuf = (char *) malloc(sizeof(char) * BLOCK_SIZE);
	char * breadindex = readbuf; // byte read index not bread index
	while(dataindex != -1 && readindex < block2read)
	{		
		if(block_read(dataindex + fs->super->dataindex, blockbuf) == -1)
			return -1;
		if(readindex == 0)
		{
			if(count > BLOCK_SIZE - offset)
			{
				byteread = BLOCK_SIZE - offset; // first block may be at an offset so we read less bytes than a full block
				offset += byteread; //offset will now be at the beginning of the next data block
				count -= byteread; // subtract count by # bytes read so far
				memcpy(breadindex, blockbuf+offset, BLOCK_SIZE-offset); // copy from offset in first block 
				breadindex += byteread; // move the pointer to the next part of readbuf to read into
			} // Count is larger than the first block read - the offset we start at
			else if(count < BLOCK_SIZE - offset)
			{
				byteread = count; // only read size of count
				offset += byteread; // move offset to where we ended 
				memcpy(buf, blockbuf, count); // copy size of count of blockbuf into the buf
				free(blockbuf);	// free the block buffer after copying into buf
				free(readbuf); // free the read buffer
				return byteread;
			} // Size of read (count) is less than the size the first block - offset
		} // This is our first read, so the offset matters after this we will read block by block
		else
		{
			if(count > BLOCK_SIZE)
			{
				byteread += BLOCK_SIZE; // read a full block of bytes
				offset += byteread; // offset moved by read
				count -= byteread; // size of count remaining decremented 
				memcpy(breadindex, blockbuf, BLOCK_SIZE); // copy a full block into the index of readbuf 
				breadindex += BLOCK_SIZE; // move the byte read index
			} // size of read is larger than this block so keep going
			if(count < BLOCK_SIZE)
			{
				byteread += count; // Add the last bit of count to return the number of bytes read
				offset += byteread; // This will be the last read since size of count is less than the size of a data block
				memcpy(breadindex, blockbuf, count);
				breadindex += count;
			} // size of remaining read is smaller than a block, only copy in the remaining size
		} // Now reading block by block, either continuing 
	}
	memcpy(buf, readbuf, byteread); //copy all the blocks of memory added by blockbuf using breadindex
	free(blockbuf);	// free the blockbuffer
	free(readbuf); // free the readbuffer after copying into buf
	return byteread;

}

