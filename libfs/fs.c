#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "disk.h"
#include "fs.h" 
#define FAT_EOC 65535

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
	int usedata;
	int entries;
}; // File system struct, contains a superblock, list of fatblocks, rootblock, and list of openfiles and openfile num

//Pointer to the filesystem we will malloc when we mount a disk
static struct fsys * fs;
/* TODO: Phase 1 */
int fs_iteratefat()
{
	int count = 0;
	for(int x = 0; x < fs->super->fatnum; x++)
	{
		for(int y = 0; y < 2048; y++)
		{
			if(fs->fat[x]->word[y] != 0)
				count++;
		} // iterate through fat word index
	}// iterate through the fat block index
	return count;
} //iterate fat and return #usedata
int fs_iterateroot()
{
	int count = 0;
	for(int i = 0; i < 128; i++)
	{
		if(fs->root->entry[i].name[0] != '\0')
		{
			count++;
		} // file has a name add to entries
	}// iterate through the root entries
	return count;
} //iterate root and return #entries
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
		return NULL;
	}
	if((newfs->super->fatnum + 2 + newfs->super->datablocknum) != newfs->super->blocksize)
	{
		return NULL;
	}
	
	uint16_t blockindex = 1;
	size_t diskindex = 1;
	//Malloc for the fatblocks
	newfs->fat = (struct fatblock **) malloc(sizeof(struct fatblock *) * newfs->super->fatnum);
	for(uint16_t i=0; i<newfs->super->fatnum; i++)
	{
		newfs->fat[i] = (struct fatblock *) malloc(sizeof(struct fatblock));
	}
	while(blockindex < newfs->super->rootindex)
	{
		block_read(diskindex, newfs->fat[blockindex-1]);
		diskindex++;
		blockindex++;
	} // iterate through each fat and insert into fs
	newfs->root = (struct rootblock *) malloc(sizeof(struct rootblock));
	block_read(blockindex++, newfs->root);
	newfs->files = (struct openfile **) malloc(sizeof(struct openfile*)*32);
	newfs->ofnum = 0;
	newfs->usedata = 0;
	newfs->entries = 0;
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
		
		//Make a superblock *
		fs = fs_malloc();
		if(fs == NULL)
		{
			return -1;
		}
		fs->usedata = fs_iteratefat();
		fs->entries = fs_iterateroot();
		//Make a fatblock *
		//Make a rootblock *
		return 0;
	}
}

int fs_umount(void)
{
	/* TODO: Phase 1 */
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
			index++;
			continue;
		} // Write the fat blocks 
		if(index == fs->super->rootindex)
		{
			free(fs->fat); // Free the fatblock **
			if(block_write(diskindex++, fs->root) == -1)
				return -1; // if blockwrite fails 
			free(fs->root); // Free the rootblock *
			index++;
			continue;
		} // Write the root block into disk and free

	}
	if(block_disk_close() == -1)
		return -1;
	free(fs->super);
	free(fs);
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
		if(wordindex < 2048)
		{
			wordindex++;			
		} //Still inside fat block keep moving along words
		else
		{
			wordindex = 0;
			blockindex++;
		} //Passed this fat block move on to the next and reset wordindex;

	}
	return (fs->super->datablocknum - (wordindex + (blockindex * 2048)));
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
		if (strncmp(filename,fs->root->entry[i].name,16) == 0)
		{
			return i;
		}
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
	 	fs->super->datablocknum-fs->usedata,fs->super->datablocknum,
	 	128-fs->entries, 128);

	return 0;	/* TODO: Phase 1 */
}
int fs_create(const char *filename)
{
	int status = 0;
	if(fs->entries < 128)
	{
		status = fs_findemptyfd();
	}
	else
	{
		return -1;
	}
	// if our filename is too large or isn't NULL-terminated or already exists in root, fail
	if ((sizeof(*filename) > 16) || (filename[-1] != '\0') || (fs_findfd(filename) != -1)) 
	{
		return -1;
	}
	// adds file into file system 
	struct filedescriptor * given = &(fs->root->entry[status]);
	strcpy(given->name,filename);
	given->size = 0;
	given->startindex = FAT_EOC;
	fs->entries++;
	return 0;	/* TODO: Phase 2 */
}

