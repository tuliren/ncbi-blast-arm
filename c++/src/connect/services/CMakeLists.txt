# $Id: CMakeLists.txt 621390 2020-12-10 18:54:37Z ivanov $

NCBI_project_tags(grid)
NCBI_add_library(xconnserv ncbi_xcache_netcache ncbi_xblobstorage_netcache)
NCBI_add_subdirectory(test)

