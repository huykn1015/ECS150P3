#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "disk.h"
#include "fs.h"

#define EOC 0xFFFF


/* TODO: Phase 1 */
struct super_block{
	uint8_t signature[8];
	int16_t block_count;
	int16_t root_index;
	int16_t data_start_index;
	int16_t data_block_count;
	int8_t fat_count;
	int8_t padding[4079];
};

struct rdir_entry{
	uint8_t file_name[16];
	int32_t file_size;
	uint16_t data_index;
	int8_t padding[10];
};

struct file_descriptor{
	uint8_t file_number;
	size_t offset;
};

typedef int16_t fat_entry;

struct super_block * cur_super_block;

struct file_descriptor open_files[FS_OPEN_MAX_COUNT];// = {{.file_number = 0xFF, .offset = 0}};


// HELPER FUNCTIONS

/**
 * get_fat_page - get FAT page of index
 * @index: Entry to be searched for
 * @entries: Array to write FAT page into
 * 
 * Reads the FAT page where index is located and writes it into entries
*/
void get_fat_page(int16_t index, fat_entry * entries){
	int i = 1;
	while(index > BLOCK_SIZE / 2){
		index -= BLOCK_SIZE / 2;
	}
	block_read(i, entries);
}
/**
 * write_fat_page - get FAT page of index
 * @index: Index to be updated
 * @entries: Array to write FAT page into
 * 
 * Reads the FAT page where index is located and writes it into entries
*/
void write_fat_page(int16_t index, fat_entry * entries){
	int i = 1;
	while(index > BLOCK_SIZE / 2){
		index -= BLOCK_SIZE / 2;
	}
	block_write(i, entries);
}
/**
 * get_free_fat_index - get first free fat index
 * 
 * returns index of first empty FAT entry
*/
int get_free_fat_index(){
	if(!cur_super_block){
		return -1;
	}
	for(int i = 1; i <= cur_super_block->fat_count; i++){
		fat_entry entries[BLOCK_SIZE / 2];
		block_read(i, &entries);
		for (int j = 0; j < BLOCK_SIZE / 2; j ++){
			if (entries[j] == 0){
				return ((BLOCK_SIZE / 2 * (i - 1)) + j);
			}
		}
	}
	return -1;
}
/**
 * get_free_fat - get count of free fat entries
 * 
 * returns number of free fat entries
*/
int get_free_fat(){
    int free_entries = 0;
    int loop = 0;
    if(!cur_super_block){
        return -1;
    }
    for(int i = 1; i <= cur_super_block->fat_count; i++){
        fat_entry entries[BLOCK_SIZE / 2];
        block_read(i, &entries);
        for (int j = 0; j < BLOCK_SIZE / 2; j ++){
            
            if(loop == cur_super_block->data_block_count){
                break;
            }
            loop ++;
            if (entries[j] == 0){
                free_entries++;
            }
        }
    }
    return free_entries;
}

/**
 * allocate_data_block - allocates a new data block on the FAT table
 * @fd: File desciptor of file to be extended
 * 
 * entends a file by one data block, returns the index of the new data block
 * if file is empty,
*/
int allocate_data_block(int fd){
	if(get_free_fat() == 0){
		return -1;
	}
	int fn = open_files[fd].file_number;
	struct rdir_entry rdir[FS_FILE_MAX_COUNT];
	block_read(cur_super_block->root_index, &rdir);	
	uint16_t data_index = rdir[fn].data_index;
	fat_entry entries[BLOCK_SIZE / 2];
	fat_entry old_index = data_index;
	while(data_index != EOC){
		old_index = data_index;
		get_fat_page(data_index, (fat_entry * )&entries);
		data_index = entries[data_index % (BLOCK_SIZE / 2)];
	}
	data_index = get_free_fat_index();
	if(rdir[fn].data_index == EOC){
		rdir[fn].data_index = data_index;
	}
	else{
		entries[old_index % (BLOCK_SIZE / 2)] = data_index;
		write_fat_page(old_index, (short int *)&entries);
	}
	get_fat_page(data_index, (fat_entry * )&entries);
	entries[data_index % (BLOCK_SIZE / 2)] = EOC;
	write_fat_page(data_index, (short int *)&entries);
	block_write(cur_super_block->root_index, rdir);	
	return data_index;
}

