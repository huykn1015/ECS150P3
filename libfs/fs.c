#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>

#include "disk.h"
#include "fs.h"

/* TODO: Phase 1 */
struct super_block{
	uint8_t signature[8];
	int16_t block_count;
	int16_t root_index;
	int16_t data_start_index;
	int16_t data_block_count;
	int8_t FAT_count;
};

struct super_block * cur_super_block;
int fs_mount(const char *diskname)
{
	if (block_disk_open(diskname)){
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

int fs_umount(void)
{
	return block_disk_close();
	/* TODO: Phase 1 */
}

int fs_info(void)
{
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

