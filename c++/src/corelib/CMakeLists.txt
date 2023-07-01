# $Id: CMakeLists.txt 636432 2021-08-24 14:28:12Z grichenk $

NCBI_project_tags(core)
NCBI_add_library(corelib test_mt test_boost)
NCBI_add_subdirectory(test)
NCBI_add_subdirectory(jaeger)
