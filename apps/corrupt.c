
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "disk.h"

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


int main(){
    printf("Corrupting FS\n");
    block_disk_open("corrupt1.fs");
	struct super_block * cur_super_block = malloc(sizeof(uint8_t) * BLOCK_SIZE);
	block_read(0,(void *) cur_super_block);
    char * wrong = "ECS150SF";
    memcpy((char*)cur_super_block->signature, wrong, 8);
    block_write(0,cur_super_block);
    block_disk_close();

    block_disk_open("corrupt2.fs");
	block_read(0,(void *) cur_super_block);
    cur_super_block->block_count += 1;
    block_write(0,cur_super_block);
    block_disk_close();
    printf("FS Currupted\n");
}
