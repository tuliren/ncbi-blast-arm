# $Id: CMakeLists.txt 621427 2020-12-11 14:26:55Z ivanov $

NCBI_disable_pch()
NCBI_add_library(regexp)
NCBI_add_app(test_pcre)

