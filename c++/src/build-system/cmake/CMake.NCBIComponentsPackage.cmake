#############################################################################
# $Id: CMake.NCBIComponentsPackage.cmake 662761 2023-02-06 19:10:00Z ivanov $
#############################################################################

##
## NCBI CMake components description - download/build using Conan
##
##
## As a result, the following variables should be defined for component XXX
##  NCBI_COMPONENT_XXX_FOUND
##  NCBI_COMPONENT_XXX_INCLUDE
##  NCBI_COMPONENT_XXX_DEFINES
##  NCBI_COMPONENT_XXX_LIBS
##  HAVE_LIBXXX
##  HAVE_XXX


#set(NCBI_TRACE_ALLCOMPONENTS ON)
#set(NCBI_TRACE_COMPONENT_BZ2 ON)

if(NCBI_PTBCFG_USECONAN AND COMMAND conan_define_targets)
    set(__silent ${CONAN_CMAKE_SILENT_OUTPUT})	 
    set(CONAN_CMAKE_SILENT_OUTPUT TRUE)	 
    conan_define_targets()
    set(CONAN_CMAKE_SILENT_OUTPUT ${__silent})
endif()
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

include(CheckIncludeFile)
include(CheckSymbolExists)
#############################################################################
function(NCBI_define_Pkgcomponent)
    cmake_parse_arguments(DC "" "NAME;PACKAGE;FIND" "REQUIRES" ${ARGN})

    if("${DC_NAME}" STREQUAL "")
        message(FATAL_ERROR "No component name")
    endif()
    if("${DC_PACKAGE}" STREQUAL "")
        message(FATAL_ERROR "No package name")
    endif()
    if(WIN32)
        set(_prefix "")
        set(_suffixes ${CMAKE_STATIC_LIBRARY_SUFFIX})
    else()
        set(_prefix lib)
        if(NCBI_PTBCFG_COMPONENT_StaticComponents)
            set(_suffixes ${CMAKE_STATIC_LIBRARY_SUFFIX} ${CMAKE_SHARED_LIBRARY_SUFFIX})
        else()
            if(BUILD_SHARED_LIBS OR TRUE)
                set(_suffixes ${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_STATIC_LIBRARY_SUFFIX})
            else()
                set(_suffixes ${CMAKE_STATIC_LIBRARY_SUFFIX} ${CMAKE_SHARED_LIBRARY_SUFFIX})
            endif()
        endif()
    endif()
    string(TOUPPER ${DC_PACKAGE} _UPPACKAGE)
    set(DC_REQUIRES ${DC_PACKAGE} ${DC_REQUIRES})

    set(_found NO)
    set(NCBI_COMPONENT_${DC_NAME}_FOUND NO PARENT_SCOPE)
    if(NCBI_COMPONENT_${DC_NAME}_DISABLED)
        message("DISABLED ${DC_NAME}")
    elseif(DEFINED CONAN_${_UPPACKAGE}_ROOT)
        message(STATUS "Found ${DC_NAME}: ${CONAN_${_UPPACKAGE}_ROOT}")
        set(_found YES)
        set(NCBI_COMPONENT_${DC_NAME}_FOUND YES PARENT_SCOPE)
        set(_include ${CONAN_INCLUDE_DIRS_${_UPPACKAGE}})
        set(_defines ${CONAN_DEFINES_${_UPPACKAGE}})
        set(NCBI_COMPONENT_${DC_NAME}_INCLUDE ${_include} PARENT_SCOPE)
        set(NCBI_COMPONENT_${DC_NAME}_DEFINES ${_defines} PARENT_SCOPE)

        set(_all_libs "")
        foreach(_package IN LISTS DC_REQUIRES)
            string(TOUPPER ${_package} _UPPACKAGE)
            if(DEFINED CONAN_${_UPPACKAGE}_ROOT)
                if(TARGET CONAN_PKG::${_package})
                    list(APPEND _all_libs CONAN_PKG::${_package})
                else()
                    if(NOT "${CONAN_LIB_DIRS_${_UPPACKAGE}}" STREQUAL "" AND NOT "${CONAN_LIBS_${_UPPACKAGE}}" STREQUAL "")
                        foreach(_lib IN LISTS CONAN_LIBS_${_UPPACKAGE})
                            set(_this_found NO)
                            foreach(_dir IN LISTS CONAN_LIB_DIRS_${_UPPACKAGE})
                                foreach(_sfx IN LISTS _suffixes)
                                    if(EXISTS ${_dir}/${_prefix}${_lib}${_sfx})
                                        list(APPEND _all_libs ${_dir}/${_prefix}${_lib}${_sfx})
                                        set(_this_found YES)
                                        if(NCBI_TRACE_COMPONENT_${DC_NAME} OR NCBI_TRACE_ALLCOMPONENTS)
                                            message("${DC_NAME}: found:  ${_dir}/${_prefix}${_lib}${_sfx}")
                                        endif()
                                        break()
                                    endif()
                                endforeach()
                                if(_this_found)
                                    break()
                                endif()
                            endforeach()
                            if(NOT _this_found)
                                list(APPEND _all_libs ${_lib})
                            endif()
                        endforeach()
                    endif()
                endif()
            else()
                message("ERROR: ${DC_NAME}: ${_package} not found")
            endif()
        endforeach()
        set(NCBI_COMPONENT_${DC_NAME}_LIBS ${_all_libs} PARENT_SCOPE)
        if(MSVC)
            set(NCBI_COMPONENT_${DC_NAME}_BINPATH ${CONAN_BIN_DIRS_${_UPPACKAGE}} PARENT_SCOPE)
        endif()
    elseif(NOT "${DC_FIND}" STREQUAL "")
       set(CONAN_CMAKE_SILENT_OUTPUT TRUE)
