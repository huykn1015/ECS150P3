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
    fs_mount("test.fs");
    if(!fs_mount("test2.fs")){
       printf("Mount error\n"); 
    }
    else{
        printf("Another FS currently open\n");
    }
    fs_info();
    fs_ls();
    int fd = fs_open("test.txt");
    char buf[100];
    fs_read(fd, &buf, 6);
    printf("Read: %s\n", buf);
    printf("Size: %i\n", fs_stat(fd));
    fs_create("hi.txt");
    fs_create("idk.txt");
    fs_create("whatever.txt");
    //test write and lseek
    // writes 4 characters, attempts to read
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
    fs_delete("test.txt");
    if(!fs_umount()){
        printf("Unmount error\n");
    }
    else{
        printf("Files currently open\n");
    }
    fs_close(fd);
    fs_info();
    fs_ls();
    fs_delete("hi.txt");
    fs_info();
    fs_ls();

    if(!fs_umount()){
        printf("Unmount Successful\n");
    }
    else{
        printf("Unmount failed\n");
    }
    if(!fs_umount()){
        printf("Unmount error\n");
    }
    return 0;
}

int test_full_rdir(){
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
    return 0;
}

int test_open_file_max(){
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
    return 0;
}


int test_multi_block_read_write(){
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

    fs_read(fd3, &buf3, 5000);
    printf("S: %li\n", strlen(buf3));
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


int test_eof_read(){
    return 0;
}

int test_no_space_write(){
    printf("=========\n");
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
    fs_read(fd3, &rbuf, 100);
    printf("r: %s\n", rbuf);
    fs_close(fd3);
    fs_info();
    fs_ls();
    fs_umount();
    return 0;
}


int main(){

    test_basic();
    test_full_rdir();
    test_open_file_max();
    test_multi_block_read_write();
    test_no_space_write();
    return 0;
}


