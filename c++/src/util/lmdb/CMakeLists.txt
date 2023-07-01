# $Id: CMakeLists.txt 621427 2020-12-11 14:26:55Z ivanov $

NCBI_disable_pch()
NCBI_add_library(lmdb)
NCBI_add_app(lmdb_test1 lmdb_test2 lmdb_test3 lmdb_test4 lmdb_test5)
