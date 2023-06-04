#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fs.h>


int test_basic(){

    printf("===== Testing Basics =====\n");
    printf("----- Mounting FS -----\n");
    if(!fs_mount("dne.fs")){
       printf("Mount error\n"); 
    }
    else{
        printf("FS does not exist\n");
    }
    fs_mount("test.fs");
    if(!fs_mount("test2.fs")){
       printf("Mount error\n"); 
    }
    else{
        printf("Another FS currently open\n");
    }
    fs_info();
    fs_ls();
    printf("\n\n");

    printf("----- Opening and reading -----\n");
    int fd = fs_open("test.txt");
    char buf[100];
    fs_read(fd, &buf, 6);
    printf("Content: %s\n", buf);
    printf("Size: %i\n", fs_stat(fd));
    printf("\n\n");
    printf("----- File Creation -----\n");
    if(fs_create("hi.txt")){
        printf("File Creation Failed\n");
    }
    else{
        printf("Created file \"Hi.txt\"\n");
    }
    if(fs_create("hi.txt")){
        printf("Filename \"Hi.txt\" taken\n");
    }
    else{
        printf("File creation error\n");
        return -1;
    }
    fs_create("idk.txt");
    fs_create("whatever.txt");
    //test write and lseek
    // writes 4 characters, attempts to read

    printf("----- lseek and write -----\n");
    int fd2 = fs_open("hi.txt");
    char buf1[4] = {'H', 'i', 'i', '\0'};
    char buf2[100];
    int w = fs_write(fd2, &buf1, 4);
    printf("written: %i\n", w);
    fs_lseek(fd2, 2);
    int ret = fs_read(fd2, &buf2, 4);
    printf("%i Bytes read\n", ret);
    printf("S: %s\n", buf2);
    fs_lseek(fd2, 0);
    fs_read(fd2, &buf2, 4);
    printf("S: %s\n", buf2);
    fs_close(fd2);
    fs_ls();
    fs_info();
    printf("\n\n");
    printf("----- Deletion -----\n");

    if(fs_delete("hi.txt")){
        printf("File Still open\n");
    }
    else{
        printf("Deletion Error\n");
    }
    fs_close(fd);
    printf("\n\n");
    fs_delete("hi.txt");
    fs_info();
    fs_ls();
    fd = fs_open("test.txt"); 
    printf("----- Umount -----\n");
    if(!fs_umount()){
        printf("Unmount error\n");
    }
    else{
        printf("Files currently open\n");
    }
    fs_close(fd);
    fs_delete("test.txt");
    if(!fs_umount()){
        printf("Unmount Successful\n");
    }
    else{
        printf("Unmount failed\n");
    }
    if(!fs_umount()){
        printf("Unmount error\n");
    }
    printf("\n\n\n\n\n\n");
    return 0;
}

int test_full_rdir(){
    printf("===== Testing creating too many files =====\n");
    if(fs_mount("test_rdir.fs")){
        printf("open failed\n");
        return -1;
    }
    printf("rdir test\n");
    char name[9] = "File_.txt";
    for( int i = 0; i < 128; i ++){
        name[4] = i;
        fs_create(name);
    }
    if(!fs_create("idk.txt")){
        printf("create error\n");
    }
    else{
        printf("rdir full\n");
    }
    fs_umount();

    printf("\n\n");
    return 0;
}

int test_open_file_max(){
    printf("===== Testing Opening too many files =====\n");
    if(fs_mount("test_rdir.fs")){
        printf("open failed\n");
        return -1;
    }
    char name[9] = "File_.txt";
    for( int i = 0; i < 32; i ++){
        name[4] = i;
        int f = fs_open(name);
        if(f == -1){
            printf("failure\n");
        }
    }
    int fd = fs_open("File1.txt");
    if(fd == -1){
        printf("open files full\n");
    }
    for( int i = 0; i < 32; i ++){
        fs_close(i);
    }
    fs_umount();
    printf("\n\n");
    return 0;
}