if(OFF)
        NCBIcomponent_find_package(${DC_NAME} ${DC_FIND} CONFIG)
        if(NOT NCBI_COMPONENT_${DC_NAME}_FOUND)
            file(GLOB _files "${CMAKE_BINARY_DIR}/Find${DC_FIND}*")
            list(LENGTH _files _count)
            if(NOT ${_count} EQUAL 0)
                NCBIcomponent_find_package(${DC_NAME} ${DC_FIND})
            endif()
        endif()
else()
        NCBIcomponent_find_package(${DC_NAME} ${DC_FIND})
endif()
        if(NCBI_COMPONENT_${DC_NAME}_FOUND)
            set(_found YES)
            set(_include ${NCBI_COMPONENT_${DC_NAME}_INCLUDE})
            set(_defines ${NCBI_COMPONENT_${DC_NAME}_DEFINES})
            set(_all_libs ${NCBI_COMPONENT_${DC_NAME}_LIBS})
            if("${_all_libs}" STREQUAL "")
                if(TARGET "${DC_FIND}::${DC_FIND}")
                    set(_all_libs ${DC_FIND}::${DC_FIND})
                endif()
            endif()

            if("${_all_libs}" STREQUAL "" AND "${_include}" STREQUAL "")
                set(NCBI_COMPONENT_${DC_NAME}_FOUND NO PARENT_SCOPE)
                set(_found NO)
            else()
                set(NCBI_COMPONENT_${DC_NAME}_FOUND YES PARENT_SCOPE)
                set(NCBI_COMPONENT_${DC_NAME}_VERSION ${NCBI_COMPONENT_${DC_NAME}_VERSION} PARENT_SCOPE)
                set(NCBI_COMPONENT_${DC_NAME}_INCLUDE ${_include} PARENT_SCOPE)
                set(NCBI_COMPONENT_${DC_NAME}_LIBS ${_all_libs} PARENT_SCOPE)
                set(NCBI_COMPONENT_${DC_NAME}_DEFINES ${NCBI_COMPONENT_${DC_NAME}_DEFINES} PARENT_SCOPE)
            endif()
        endif()
    endif()

    if(_found)
        string(TOUPPER ${DC_NAME} _upname)
        set(HAVE_LIB${_upname} 1 PARENT_SCOPE)
        string(REPLACE "." "_" _altname ${_upname})
        set(HAVE_${_altname} 1 PARENT_SCOPE)

        list(APPEND NCBI_ALL_COMPONENTS ${DC_NAME})
        list(REMOVE_DUPLICATES NCBI_ALL_COMPONENTS)
        set(NCBI_ALL_COMPONENTS ${NCBI_ALL_COMPONENTS} PARENT_SCOPE)
        if(NCBI_TRACE_COMPONENT_${DC_NAME} OR NCBI_TRACE_ALLCOMPONENTS)
            message("----------------------")
            message("NCBI_define_Pkgcomponent: ${DC_NAME}")
            message("include: ${_include}")
            message("libs:    ${_all_libs}")
            message("defines: ${_defines}")
            message("----------------------")
        endif()
    else()
        if(NCBI_TRACE_COMPONENT_${DC_NAME} OR NCBI_TRACE_ALLCOMPONENTS)
            message("NOT FOUND ${DC_NAME}")
        endif()
    endif()
