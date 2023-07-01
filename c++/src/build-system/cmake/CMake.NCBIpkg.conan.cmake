#############################################################################
# $Id: CMake.NCBIpkg.conan.cmake 656904 2022-09-29 21:46:57Z fukanchi $
#############################################################################
#############################################################################
##
##  NCBI C++ Toolkit Conan package adapter
##  it is used when the Toolkit is built and installed as Conan package
##    Author: Andrei Gourianov, gouriano@ncbi
##

if(NOT DEFINED NCBI_TOOLKIT_NCBIPTB_BUILD_SYSTEM_INCLUDED)
set( NCBI_TOOLKIT_NCBIPTB_BUILD_SYSTEM_INCLUDED ON)

###############################################################################
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0057 NEW)

set(NCBI_PTBCFG_PACKAGED               ON)
set(NCBI_PTBCFG_ENABLE_COLLECTOR       ON)
#set(NCBI_VERBOSE_ALLPROJECTS           OFF)
#set(NCBI_PTBCFG_ALLOW_COMPOSITE        OFF)
#set(NCBI_PTBCFG_ADDTEST                OFF)

###############################################################################
set(NCBI_PTBCFG_INSTALL_EXPORT ncbi-cpp-toolkit)

set(_listdir "${CMAKE_CURRENT_LIST_DIR}")
include(${_listdir}/CMake.NCBIptb.definitions.cmake)
include(${_listdir}/CMakeMacros.cmake)
include(${_listdir}/CMake.NCBIptb.cmake)
include(${_listdir}/CMakeChecks.compiler.cmake)
include(${_listdir}/CMake.NCBIpkg.codegen.cmake)
if(NCBI_PTBCFG_ADDTEST)
    include(${_listdir}/CMake.NCBIptb.ctest.cmake)
endif()

###############################################################################
macro(NCBIptb_setup)
    set(_listdir "${NCBI_TREE_CMAKECFG}")
    include(${_listdir}/CMake.NCBIComponents.cmake)
    include_directories(${NCBITK_INC_ROOT} ${NCBI_INC_ROOT})

    include(${_listdir}/CMake.NCBIptb.datatool.cmake)
    include(${_listdir}/CMake.NCBIptb.grpc.cmake)
    if (DEFINED NCBI_EXTERNAL_TREE_ROOT)
        set(NCBI_EXTERNAL_BUILD_ROOT ${NCBI_EXTERNAL_TREE_ROOT})
        if (EXISTS ${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.cmake)
            include(${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.cmake)
        else()
            message(FATAL_ERROR "${NCBI_PTBCFG_INSTALL_EXPORT} was not found in ${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}")
        endif()
        NCBI_import_hostinfo(${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.hostinfo)
        NCBI_process_imports(${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.imports)
        if(COMMAND conan_basic_setup)
            NCBI_verify_targets(${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.imports)
        endif()
    endif()
    if(TARGET PCRE::libpcre)
        if(NOT TARGET pcre::libpcre)
            add_library(pcre::libpcre ALIAS PCRE::libpcre)
        endif()
        if(NOT TARGET pcre::pcre)
            add_library(pcre::pcre ALIAS PCRE::libpcre)
        endif()
    endif()
    include(${_listdir}/CMakeChecks.final-message.cmake)
endmacro()
endif(NOT DEFINED NCBI_TOOLKIT_NCBIPTB_BUILD_SYSTEM_INCLUDED)
