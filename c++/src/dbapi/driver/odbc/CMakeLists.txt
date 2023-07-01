# $Id: CMakeLists.txt 637239 2021-09-09 13:34:08Z ucko $

NCBI_requires(ODBC SQLServer)
NCBI_add_library(ncbi_xdbapi_odbc)
NCBI_add_subdirectory(samples)