/**
 * block_index - return data index 
 * @fd: File desciptor
 * @offset: File offse
 * 
 * Returns data index of file where offset is
*/
int block_index(int fd, size_t offset){
	int fn = open_files[fd].file_number;
	struct rdir_entry entries[FS_FILE_MAX_COUNT];
	block_read(cur_super_block->root_index, &entries);
	uint16_t data_index = entries[fn].data_index;
	if(entries[fn].file_size < offset){
		return -1;
	}
	int block_count = offset / BLOCK_SIZE;
	while(data_index != EOC && block_count > 0){
		fat_entry entries[BLOCK_SIZE / 2];
		fat_entry old_index = data_index;
		get_fat_page(data_index, (fat_entry * )&entries);
		data_index = entries[data_index % (BLOCK_SIZE / 2)];
		block_count--;
	}
	return data_index + cur_super_block->root_index + 1;
}

/**
 * create_file_entry - create file entry in root dir
 * @entries: Root dir entries array
 * @filename: Name of new file
 * 
 * Creates a new entry in root directory for the file
 * Returns -1 if unsuccessful, Returns 0 if successful 
*/
int create_file_entry(struct rdir_entry * entries, const char * filename){
	int i;
	bool rdir_full = true;
	if(!cur_super_block){
		return -1;
	}
	for (i = 0; i < FS_FILE_MAX_COUNT; i++){
		if (entries[i].file_name[0] == 0){
			rdir_full = false;
			break;
		}
	}
	if(rdir_full){
		return -1;
	}
	strcpy(entries[i].file_name, filename);
	entries[i].data_index = EOC;
	entries[i].file_size = 0;
	return 0;
}

/**
 * clear_fat - clears all FAT entries of a file
 * @data_index: index of FAT entry of the file
 * 
 * sets all fat entries of a file to 0
*/
int clear_fat(uint16_t data_index){
	while(data_index != EOC){
		fat_entry entries[BLOCK_SIZE / 2];
		fat_entry old_index = data_index;
		get_fat_page(data_index, (fat_entry * )&entries);
		data_index = entries[data_index % (BLOCK_SIZE / 2)];
		entries[old_index] = 0;	
		write_fat_page(old_index, (fat_entry *)&entries);
	}
	return 0;
}

/**
 * get_free_rdir_count - get count of free root dir entries
 * 
 * returns number of empty entries in root dir
*/
int get_free_rdir_count(){
	struct rdir_entry entries[FS_FILE_MAX_COUNT];
	int free_files = 0;
	if(!cur_super_block){
		return -1;
	}
	block_read(cur_super_block->root_index, &entries);
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++){
		if (entries[i].file_name[0] == 0){
			free_files++;
		}
	}
	return free_files;
}



/**
 * delete_rdir_entry - delete root directory entry
 * @entries: Array of root directoy entries
 * @filename: Name of file to be deleted
 * 
 * erases the file from the array of root directory entries
*/
int delete_rdir_entry(struct rdir_entry * entries, const char * filename){
	int i;
	bool missing = true;
	for (i = 0; i < FS_FILE_MAX_COUNT; i++){
		if (strncmp(entries[i].file_name, filename, strlen(filename))){
			missing = false;
			break;
		}
	}
	if(missing){
		return -1;
	}
	clear_fat(entries[i].data_index);
	memset(&entries[i], 0, sizeof(struct rdir_entry));
	return 0;
}


/**
 * next_block - returns index of next block
 * @fd: File descriptor
 * @cur_index: current index
 * 
 * returns the index of the next block, if cur_index is the last block
 * this function will allocate a new block and return its index instead
*/
int next_block(int fd, int cur_index){
	cur_index -= cur_super_block->root_index + 1;
	fat_entry entries[BLOCK_SIZE / 2];
	get_fat_page(cur_index, (fat_entry * )&entries);
	uint16_t next_index = entries[cur_index % (BLOCK_SIZE / 2)];
	if(next_index == EOC){
		next_index = allocate_data_block(fd);
		if (next_index == -1){
			return -1;
		}
	}
	return next_index + cur_super_block->root_index + 1;
}



/**
 * increase_file_size	- change file size
 * @fd: File Descriptor
 * @inc: amount to increase file  size by
 * 
 * increases the file size of a file in the root directory
*/
void increase_file_size(int fd, int inc){
	int fn = open_files[fd].file_number;
	struct rdir_entry entries[FS_FILE_MAX_COUNT];
	block_read(cur_super_block->root_index, &entries);
	entries[fn].file_size += inc;	
	block_write(cur_super_block->root_index, entries);
}




// API FUNCTIONS