int fs_delete(const char *filename)
{
	// we will have to add check for file being currently open, return -1 if so
	int status = fs_findfd(filename);
	char empty[16] = "\0";
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
		memcpy(given->name, empty, 16);
		fs->entries--;
		return 0;
	}
	// we need to find out where in FAT our file info is stored
	void *delete_ptr = malloc(BLOCK_SIZE);
	int x = 0, y = 0;
	y = given->startindex % 2048; // find word index 
	int fatblock = (given->startindex - y)/2048; // find fatblock index
	x = fatblock;
	memcpy(given->name, empty, 16);
	given->size = 0;
	given->startindex = FAT_EOC;
	fs->usedata--;
	// looping through disk and FAT to clear data
	while ((fs->fat[x])->word[y] != FAT_EOC)
	{
		int old_y = y;	// Save old word index
		int new_y = (fs->fat[x])->word[y]; // Find the new word index
		x = new_y % 2048;
		new_y -= (2048*x);
		// decrement the global # of used datablocks
		(fs->fat[x])->word[y] = 0;
		block_write(old_y + fs->super->dataindex,delete_ptr);
		fs->usedata--;
		y = new_y;
	} // While the current word in the chain doesn't equal FAT_EOC empty the data block associated with word
	// one last block_write to clean FAT_EOC root/disk
	(fs->fat[x])->word[y] = 0;
	block_write(y + fs->super->dataindex,delete_ptr);
	fs->entries--;
	free(delete_ptr);
	return 0;	/* TODO: Phase 2 */
}

int fs_ls(void)
{
	// need to check if no underlying virtual disk was opened, return -1 if so
	// also need to match test_ref's output
	printf("FS Ls:\n");
	for (int i = 0; i < 128; i++)
	{
		if (fs->root->entry[i].name[0] != '\0')
			printf("file: %s, size: %d, data_blk: %d\n",fs->root->entry[i].name, fs->root->entry[i].size, fs->root->entry[i].startindex);
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
		if (fs->files[i] != NULL)
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

	return (fs->files[status])->file_d;	/* TODO: Phase 3 */
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
		FILE * fp;
		if (fs->files[i])
		{
			
			// we found a match
			if ((fs->files[i])->file_d == fd)
			{
				fp = (fs->files[i])->pointer;
				fseek(fp, 0, SEEK_END);
				int size = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				return size;
			}
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
	y =  given->startindex % 2048; // y is startindex%2048
	int fatindex =  (given->startindex - y) / 2048;
	// Find the fatblock to search through
	int wordindex = 0;
	x = fatindex;
	// x equals fat block index
	for(int i = 0; i < blockindex; i++)
	{
		wordindex = (fs->fat[x])->word[y];
		if(wordindex == FAT_EOC)
		{
			return -1;
		} // the offset is trying to read past allocated datablock
		y = wordindex % 2048;
		x = (wordindex - y) / 2048;
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
				// have a global # of data blocks being used
				// increment the global #
				return (findex*2048 + windex); // return the fat array index
			}
		}
	}
	return -1; // If we search through entire fat array and no block equals zero fat array is full;

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
		if(given->startindex == -1)
		{
			given->startindex = FAT_EOC;
			return -1;
		} // No free blocks left to be allocated 
		//Find the fat block we are searching through
		windex = given->startindex % 2048;
		fatindex = (given->startindex - windex) / 2048;
		(fs->fat[fatindex])->word[windex] = FAT_EOC;
		fs->usedata++; //Using another datablock
		//Assign given->startindex to the empty data block index in the fat array
		//Assign index in the fat array to FAT_EOC
	} // File descriptor hasn't been written into yet, find the starting index set next word link to FAT_EOC and add to fs->usedata
	if(blockindex == 0)
	{
		return given->startindex;
	} //if blockindex == 0 then we return the index to the first data block
	// otherwise we use the fat chain to find the correct datablock based on blockindex
	int x = 0, y = 0;
	y = given->startindex % 2048;
	fatindex = (given->startindex - y) / 2048;
	// Find the fatblock to search through
	int wordindex = 0;
	x = fatindex;
	// x equals fat block index
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
				(fs->fat[x])->word[y] = wordindex; // Attach the new word to the file block chain
				y = wordindex % 2048; // Fatblock is equal to the wordindex mod 2048
				x = (wordindex - y) / 2048; // Index in fatblock equal to wordindex - (fatblockindex * 2048)
				(fs->fat[x])->word[y] = FAT_EOC; // Make the new block added equal to fat eoc
				fs->usedata++;
			} // Fat word linked to next empty fat index
		} // If the next word in the chain is FAT_EOC we need to keep adding
		else
		{
			y = wordindex % 2048; // Fatblock is equal to the wordindex mod 2048
			x = (wordindex - y) / 2048; // Index in fatblock equal to wordindex - (fatblockindex * 2048)	
		}
		
	} // Move along the chain until the correct data block index is found
	
	return wordindex;
}