endfunction()

#############################################################################
function(NCBI_verify_targets _file)
    if(NOT EXISTS ${_file})
        return()
    endif()

    file(STRINGS ${_file} _targets)
    if("${_targets}" STREQUAL "")
        return()
    endif()
    foreach( _prj IN LISTS _targets)
        if(NOT TARGET ${_prj})
            continue()
        endif()
        get_target_property(_deps ${_prj} INTERFACE_LINK_LIBRARIES)
        if(_deps)
            foreach( _dep IN LISTS _deps)
                if(TARGET ${_dep})
                    continue()
                endif()
                if("${_dep}" MATCHES ".+::.+")
                    string(REPLACE "::" ";" _names ${_dep})
                    list(GET _names 0 _name)
                    string(TOUPPER ${_name} _UPPACKAGE)
                    if(NOT DEFINED CONAN_${_UPPACKAGE}_ROOT)
                        list(GET _names 1 _name)
                        string(TOUPPER ${_name} _UPPACKAGE)
                    endif()
                    if(DEFINED CONAN_${_UPPACKAGE}_ROOT)
                        set(NCBI_COMPONENT_required_FOUND NO)
                        NCBI_define_Pkgcomponent(NAME required PACKAGE ${_name})
                        if(NCBI_COMPONENT_required_FOUND)
                            add_library(${_dep} INTERFACE IMPORTED)
                            set_property(TARGET ${_dep} PROPERTY INTERFACE_LINK_LIBRARIES      ${NCBI_COMPONENT_required_LIBS})
                            set_property(TARGET ${_dep} PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${NCBI_COMPONENT_required_INCLUDE})
                            set_property(TARGET ${_dep} PROPERTY INTERFACE_COMPILE_DEFINITIONS ${NCBI_COMPONENT_required_DEFINES})
                        endif()
                    endif()
                else()
                    get_filename_component(_ext ${_dep} EXT)
                    if (NOT "${_ext}" STREQUAL "")
                        continue()
                    endif()
                    foreach(_component IN LISTS NCBI_ALL_COMPONENTS)
                        if(NOT "${NCBI_COMPONENT_${_component}_LIBS}" STREQUAL "")
                            foreach(_lib IN LISTS NCBI_COMPONENT_${_component}_LIBS)
                                get_filename_component(_directory ${_lib} DIRECTORY)
                                if("${_directory}" STREQUAL "")
                                    continue()
                                endif()
                                get_filename_component(_basename ${_lib} NAME_WE)
                                if ("${_dep}" STREQUAL "${_basename}" OR "lib${_dep}" STREQUAL "${_basename}")
                                    add_library(${_dep} INTERFACE IMPORTED)
                                    set_property(TARGET ${_dep} PROPERTY INTERFACE_LINK_LIBRARIES      ${NCBI_COMPONENT_${_component}_LIBS})
                                    set_property(TARGET ${_dep} PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${NCBI_COMPONENT_${_component}_INCLUDE})
                                    set_property(TARGET ${_dep} PROPERTY INTERFACE_COMPILE_DEFINITIONS ${NCBI_COMPONENT_${_component}_DEFINES})
                                    break()
                                endif()
                            endforeach()
                        endif()
                        if(TARGET ${_dep})
                            break()
                        endif()
                    endforeach()
                endif()
            endforeach()
        endif()
    endforeach()
