#! /bin/bash
set -e
make
#Run Simple Reader and Simple Writer
fs_make.x simple.fs 4096
simple_writer.x simple.fs
simple_reader.x simple.fs


fs_make.x test_EOF.fs 3
fs_make.x test.fs 1000
fs_make.x test_overwrite.fs 1000
fs_make.x corrupt1.fs 1000
fs_make.x corrupt2.fs 1000
corrupt.x
fs_make.x test_rdir.fs 1000
fs_ref.x add test.fs test.txt
stest.x



echo "===== Testing with ref ====="

fs_make.x ref.fs 100
fs_make.x t.fs 100

fs_ref.x add ref.fs test.txt
test_fs.x add t.fs test.txt

fs_ref.x ls ref.fs
test_fs.x ls t.fs

fs_ref.x cat ref.fs test.txt
test_fs.x cat t.fs test.txt 

fs_ref.x info ref.fs
test_fs.x info t.fs

fs_ref.x stat ref.fs test.txt
test_fs.x stat t.fs test.txt


fs_ref.x rm ref.fs test.txt
test_fs.x rm t.fs test.txt

fs_ref.x info ref.fs
test_fs.x info t.fs

fs_ref.x add ref.fs large.txt
test_fs.x add t.fs large.txt

fs_ref.x add ref.fs bigger.txt
test_fs.x add t.fs bigger.txt
fs_ref.x info ref.fs
test_fs.x info t.fs

fs_ref.x cat ref.fs large.txt > r.txt
test_fs.x cat t.fs large.txt > t.txt

if cmp -s "r.txt" "t.txt"; then 
    echo "Files are same"
else
    echo "Files are different"
fi

fs_ref.x cat ref.fs bigger.txt > r.txt
test_fs.x cat t.fs bigger.txt > t.txt



if cmp -s "r.txt" "t.txt"; then 
    echo "Files are same"
else
    echo "Files are different"
fi


fs_ref.x stat ref.fs bigger.txt
test_fs.x stat t.fs bigger.txt