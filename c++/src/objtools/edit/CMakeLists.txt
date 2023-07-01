# $Id: CMakeLists.txt 658825 2022-11-09 12:44:10Z ivanov $

NCBI_add_library(edit)
NCBI_add_subdirectory(unit_test)
NCBI_add_subdirectory(pubmed_fetch)
NCBI_add_subdirectory(pubmed_citmatch)
NCBI_add_subdirectory(test_remote_updater)