int test_multi_block_read_write(){
    printf("===== Testing Multiblock =====\n");
    //writes 10k 'a' across 3 blocks
    //then attemps to read 5k 'a' across 3 blocks
    if(fs_mount("test.fs")){
       printf("Mount error\n"); 
    }
    int fd3 = fs_open("whatever.txt");
    char buf3[10000];
	char data[10000];
    memset(&data, 97, 10000);
    data[9999] = '\0';
    fs_write(fd3, &data, sizeof(data));
    fs_lseek(fd3, 4095);
    fs_info();
    printf("\n\n");
    fs_read(fd3, &buf3, 5000);
    printf("S: %s\n", buf3);
    fs_lseek(fd3, 0);
    fs_read(fd3, &buf3, 9000);
    printf("S: %s\n", buf3);

    fs_lseek(fd3, 0);
    memset(&buf3, 0, 10000);
    int rnum = fs_read(fd3, &buf3, 12000);
    printf("Read %i bytes\n",rnum);
    printf("\n\n");
    fs_close(fd3);
    fs_info();
    fs_ls();
    fs_delete("whatever.txt");
    fs_info();
    fs_ls();

    if(fs_umount()){
        printf("umount failed\n");
    }
    return 0;
}



int test_no_space_write(){
    printf("===== Testing Writing with no availible space =====\n");
    if(fs_mount("test_EOF.fs")){
       printf("Mount error\n"); 
    }
    fs_create("whatever.txt");
    fs_info();
    int fd3 = fs_open("whatever.txt");
    printf("fd: %i\n", fd3);
	char data[10000];
    char buf1[4] = {'H', 'i', 'i'};
    fs_write(fd3, &buf1, 3);
    memset(&data, 97, 10000);
    data[9999] = '\0';
    int ret = fs_write(fd3, &data, sizeof(data));
    printf("written: %i\n", ret);
    fs_lseek(fd3, 0);
    char rbuf[100];
    fs_read(fd3, &rbuf, 10);
    printf("r: %s\n", rbuf);
    fs_close(fd3);
    fs_info();
    fs_ls();
    fs_umount();
    return 0;
}

int test_overwrite(){
    printf("===== Testing Overwriting Data =====\n");
    if(fs_mount("test_overwrite.fs")){
       printf("Mount error\n"); 
    }
    fs_create("whatever.txt");
    fs_info();
    int fd3 = fs_open("whatever.txt");
    printf("fd: %i\n", fd3);
	char data[10000];
    char data2[5000];
    memset(&data, 97, 10000);
    memset(&data2, 98, 5000);
    data[9999] = '\0';
    int ret = fs_write(fd3, &data, sizeof(data));
    printf("written: %i\n", ret);
    fs_lseek(fd3, 4095);
    char rbuf[100];
    fs_read(fd3, &rbuf, 10);
    printf("r: %s\n", rbuf);
    fs_lseek(fd3, 4100);
    ret = fs_write(fd3, &data2, sizeof(data2));
    printf("written: %i\n", ret);
    fs_lseek(fd3, 4095);
    memset(&rbuf, 0 , 100);
    fs_read(fd3, &rbuf, 10);
    printf("r: %s\n", rbuf);

    fs_close(fd3);
    fs_info();
    fs_ls();
    fs_umount();
    return 0;
}


int test_corrupted(){
    printf("===== Testing Opening Corrupted FS =====\n");
    if(!fs_mount("corrupt1.fs")){
       printf("Mount error\n"); 
    }
    else{
        printf("FS corrupted\n");
    }
    if(!fs_mount("corrupt2.fs")){
       printf("Mount error\n"); 
    }
    else{
        printf("FS corrupted\n");
    }
    return 0;
}


int main(){
    test_basic();
    test_full_rdir();
    test_open_file_max();
    test_multi_block_read_write();
    test_no_space_write();
    test_overwrite();
    test_corrupted();
    return 0;
}