endfunction()

#############################################################################
#############################################################################

if(NCBI_PTBCFG_USECONAN AND NOT COMMAND conan_define_targets)
    NCBI_define_Pkgcomponent(NAME OpenSSL PACKAGE openssl FIND OpenSSL)
endif()

#############################################################################
# BACKWARD, UNWIND
NCBI_define_Pkgcomponent(NAME BACKWARD PACKAGE backward-cpp REQUIRES libdwarf FIND Backward)
list(REMOVE_ITEM NCBI_ALL_COMPONENTS BACKWARD)
if(NCBI_COMPONENT_BACKWARD_FOUND)
    set(HAVE_LIBBACKWARD_CPP YES)
endif()
NCBI_define_Pkgcomponent(NAME UNWIND PACKAGE libunwind REQUIRES xz_utils;zlib FIND libunwind)
#list(REMOVE_ITEM NCBI_ALL_COMPONENTS UNWIND)

#############################################################################
# Iconv
if(DEFINED CONAN_LIBICONV_ROOT)
    set(ICONV_LIBS ${CONAN_LIBS_LIBICONV})
    set(HAVE_LIBICONV 1)
    set(NCBI_REQUIRE_Iconv_FOUND YES)
elseif(TARGET Iconv::Iconv)
    set(ICONV_LIBS Iconv::Iconv)
    set(HAVE_LIBICONV 1)
    set(NCBI_REQUIRE_Iconv_FOUND YES)
endif()

#############################################################################
# LMDB
NCBI_define_Pkgcomponent(NAME LMDB PACKAGE lmdb FIND lmdb)
if(NOT NCBI_COMPONENT_LMDB_FOUND)
    set(NCBI_COMPONENT_LMDB_FOUND ${NCBI_COMPONENT_LocalLMDB_FOUND})
    set(NCBI_COMPONENT_LMDB_INCLUDE ${NCBI_COMPONENT_LocalLMDB_INCLUDE})
    set(NCBI_COMPONENT_LMDB_NCBILIB ${NCBI_COMPONENT_LocalLMDB_NCBILIB})
    set(HAVE_LIBLMDB ${NCBI_COMPONENT_LMDB_FOUND})
endif()

#############################################################################
# PCRE
NCBI_define_Pkgcomponent(NAME PCRE PACKAGE pcre REQUIRES bzip2;zlib FIND PCRE)
if(NOT NCBI_COMPONENT_PCRE_FOUND)
    set(NCBI_COMPONENT_PCRE_FOUND ${NCBI_COMPONENT_LocalPCRE_FOUND})
    set(NCBI_COMPONENT_PCRE_INCLUDE ${NCBI_COMPONENT_LocalPCRE_INCLUDE})
    set(NCBI_COMPONENT_PCRE_NCBILIB ${NCBI_COMPONENT_LocalPCRE_NCBILIB})
    set(HAVE_LIBPCRE ${NCBI_COMPONENT_PCRE_FOUND})
endif()

#############################################################################
# Z
NCBI_define_Pkgcomponent(NAME Z PACKAGE zlib FIND ZLIB)
if(NOT NCBI_COMPONENT_Z_FOUND)
    set(NCBI_COMPONENT_Z_FOUND ${NCBI_COMPONENT_LocalZ_FOUND})
    set(NCBI_COMPONENT_Z_INCLUDE ${NCBI_COMPONENT_LocalZ_INCLUDE})
    set(NCBI_COMPONENT_Z_NCBILIB ${NCBI_COMPONENT_LocalZ_NCBILIB})
    set(HAVE_LIBZ ${NCBI_COMPONENT_Z_FOUND})
endif()

#############################################################################
# BZ2
NCBI_define_Pkgcomponent(NAME BZ2 PACKAGE bzip2 FIND BZip2)
if(NOT NCBI_COMPONENT_BZ2_FOUND)
    set(NCBI_COMPONENT_BZ2_FOUND ${NCBI_COMPONENT_LocalBZ2_FOUND})
    set(NCBI_COMPONENT_BZ2_INCLUDE ${NCBI_COMPONENT_LocalBZ2_INCLUDE})
    set(NCBI_COMPONENT_BZ2_NCBILIB ${NCBI_COMPONENT_LocalBZ2_NCBILIB})
    set(HAVE_LIBBZ2 ${NCBI_COMPONENT_BZ2_FOUND})
