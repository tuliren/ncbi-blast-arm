# $Id: CMakeLists.txt 635947 2021-08-12 15:01:40Z gouriano $

NCBI_project_tags(web)
NCBI_add_library(cgi fcgi fcgi_mt)
NCBI_add_subdirectory(test)
#NCBI_add_subdirectory(demo)