int fs_write(int fd, void *buf, size_t count)
{

	size_t byteread = 0;
	int ofindex = fs_findfilefd(fd);
	if(ofindex == -1)
	{
		return -1;
	}
	int entryindex = fs_findfd(fs->files[ofindex]->name);
	int blockrem = (fs->files[ofindex])->offset % BLOCK_SIZE;
	int blockindex = ((fs->files[ofindex])->offset - blockrem) / BLOCK_SIZE;
	int offset = (fs->files[ofindex])->offset - (blockindex * BLOCK_SIZE); // Offset after being inside the right data block
	int dataindex = fs_addblock(entryindex, blockindex);	// Find the index of datablock in file based on offset add datablocks if needed
	if(dataindex == -1)
	{
		return byteread;
	} // No more datablocks that can be allocated
	blockrem = (count + offset) % BLOCK_SIZE;
	int block2write = (count + offset - blockrem) / BLOCK_SIZE;
	struct filedescriptor * given = &(fs->root->entry[entryindex]);
	int writeindex = 0;
	char * blockbuf = (char *) malloc(sizeof(char) * BLOCK_SIZE);
	char * bwriteindex = buf; // byte write index not bread index
	char * breadindex = blockbuf;
	while(dataindex != -1 && writeindex < block2write+1)
	{
		if(writeindex == 0)
		{
			if(block_read(dataindex + fs->super->dataindex, blockbuf) == -1)
			{
				return -1;
			} // We have to read the first block in and only change past the offset of the block
			if(count > BLOCK_SIZE - offset)
			{
				breadindex += offset; // Move the pointer in blockbuf to offset
				memcpy(breadindex, buf, BLOCK_SIZE-offset); // Copy the buf into block
				bwriteindex += BLOCK_SIZE-offset; // Move the pointer in write buffer 
				if(block_write(dataindex + fs->super->dataindex, blockbuf) == -1)
				{
					return -1;
				} // Write the new block with previous data + offset data
				byteread += BLOCK_SIZE - offset;
				count -= BLOCK_SIZE - offset;
				dataindex = fs_addblock(entryindex, ++writeindex + blockindex);
				continue;

			} // Gonna continue 
			else if(count <= BLOCK_SIZE - offset)
			{
				breadindex += offset;
				memcpy(breadindex, buf, count);
				if(block_write(dataindex + fs->super->dataindex, blockbuf) == -1)
				{
					return byteread;
				}
				byteread += count;
				(fs->files[ofindex])->offset += count;
				given->size += count;
				free(blockbuf);
				return byteread;
			}
		}
		else
		{
			if(count > BLOCK_SIZE)
			{
				memcpy(blockbuf,bwriteindex, BLOCK_SIZE);
				if(block_write(dataindex + fs->super->dataindex, blockbuf) == -1)
				{
					return byteread;
				}
				bwriteindex += BLOCK_SIZE; // Move the pointer in the given buffer
				byteread += BLOCK_SIZE; // Add 4096 bytes read
				count -= BLOCK_SIZE; // Size of block subtracted from total size of write 
				dataindex = fs_addblock(entryindex, ++writeindex + blockindex); 
				continue;
				// Dataindex = entry in root and blockindex + blocks we have written to alread
			}
			else if(count <= BLOCK_SIZE)
			{
				memcpy(blockbuf,bwriteindex, count);
				if(block_write(dataindex + fs->super->dataindex, blockbuf) == -1)
				{
					return byteread;
				}
				byteread += count;
				dataindex = fs_addblock(entryindex, ++writeindex + blockindex);
				continue;
			}
		}
		

	}
	// number of bytes written can be smaller than count if we run out of size
	given->size += byteread;
	(fs->files[ofindex])->offset += byteread; // Offset is moved to the number of bytes read
	free(blockbuf);
	return byteread;	
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
	int blockrem = (fs->files[ofindex])->offset % BLOCK_SIZE;
	int blockindex = ((fs->files[ofindex])->offset - blockrem) / BLOCK_SIZE;
	int offset = (fs->files[ofindex])->offset - (blockindex * BLOCK_SIZE); // Offset after being inside the right data block
	int dataindex = fs_findblockindex(entryindex, blockindex);
	// find the first datablock in file to read from by using openfile offset
	if(dataindex == -1)
	{
		return byteread;
	} //reached FAT_EOC right away with the offset
	blockrem = (fs->files[ofindex])->offset % BLOCK_SIZE;
	int block2read = ((count + offset) - blockrem) / BLOCK_SIZE;
	int readindex = 0;
	char * blockbuf = (char *) malloc(sizeof(char) * BLOCK_SIZE);
	while(dataindex != -1 && readindex < block2read + 1)
	{		
		if(block_read(dataindex + fs->super->dataindex, blockbuf) == -1)
		{
			return -1;
		}
		if(readindex == 0)
		{
			if(count > BLOCK_SIZE - offset)
			{
				byteread = BLOCK_SIZE - offset; // first block may be at an offset so we read less bytes than a full block
				offset += byteread; //offset will now be at the beginning of the next data block
				count -= byteread; // subtract count by # bytes read so far
				memcpy(buf, blockbuf+offset, BLOCK_SIZE-offset); // copy from offset in first block 
				dataindex = fs_findblockindex(entryindex, ++readindex + blockindex);
			} // Count is larger than the first block read - the offset we start at
			else if(count <= BLOCK_SIZE - offset)
			{
				byteread = count; // only read size of count
				offset += byteread; // move offset to where we ended 
				memcpy(buf, blockbuf, count); // copy size of count of blockbuf into the buf
				free(blockbuf);	// free the block buffer after copying into buf
				return byteread;
			} // Size of read (count) is less than the size the first block - offset
		} // This is our first read, so the offset matters after this we will read block by block
		else
		{
			if(count > BLOCK_SIZE)
			{
				memcpy(buf + byteread, blockbuf, BLOCK_SIZE); // copy a full block into the index of readbuf 
				byteread += BLOCK_SIZE; // read a full block of bytes
				offset += byteread; // offset moved by read
				count -= BLOCK_SIZE; // size of count remaining decremented 
				dataindex = fs_findblockindex(entryindex, ++readindex + blockindex);
				continue;
			} // size of read is larger than this block so keep going
			if(count <= BLOCK_SIZE)
			{
				memcpy(buf + byteread, blockbuf, count);
				byteread += count; // Add the last bit of count to return the number of bytes read
				offset += byteread; // This will be the last read since size of count is less than the size of a data block
				dataindex = fs_findblockindex(entryindex, ++readindex + blockindex);
				free(blockbuf);
			} // size of remaining read is smaller than a block, only copy in the remaining size
		} // Now reading block by block, either continuing 
	}
	return byteread;

}