int fs_mount(const char *diskname){
	if(cur_super_block){
		return -1;
	}
	if (block_disk_open(diskname)){
		cur_super_block = NULL;
		return -1;
	}
	cur_super_block = malloc(sizeof(uint8_t) * BLOCK_SIZE);
	block_read(0,cur_super_block);
	if (strncmp((char * )cur_super_block->signature, "ECS150FS", 8)){
		return -1;
	} 
	if((int)block_disk_count() != (int)cur_super_block->block_count){
		return -1;
	}
	for(int i = 0; i <FS_OPEN_MAX_COUNT; i ++ ){
		open_files[i].file_number = 0xFF;
	}
	return 0;

}


int fs_umount(void){
	if(!cur_super_block){
		return -1;
	}
	for (int i = 0; i < FS_OPEN_MAX_COUNT; i++){
		if(open_files[i].file_number != 0xFF){
			return -1;
		}
	}
	memset(cur_super_block, 0, sizeof(uint8_t) * BLOCK_SIZE);
	cur_super_block = NULL;
	return block_disk_close();
	/* TODO: Phase 1 */
}

int fs_info(void){
	if(!cur_super_block){
		return -1;
	}
	printf("FS Info:\n");
	printf("total_blk_count=%i\n", cur_super_block->block_count);
	printf("fat_blk_count=%i\n", cur_super_block->fat_count);
	printf("rdir_blk=%i\n", cur_super_block->root_index);
	printf("data_blk=%i\n", cur_super_block->data_start_index);
	printf("data_blk_count=%i\n", cur_super_block->data_block_count);
	printf("fat_free_ratio=%i/%i\n", get_free_fat(), cur_super_block-> data_block_count);
	printf("rdir_free_ratio=%i/%i\n", get_free_rdir_count(), FS_FILE_MAX_COUNT);
	return 0;
}

int fs_create(const char *filename){
	struct rdir_entry entries[FS_FILE_MAX_COUNT];
	block_read(cur_super_block->root_index, &entries);
	if (!cur_super_block || strlen(filename) > FS_FILENAME_LEN){
		return -1;
	}
	if(create_file_entry((struct rdir_entry *)&entries, filename)){
		return -1;
	}
	block_write(cur_super_block->root_index, &entries);
	return 0;
}

int fs_delete(const char *filename){
	if (!cur_super_block || strlen(filename) > FS_FILENAME_LEN){
		return -1;
	}
	struct rdir_entry entries[FS_FILE_MAX_COUNT];
	block_read(cur_super_block->root_index, &entries);
	if(delete_rdir_entry((struct rdir_entry *)&entries, filename)){
		return -1;
	}
	block_write(cur_super_block->root_index, entries);
	return 0;
}

int fs_ls(void){
	struct rdir_entry entries[FS_FILE_MAX_COUNT];
	block_read(cur_super_block->root_index, &entries);
	printf("FS ls:\n");
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++){
		if (entries[i].file_name[0] != 0){
			printf("file: %s, size: %i, data_blk: %u\n", entries[i].file_name, 
				   entries[i].file_size, entries[i].data_index);
		}
	}
	return 0;
}

int fs_open(const char *filename){
	if (cur_super_block == NULL || strlen(filename) >= FS_FILENAME_LEN){
		return -1;
	}
	struct rdir_entry entries[FS_FILE_MAX_COUNT];
	block_read(cur_super_block->root_index, &entries);
	int fd;
	bool open_files_full = true;
	for(fd = 0; fd < FS_OPEN_MAX_COUNT; fd++){
		if(open_files[fd].file_number == 0xFF){
			open_files_full = false;
			break;
		}
	}
	if(open_files_full){
		return -1;
	}
	int i;
	bool missing = true;
	for (i = 0; i < FS_FILE_MAX_COUNT; i++){
		if (!strncmp(entries[i].file_name, filename, strlen(filename))){
			missing = false;
			break;
		}
	}
	if(missing){
		return -1;
	}
	open_files[fd].file_number = i;
	open_files[fd].offset = 0;
	return fd;
}

int fs_close(int fd){
	if (cur_super_block == NULL || open_files[fd].file_number == 0xFF){
		return -1;
	}
	open_files[fd].file_number = 0xFF;
	open_files[fd].offset = 0;
	return 0;
}

int fs_stat(int fd){
	struct rdir_entry entries[FS_FILE_MAX_COUNT];
	block_read(cur_super_block->root_index, &entries);
	return entries[open_files[fd].file_number].file_size;
}

int fs_lseek(int fd, size_t offset)
{
	if(fd >= FS_OPEN_MAX_COUNT || (open_files[fd].file_number == 0xFF)){
		return -1;
	}
	open_files[fd].offset = offset;
	return 0;
}

