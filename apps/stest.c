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

int main(){
    fs_mount("test.fs");
    fs_info();
    fs_ls();
    fs_create("hi.txt");
    fs_create("idk.txt");
    fs_create("whatever.txt");
    fs_ls();
    int fd = fs_open("test.txt");
    char buf[100];
    fs_read(fd, &buf, 6);
    printf("S: %s\n", buf);
    fs_close(fd);

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
    char buf3[100];

	char data[10000];
    memset(&data, 97, 10000);
    data[9999] = '\0';
    fs_write(fd3, &data, sizeof(data));
    fs_ls();
    fs_lseek(fd3, 8191);
    fs_info();

    fs_read(fd3, &buf3, 10);
    printf("S: %s\n", buf3);
    if(!fs_umount()){
        printf("Unmount error\n");
    }
    else{
        printf("open files\n");
    }
    fs_close(fd3);
    fs_info();
    fs_delete("whatever.txt");
    fs_delete("test.txt");
    fs_ls();
    fs_umount();
}