endif()

#############################################################################
# LZO
NCBI_define_Pkgcomponent(NAME LZO PACKAGE lzo FIND lzo)

#############################################################################
# ZSTD
NCBI_define_Pkgcomponent(NAME ZSTD PACKAGE zstd FIND zstd)
if(NCBI_COMPONENT_ZSTD_FOUND AND
    (DEFINED NCBI_COMPONENT_ZSTD_VERSION AND "${NCBI_COMPONENT_ZSTD_VERSION}" VERSION_LESS "1.4"))
    message("ZSTD: Version requirement not met (required at least v1.4)")
    set(NCBI_COMPONENT_ZSTD_FOUND NO)
    set(HAVE_LIBZSTD 0)
endif()

#############################################################################
# Boost
NCBI_define_Pkgcomponent(NAME Boost PACKAGE boost FIND Boost)
set(Boost_FOUND ${NCBI_COMPONENT_Boost_FOUND})

if(NCBI_COMPONENT_Boost_FOUND)
    set(NCBI_COMPONENT_Boost.Test.Included_FOUND YES)
    set(NCBI_COMPONENT_Boost.Test.Included_INCLUDE ${NCBI_COMPONENT_Boost_INCLUDE})
    set(NCBI_COMPONENT_Boost.Test.Included_DEFINES BOOST_TEST_NO_LIB)
    list(APPEND NCBI_ALL_COMPONENTS Boost.Test.Included)
else()
    set(NCBI_COMPONENT_Boost.Test.Included_FOUND NO)  
endif()

if(TARGET Boost::test)
    if(NCBI_TRACE_COMPONENT_Boost OR NCBI_TRACE_ALLCOMPONENTS)
        message("Boost.Test: found")
    endif()
    set(NCBI_COMPONENT_Boost.Test_FOUND YES)
    set(NCBI_COMPONENT_Boost.Test_LIBS    Boost::test)
    list(APPEND NCBI_ALL_COMPONENTS Boost.Test)
else()
    set(NCBI_COMPONENT_Boost.Test_FOUND NO)
endif()

if(TARGET Boost::thread AND TARGET Boost::system)
    if(NCBI_TRACE_COMPONENT_Boost OR NCBI_TRACE_ALLCOMPONENTS)
        message("Boost.Spirit: found")
    endif()
    set(NCBI_COMPONENT_Boost.Spirit_FOUND YES)
    set(NCBI_COMPONENT_Boost.Spirit_LIBS    Boost::thread Boost::system)
    list(APPEND NCBI_ALL_COMPONENTS Boost.Spirit)
elseif(NCBI_COMPONENT_Boost_FOUND)
    if(NCBI_TRACE_COMPONENT_Boost OR NCBI_TRACE_ALLCOMPONENTS)
        message("Boost.Spirit: found")
    endif()
    set(NCBI_COMPONENT_Boost.Spirit_FOUND YES)
    set(NCBI_COMPONENT_Boost.Spirit_INCLUDE ${NCBI_COMPONENT_Boost_INCLUDE})
    set(NCBI_COMPONENT_Boost.Spirit_LIBS    ${NCBI_COMPONENT_Boost_LIBS})
    set(NCBI_COMPONENT_Boost.Spirit_DEFINES ${NCBI_COMPONENT_Boost_DEFINES})
    list(APPEND NCBI_ALL_COMPONENTS Boost.Spirit)
else()
    set(NCBI_COMPONENT_Boost.Spirit_FOUND NO)
endif()

if(TARGET Boost::thread)
    if(NCBI_TRACE_COMPONENT_Boost OR NCBI_TRACE_ALLCOMPONENTS)
        message("Boost.Thread: found")
    endif()
    set(NCBI_COMPONENT_Boost.Thread_FOUND YES)
    set(NCBI_COMPONENT_Boost.Thread_LIBS    Boost::thread)
    list(APPEND NCBI_ALL_COMPONENTS Boost.Thread)
