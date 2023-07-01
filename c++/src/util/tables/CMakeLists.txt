# $Id: CMakeLists.txt 621427 2020-12-11 14:26:55Z ivanov $

NCBI_project_tags(core)
NCBI_disable_pch()
NCBI_add_library(tables)
NCBI_add_subdirectory(test)

