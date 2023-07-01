# $Id: CMakeLists.txt 621427 2020-12-11 14:26:55Z ivanov $

NCBI_project_tags(core)
NCBI_requires(PCRE)
NCBI_add_library(xregexp xregexp_template_tester)