elseif(NCBI_COMPONENT_Boost_FOUND)
    if(NCBI_TRACE_COMPONENT_Boost OR NCBI_TRACE_ALLCOMPONENTS)
        message("Boost.Thread: found")
    endif()
    set(NCBI_COMPONENT_Boost.Thread_FOUND YES)    
    set(NCBI_COMPONENT_Boost.Thread_INCLUDE ${NCBI_COMPONENT_Boost_INCLUDE})
    set(NCBI_COMPONENT_Boost.Thread_LIBS    ${NCBI_COMPONENT_Boost_LIBS})
    set(NCBI_COMPONENT_Boost.Thread_DEFINES ${NCBI_COMPONENT_Boost_DEFINES})
    list(APPEND NCBI_ALL_COMPONENTS Boost.Thread)
else()
    set(NCBI_COMPONENT_Boost.Thread_FOUND NO)
endif()

foreach( _type IN ITEMS Chrono Filesystem Iostreams Regex Serialization System)
    string(TOLOWER ${_type} _lowtype)
    if(TARGET Boost::${_lowtype})
        if(NCBI_TRACE_COMPONENT_Boost OR NCBI_TRACE_ALLCOMPONENTS)
            message("Boost.${_type}: found")
        endif()
        set(NCBI_COMPONENT_Boost.${_type}_FOUND YES)
        set(NCBI_COMPONENT_Boost.${_type}_LIBS    Boost::${_lowtype})
    else()
        set(NCBI_COMPONENT_Boost.${_type}_FOUND NO)
    endif()
endforeach()

if(NCBI_COMPONENT_Boost_FOUND)
    set(_libs)
    foreach( _lib IN LISTS NCBI_COMPONENT_Boost_LIBS)
        if(NOT ${_lib} MATCHES "unit_test_framework")
            list(APPEND _libs ${_lib})
        endif()
    endforeach()
    set(NCBI_COMPONENT_Boost_LIBS ${_libs})
    if(NCBI_TRACE_COMPONENT_Boost OR NCBI_TRACE_ALLCOMPONENTS)
        message("NCBI_COMPONENT_Boost_LIBS: ${NCBI_COMPONENT_Boost_LIBS}")
    endif()
endif()

#############################################################################
# JPEG
NCBI_define_Pkgcomponent(NAME JPEG PACKAGE libjpeg FIND JPEG)

#############################################################################
# PNG
NCBI_define_Pkgcomponent(NAME PNG PACKAGE libpng REQUIRES zlib FIND PNG)

#############################################################################
# GIF
NCBI_define_Pkgcomponent(NAME GIF PACKAGE giflib FIND GIF)

#############################################################################
# TIFF
NCBI_define_Pkgcomponent(NAME TIFF PACKAGE libtiff REQUIRES zlib;libdeflate;xz_utils;libjpeg;jbig;zstd;libwebp FIND TIFF)

#############################################################################
# FASTCGI
NCBI_define_Pkgcomponent(NAME FASTCGI PACKAGE ncbi-fastcgi FIND ncbi-fastcgi)

#############################################################################
# SQLITE3
NCBI_define_Pkgcomponent(NAME SQLITE3 PACKAGE sqlite3 FIND SQLite3)
if(NCBI_COMPONENT_SQLITE3_FOUND)
    check_symbol_exists(sqlite3_unlock_notify ${NCBI_COMPONENT_SQLITE3_INCLUDE}/sqlite3.h HAVE_SQLITE3_UNLOCK_NOTIFY)
    check_include_file(sqlite3async.h HAVE_SQLITE3ASYNC_H -I${NCBI_COMPONENT_SQLITE3_INCLUDE})
endif()

#############################################################################
# BerkeleyDB
NCBI_define_Pkgcomponent(NAME BerkeleyDB PACKAGE libdb FIND libdb)
if(NCBI_COMPONENT_BerkeleyDB_FOUND)
    set(HAVE_BERKELEY_DB 1)
    set(HAVE_BDB         1)
    set(HAVE_BDB_CACHE   1)
endif()

