# $Id: CMakeLists.txt 621735 2020-12-16 15:47:41Z ivanov $

NCBI_requires(LMDB)
NCBI_add_library(writedb)
NCBI_add_subdirectory(unit_test)