int fs_write(int fd, void *buf, size_t count){
	if(fd < 0 || fd >= FS_OPEN_MAX_COUNT || buf == NULL || cur_super_block == NULL){
		return -1;
	}
	if(count == 0){
		return 0;
	}
	int wcount = 0;
    int fn = open_files[fd].file_number;
    struct rdir_entry rdir[FS_FILE_MAX_COUNT];
    block_read(cur_super_block->root_index, &rdir);	
    uint16_t data_index = rdir[fn].data_index;
    if(data_index == EOC){
        int ret = allocate_data_block(fd);
		if (ret == -1){
			return 0;
		}
    }
	uint8_t wbuf[BLOCK_SIZE];
	if (count + (open_files[fd].offset % BLOCK_SIZE) > BLOCK_SIZE){
		block_read(block_index(fd, open_files[fd].offset), &wbuf);
		memcpy(&wbuf + open_files[fd].offset , buf, BLOCK_SIZE - (open_files[fd].offset % BLOCK_SIZE));
		block_write(block_index(fd,open_files[fd].offset), &wbuf);
		increase_file_size(fd, BLOCK_SIZE - (open_files[fd].offset % BLOCK_SIZE));
	}
	else{
		block_read(block_index(fd, open_files[fd].offset), wbuf);
		memcpy(&wbuf + open_files[fd].offset , buf, count);
		block_write(block_index(fd,0), wbuf);
		increase_file_size(fd, count);
		open_files[fd].offset += count;
		return count;
	}
	wcount = BLOCK_SIZE - open_files[fd].offset % BLOCK_SIZE;
	buf += wcount;
	open_files[fd].offset += wcount;
	int new_index = next_block(fd, block_index(fd, 0));
	while (wcount + BLOCK_SIZE < count){
		if(new_index == -1){
			return wcount;
		}
		memcpy(&wbuf, buf, BLOCK_SIZE);
		buf += BLOCK_SIZE;
		block_write(new_index, wbuf);
		increase_file_size(fd, BLOCK_SIZE);
		new_index = next_block(fd, new_index);
		wcount+= BLOCK_SIZE;
	}
	memset(&wbuf, 0, sizeof(wbuf));
	memcpy(&wbuf, buf, count - wcount);
	block_write(new_index, &wbuf);
	increase_file_size(fd, count - wcount);
	wcount += count - wcount;
	return wcount;
}



int fs_read(int fd, void *buf, size_t count){
	if(fd < 0 || fd >= FS_OPEN_MAX_COUNT || buf == NULL || cur_super_block == NULL){
		return -1;
	}
	int rcount = 0;
	int8_t inbuf[BLOCK_SIZE];
    int fn = open_files[fd].file_number;
    struct rdir_entry rdir[FS_FILE_MAX_COUNT];
    block_read(cur_super_block->root_index, &rdir);	
	int file_size = rdir[fn].file_size;
	if(count + (open_files[fd].offset % BLOCK_SIZE) > BLOCK_SIZE){
		block_read(block_index(fd, open_files[fd].offset), &inbuf);
		memcpy(buf, (char *) &inbuf[(open_files[fd].offset % BLOCK_SIZE)], BLOCK_SIZE - (open_files[fd].offset % BLOCK_SIZE));
	}
	else{
		block_read(block_index(fd, open_files[fd].offset), &inbuf);
		memcpy(buf, (char *) &inbuf[(open_files[fd].offset % BLOCK_SIZE)], count);
		if(open_files[fd].offset + count > file_size){
			int ret = file_size - open_files[fd].offset;
			open_files[fd].offset = file_size;
			return ret;
		}
		open_files[fd].offset += count;
		return count;
	}
	rcount = BLOCK_SIZE - (open_files[fd].offset % BLOCK_SIZE);
	buf += rcount;
	open_files[fd].offset += rcount;
	while (rcount + BLOCK_SIZE < count){
		block_read(block_index(fd, open_files[fd].offset), &inbuf);
		memcpy(buf, &inbuf, BLOCK_SIZE);
		if(BLOCK_SIZE + open_files[fd].offset > file_size){
			int ret = rcount + (file_size - open_files[fd].offset);
			open_files[fd].offset = file_size;
			return ret;
		}
		buf += BLOCK_SIZE;
		rcount += BLOCK_SIZE;
		open_files[fd].offset += BLOCK_SIZE;
	}
	memset(&inbuf, 0, sizeof(inbuf));
	block_read(block_index(fd, open_files[fd].offset), &inbuf);
	memcpy(buf, inbuf, count - rcount);
	if((count - rcount) + open_files[fd].offset > file_size){
		int ret = rcount + (file_size - open_files[fd].offset);
		open_files[fd].offset = file_size;
		return ret;
	}
	open_files[fd].offset += count - rcount;
	return rcount;
}
