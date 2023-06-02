#! /bin/bash
set -e
make
#Run Simple Reader and Simple Writer
fs_make.x simple.fs 4096
simple_writer.x simple.fs
simple_reader.x simple.fs


fs_make.x test_EOF.fs 3
fs_make.x test.fs 1000
fs_make.x test_rdir.fs 1000
fs_ref.x add test.fs test.txt
fs_ref.x add test.fs test2.txt
stest.x


