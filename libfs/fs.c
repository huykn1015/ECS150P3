#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>

#include "disk.h"
#include "fs.h"

#define MAX_FILE_COUNT 128
/* TODO: Phase 1 */
struct super_block{
	uint8_t signature[8];
	int16_t block_count;
	int16_t root_index;
	int16_t data_start_index;
	int16_t data_block_count;
	int8_t FAT_count;
	int8_t padding[4079];
};

struct rdir_entry{
	uint8_t file_name[16];
	int32_t file_size;
	int16_t data_index;
	int8_t padding[10];
};

typedef int16_t FAT_entry;

struct super_block * cur_super_block;

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
	struct rdir_entry entries[MAX_FILE_COUNT];
	int free_files = 0;
	if(!cur_super_block){
		return -1;
	}
	block_read(cur_super_block->root_index, &entries);
	for (int i = 0; i < MAX_FILE_COUNT; i++){
		if (entries[i].file_name[0] == 0){
			free_files++;
		}
	}
	return free_files;
}

int get_free_FAT(){
	int free_entries = 0;
	if(!cur_super_block){
		return -1;
	}
	for(int i = 1; i <= cur_super_block->FAT_count; i++){
		FAT_entry entries[BLOCK_SIZE / 2];
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
	printf("fat_blk_count=%i\n", cur_super_block->FAT_count);
	printf("rdir_block=%i\n", cur_super_block->root_index);
	printf("data_blk=%i\n", cur_super_block->data_start_index);
	printf("data_blk_count=%i\n", cur_super_block->data_block_count);
	printf("fat_free_ratio=%i/%i\n", get_free_FAT(), cur_super_block->FAT_count * 4096 / 2);
	printf("rdir_free_ratio=%i/%i\n", get_free_rdir_count(), MAX_FILE_COUNT);
	/* TODO: Phase 1 */
	return 0;
}

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
	return 0;
}

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
	return 0;
}

int fs_ls(void)
{
	/* TODO: Phase 2 */
	return 0;
}

int fs_open(const char *filename)
{
	/* TODO: Phase 3 */
	return 0;
}

int fs_close(int fd)
{
	/* TODO: Phase 3 */
	return 0;
}

int fs_stat(int fd)
{
	/* TODO: Phase 3 */
	return 0;
}

int fs_lseek(int fd, size_t offset)
{
	/* TODO: Phase 3 */
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

