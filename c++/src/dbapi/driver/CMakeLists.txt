# $Id: CMakeLists.txt 627169 2021-03-10 15:19:46Z ucko $

NCBI_add_library(dbapi_driver)
NCBI_add_subdirectory(
  util ctlib ftds100 ftds-default mysql odbc samples
)

