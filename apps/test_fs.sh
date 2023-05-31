#! /bin/bash

#make new disk and run stest
fs_make.x test.fs 4096
fs_ref.x add test.fs test.txt
stest.x


