#############################################################################
# $Id: CMakeLists.txt 665496 2023-04-05 16:13:04Z blastadm $
#############################################################################

cmake_minimum_required(VERSION 3.7)
if ("${PROJECT_NAME}" STREQUAL "")
    project(ncbi_cpp)
endif()
include(${CMAKE_CURRENT_LIST_DIR}/build-system/cmake/CMakeLists.top_builddir.txt)