#############################################################################
# XML
NCBI_define_Pkgcomponent(NAME XML PACKAGE libxml2 REQUIRES zlib;libiconv FIND LibXml2)

#############################################################################
# XSLT
NCBI_define_Pkgcomponent(NAME XSLT PACKAGE libxslt REQUIRES libxml2;zlib;libiconv FIND LibXslt)
if(NOT DEFINED NCBI_XSLTPROCTOOL AND DEFINED CONAN_BIN_DIRS_LIBXSLT)
    set(NCBI_XSLTPROCTOOL "${CONAN_BIN_DIRS_LIBXSLT}/xsltproc${CMAKE_EXECUTABLE_SUFFIX}")
    set(NCBI_REQUIRE_XSLTPROCTOOL_FOUND YES)
endif()

#############################################################################
# EXSLT
NCBI_define_Pkgcomponent(NAME EXSLT PACKAGE libxslt REQUIRES libxml2;zlib;libiconv FIND LibXslt)

#############################################################################
# UV
NCBI_define_Pkgcomponent(NAME UV PACKAGE libuv FIND libuv)

#############################################################################
# NGHTTP2
NCBI_define_Pkgcomponent(NAME NGHTTP2 PACKAGE libnghttp2 REQUIRES zlib FIND libnghttp2)

##############################################################################
# GRPC/PROTOBUF
NCBI_define_Pkgcomponent(NAME PROTOBUF PACKAGE protobuf REQUIRES zlib FIND Protobuf)
if(NOT DEFINED NCBI_PROTOC_APP)
    if(DEFINED CONAN_BIN_DIRS_PROTOBUF)
        set(NCBI_PROTOC_APP "${CONAN_BIN_DIRS_PROTOBUF}/protoc${CMAKE_EXECUTABLE_SUFFIX}")
    elseif(TARGET protobuf::protoc)
        get_property(NCBI_PROTOC_APP TARGET protobuf::protoc PROPERTY IMPORTED_LOCATION)
    endif()
endif()
if(NCBI_TRACE_COMPONENT_PROTOBUF OR NCBI_TRACE_ALLCOMPONENTS)
    message("NCBI_PROTOC_APP = ${NCBI_PROTOC_APP}")
endif()

NCBI_define_Pkgcomponent(NAME GRPC PACKAGE grpc REQUIRES abseil;c-ares;openssl;protobuf;re2;zlib FIND gRPC)
if(NOT DEFINED NCBI_GRPC_PLUGIN)
    if(DEFINED CONAN_BIN_DIRS_GRPC)
        set(NCBI_GRPC_PLUGIN "${CONAN_BIN_DIRS_GRPC}/grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX}")
    elseif(TARGET gRPC::grpc_cpp_plugin)
        get_property(NCBI_GRPC_PLUGIN TARGET gRPC::grpc_cpp_plugin PROPERTY IMPORTED_LOCATION)
    endif()
endif()
if(NCBI_TRACE_COMPONENT_GRPC OR NCBI_TRACE_ALLCOMPONENTS)
    message("NCBI_GRPC_PLUGIN = ${NCBI_GRPC_PLUGIN}")
endif()

#############################################################################
# CASSANDRA
NCBI_define_Pkgcomponent(NAME CASSANDRA PACKAGE cassandra-cpp-driver REQUIRES http_parser;libuv;minizip;openssl;rapidjson;zlib FIND cassandra-cpp-driver)

#############################################################################
# MySQL
if(NCBI_PTBCFG_PACKAGED)
    NCBI_define_Pkgcomponent(NAME MySQL PACKAGE libmysqlclient REQUIRES lz4;openssl;zlib;zstd FIND MySQL)
else()
    NCBI_define_Pkgcomponent(NAME MySQL PACKAGE libmysqlclient REQUIRES lz4;openssl;zlib;zstd FIND libmysqlclient)
endif()

#############################################################################
# VDB
NCBI_define_Pkgcomponent(NAME VDB PACKAGE ncbi-vdb FIND ncbi-vdb)
if(NCBI_COMPONENT_VDB_FOUND)
    set(HAVE_NCBI_VDB 1)
endif()
