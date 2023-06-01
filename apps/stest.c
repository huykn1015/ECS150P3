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
    fs_ls();
    fs_delete("text.txt");
    fs_ls();
    fs_info();
    if(!fs_umount()){
        printf("Unmount error\n");
    }
    else{
        printf("Files currently open\n");
    }
    fs_close(fd);
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

int test_inter_block_write(){
    return 0;
}

int test_inter_block_read(){
    return 0;
}

int test_multi_block_write(){
    return 0;
}

int test_multi_block_read(){
    return 0;
}

int test_eof_read(){
    return 0;
}

int test_no_space_write(){
    return 0;
}


int main(){

    test_basic();
    return 0;




    int fd2 = fs_open("hi.txt");
    char buf1[4] = {'H', 'i', 'i', '\0'};
    char buf2[100];
    fs_write(fd2, &buf1, 4);
    fs_read(fd2, &buf2, 4);
    printf("S: %s\n", buf2);
    fs_lseek(fd2, 0);
    fs_read(fd2, &buf2, 4);
    printf("S: %s\n", buf2);
    fs_close(fd2);

    int fd3 = fs_open("whatever.txt");
    char buf3[5000];

	char data[10000];
    memset(&data, 97, 10000);
    data[9999] = '\0';
    fs_write(fd3, &data, sizeof(data));
    fs_lseek(fd3, 4095);
    fs_info();

    fs_read(fd3, &buf3, 5000);
    printf("S: %s\n", buf3);
    fs_close(fd3);
    fs_delete("test.txt");
    fs_ls();
    fs_info();
    fs_umount();
}


