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

struct file_descriptor open_files[FS_OPEN_MAX_COUNT] = {{.file_number = 0xFF, .offset = 0}};


int fs_mount(const char *diskname)
{
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
		printf("x: %i\n", block_disk_count());
		return -1;
	}
	return 0;

}

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

int get_free_fat(){
	int free_entries = 0;
	if(!cur_super_block){
		return -1;
	}
	for(int i = 1; i <= cur_super_block->fat_count; i++){
		fat_entry entries[BLOCK_SIZE / 2];
		block_read(i, &entries);
		for (int j = 0; j < BLOCK_SIZE / 2; j ++){
			if (entries[j] == 0){
				free_entries++;
			}
		}
	}
	return free_entries;
}

int fs_umount(void)
{
	for (int i = 0; i < FS_OPEN_MAX_COUNT; i++){
		if(open_files[i].file_number != 0xFF){
			return -1;
		}
	}
	memset(cur_super_block, 0, sizeof(struct super_block));
	return block_disk_close();
	/* TODO: Phase 1 */
}

int fs_info(void)
{
	if(!cur_super_block){
		return -1;
	}
	printf("FS Info:\n");
	printf("total_blk_count=%i\n", cur_super_block->block_count);
	printf("fat_blk_count=%i\n", cur_super_block->fat_count);
	printf("rdir_block=%i\n", cur_super_block->root_index);
	printf("data_blk=%i\n", cur_super_block->data_start_index);
	printf("data_blk_count=%i\n", cur_super_block->data_block_count);
	printf("fat_free_ratio=%i/%i\n", get_free_fat(), cur_super_block->fat_count * 4096 / 2);
	printf("rdir_free_ratio=%i/%i\n", get_free_rdir_count(), FS_FILE_MAX_COUNT);
	/* TODO: Phase 1 */
	return 0;
}

int get_free_fat_index(){
	if(!cur_super_block){
		return -1;
	}
	for(int i = 1; i <= cur_super_block->fat_count; i++){
		fat_entry entries[BLOCK_SIZE / 2];
		block_read(i, &entries);
		for (int j = 0; j < BLOCK_SIZE / 2; j ++){
			if (entries[j] == 0){
				return (BLOCK_SIZE / 2 * (i - 1) + j);
			}
		}
	}
	return -1;
}

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

void get_fat_page(int16_t index, fat_entry * entries){
	int i = 1;
	while(index > BLOCK_SIZE / 2){
		index -= BLOCK_SIZE / 2;
	}
	block_read(i, entries);
}

int clear_fat(uint16_t data_index){
	while(data_index != EOC){
		fat_entry entries[BLOCK_SIZE / 2];
		fat_entry old_index = data_index;
		get_fat_page(data_index, (fat_entry * )&entries);
		data_index = data_index % (BLOCK_SIZE / 2);
		entries[old_index] = 0;	
	}
	return 0;
}

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

int fs_create(const char *filename)
{
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

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
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

int fs_ls(void)
{
	/* TODO: Phase 2 */
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

int fs_open(const char *filename)
{
	/* TODO: Phase 3 */
	if (cur_super_block == NULL || strlen(filename) >= FS_FILENAME_LEN){
		return -1;
	}
	struct rdir_entry entries[FS_FILE_MAX_COUNT];
	block_read(cur_super_block->root_index, &entries);
	//get first open fd, or returns -1 if none is found
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

	//assigns file number to fd
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
	open_files[fd].file_number = i;
	open_files[fd].offset = 0;

	return fd;
}

int fs_close(int fd)
{
	/* TODO: Phase 3 */
	if (cur_super_block == NULL || open_files[fd].file_number == 0xFF){
		return -1;
	}
	open_files[fd].file_number == 0xFF;
	open_files[fd].offset = 0;
	return 0;
}

int fs_stat(int fd)
{
	struct rdir_entry entries[FS_FILE_MAX_COUNT];
	block_read(cur_super_block->root_index, &entries);
	return entries[open_files[fd].file_number].file_size;
}

int fs_lseek(int fd, size_t offset)
{
	/* TODO: Phase 3 */
	if(fd >= FS_OPEN_MAX_COUNT || (open_files[fd].file_number == 0xFF)){
		return -1;
	}
	open_files[fd].offset = offset;
	return 0;
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

