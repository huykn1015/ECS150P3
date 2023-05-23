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
    fs_create("hi.txt");
    fs_create("idk.txt");
    fs_create("whatever.txt");
    fs_ls();
    fs_delete("whatever.txt");
    fs_ls();
    //fs_delete("hi.fs");
    //fs_info();
}