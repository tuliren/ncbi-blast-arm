# $Id: CMakeLists.txt 656900 2022-09-29 21:45:59Z fukanchi $

NCBI_add_library(serial cserial)
NCBI_add_subdirectory(datatool test soap grpc_integration)


