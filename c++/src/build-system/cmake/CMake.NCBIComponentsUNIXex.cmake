#############################################################################
# $Id: CMake.NCBIComponentsUNIXex.cmake 664466 2023-03-14 18:09:21Z ivanov $
#############################################################################

##
## NCBI CMake components description - UNIX/Linux
##
##
## As a result, the following variables should be defined for component XXX
##  NCBI_COMPONENT_XXX_FOUND
##  NCBI_COMPONENT_XXX_INCLUDE
##  NCBI_COMPONENT_XXX_DEFINES
##  NCBI_COMPONENT_XXX_LIBS
##  HAVE_LIBXXX
##  HAVE_XXX

option(USE_LOCAL_BZLIB "Use a local copy of libbz2")
option(USE_LOCAL_PCRE "Use a local copy of libpcre")
if(USE_LOCAL_BZLIB)
    set(NCBI_COMPONENT_BZ2_DISABLED TRUE)
endif()
if(USE_LOCAL_PCRE)
    set(NCBI_COMPONENT_PCRE_DISABLED TRUE)
endif()

#to debug
#set(NCBI_TRACE_COMPONENT_GRPC ON)

#############################################################################
# common settings
set(NCBI_OPT_ROOT  /opt/ncbi/64)

#############################################################################
# prebuilt libraries

set(NCBI_ThirdParty_BACKWARD      ${NCBI_TOOLS_ROOT}/backward-cpp-1.3.20180206-44ae960 CACHE PATH "BACKWARD root")
set(NCBI_ThirdParty_UNWIND        ${NCBI_TOOLS_ROOT}/libunwind-1.1 CACHE PATH "UNWIND root")
set(NCBI_ThirdParty_LMDB          ${NCBI_TOOLS_ROOT}/lmdb-0.9.24 CACHE PATH "LMDB root")
set(NCBI_ThirdParty_LZO           ${NCBI_TOOLS_ROOT}/lzo-2.05 CACHE PATH "LZO root")
set(NCBI_ThirdParty_ZSTD          ${NCBI_TOOLS_ROOT}/zstd-1.5.2 CACHE PATH "ZSTD root")
set(NCBI_ThirdParty_Boost_VERSION "1.76.0")
set(NCBI_ThirdParty_Boost         ${NCBI_TOOLS_ROOT}/boost-${NCBI_ThirdParty_Boost_VERSION}-ncbi1 CACHE PATH "Boost root")
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(NCBI_ThirdParty_FASTCGI       ${NCBI_TOOLS_ROOT}/fcgi-2.4.2 CACHE PATH "FASTCGI root")
else()
  set(NCBI_ThirdParty_FASTCGI       ${NCBI_TOOLS_ROOT}/fcgi-2.4.0 CACHE PATH "FASTCGI root")
endif()
set(NCBI_ThirdParty_FASTCGI_SHLIB ${NCBI_ThirdParty_FASTCGI})
set(NCBI_ThirdParty_FASTCGIPP     ${NCBI_TOOLS_ROOT}/fastcgi++-3.1~a1+20210601 CACHE PATH "FASTCGIPP root")
set(NCBI_ThirdParty_SQLITE3       ${NCBI_TOOLS_ROOT}/sqlite-3.26.0-ncbi1 CACHE PATH "SQLITE3 root")
set(NCBI_ThirdParty_BerkeleyDB    ${NCBI_TOOLS_ROOT}/BerkeleyDB-4.6.21.1 CACHE PATH "BerkeleyDB root")
set(NCBI_ThirdParty_SybaseNetPath "/opt/sybase/clients/16.0-64bit" CACHE PATH "SybaseNetPath")
set(NCBI_ThirdParty_SybaseLocalPath "" CACHE PATH "SybaseLocalPath")
set(NCBI_ThirdParty_PYTHON_VERSION "3.9")
set(NCBI_ThirdParty_PYTHON        "/opt/python-${NCBI_ThirdParty_PYTHON_VERSION}" CACHE PATH "PYTHON root")
set(NCBI_ThirdParty_VDB           ${NCBI_OPT_ROOT}/trace_software/vdb/vdb-versions/cxx_toolkit/3 CACHE PATH "VDB root")
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(NCBI_ThirdParty_XML           ${NCBI_TOOLS_ROOT}/libxml-2.9.1 CACHE PATH "XML root")
  set(NCBI_ThirdParty_XSLT          ${NCBI_TOOLS_ROOT}/libxml-2.9.1 CACHE PATH "XSLT root")
else()
  set(NCBI_ThirdParty_XML           ${NCBI_TOOLS_ROOT}/libxml-2.7.8 CACHE PATH "XML root")
  set(NCBI_ThirdParty_XSLT          ${NCBI_TOOLS_ROOT}/libxml-2.7.8 CACHE PATH "XSLT root")
endif()
set(NCBI_ThirdParty_EXSLT         ${NCBI_ThirdParty_XSLT})
set(NCBI_ThirdParty_XLSXWRITER    ${NCBI_TOOLS_ROOT}/libxlsxwriter-0.6.9 CACHE PATH "XLSXWRITER root")
set(NCBI_ThirdParty_SAMTOOLS      ${NCBI_TOOLS_ROOT}/samtools CACHE PATH "SAMTOOLS root")
set(NCBI_ThirdParty_FTGL          ${NCBI_TOOLS_ROOT}/ftgl-2.1.3-rc5 CACHE PATH "FTGL root")
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(NCBI_ThirdParty_OpenGL        ${NCBI_TOOLS_ROOT}/Mesa-7.0.2-ncbi2 CACHE PATH "OpenGL root")
  set(NCBI_ThirdParty_OSMesa        ${NCBI_ThirdParty_OpenGL})
  set(NCBI_ThirdParty_GLEW          ${NCBI_TOOLS_ROOT}/glew-2.2.0-ncbi1 CACHE PATH "GLEW root")
else()
  set(NCBI_ThirdParty_GLEW          ${NCBI_TOOLS_ROOT}/glew-1.5.8 CACHE PATH "GLEW root")
endif()
set(NCBI_ThirdParty_XERCES        ${NCBI_TOOLS_ROOT}/xerces-3.1.2 CACHE PATH "XERCES root")
set(NCBI_ThirdParty_GRPC          ${NCBI_TOOLS_ROOT}/grpc-1.36.4-ncbi1 CACHE PATH "GRPC root")
set(NCBI_ThirdParty_Boring        ${NCBI_ThirdParty_GRPC})
set(NCBI_ThirdParty_PROTOBUF      ${NCBI_TOOLS_ROOT}/grpc-1.36.4-ncbi1 CACHE PATH "PROTOBUF root")
set(NCBI_ThirdParty_XALAN         ${NCBI_TOOLS_ROOT}/xalan-1.11 CACHE PATH "XALAN root")
set(NCBI_ThirdParty_GPG           ${NCBI_TOOLS_ROOT}/libgpg-error-1.6 CACHE PATH "GPG root")
set(NCBI_ThirdParty_GCRYPT        ${NCBI_TOOLS_ROOT}/libgcrypt-1.4.3 CACHE PATH "GCRYPT root")
set(NCBI_ThirdParty_MSGSL         ${NCBI_TOOLS_ROOT}/msgsl-0.0.20171114-1c95f94 CACHE PATH "MSGSL root")
set(NCBI_ThirdParty_SGE           "/netmnt/gridengine/current/drmaa" CACHE PATH "SGE root")
set(NCBI_ThirdParty_MONGOCXX      ${NCBI_TOOLS_ROOT}/mongodb-3.6.3 CACHE PATH "MONGOCXX root")
set(NCBI_ThirdParty_MONGOC        ${NCBI_TOOLS_ROOT}/mongo-c-driver-1.17.5 CACHE PATH "MONGOC root")
set(NCBI_ThirdParty_LEVELDB       ${NCBI_TOOLS_ROOT}/leveldb-1.21 CACHE PATH "LEVELDB root")
if (APPLE)
  set(NCBI_ThirdParty_wxWidgets     ${NCBI_TOOLS_ROOT}/wxWidgets-3.1.4-ncbi2 CACHE PATH "wxWidgets root")
else(APPLE)
  set(NCBI_ThirdParty_wxWidgets     ${NCBI_TOOLS_ROOT}/wxWidgets-3.1.3-ncbi1 CACHE PATH "wxWidgets root")
endif(APPLE)
set(NCBI_ThirdParty_GLPK          "/usr/local/glpk/4.45" CACHE PATH "GLPK root")
set(NCBI_ThirdParty_UV            ${NCBI_TOOLS_ROOT}/libuv-1.35.0 CACHE PATH "UV root")
set(NCBI_ThirdParty_NGHTTP2       ${NCBI_TOOLS_ROOT}/nghttp2-1.40.0 CACHE PATH "NGHTTP2 root")
set(NCBI_ThirdParty_GL2PS         ${NCBI_TOOLS_ROOT}/gl2ps-1.4.0 CACHE PATH "GL2PS root")
set(NCBI_ThirdParty_GMOCK         ${NCBI_TOOLS_ROOT}/googletest-1.8.1 CACHE PATH "GMOCK root")
set(NCBI_ThirdParty_GTEST         ${NCBI_TOOLS_ROOT}/googletest-1.8.1 CACHE PATH "GTEST root")
set(NCBI_ThirdParty_CASSANDRA     ${NCBI_TOOLS_ROOT}/datastax-cpp-driver-2.15.1 CACHE PATH "CASSANDRA root")
set(NCBI_ThirdParty_H2O           ${NCBI_TOOLS_ROOT}/h2o-2.2.6 CACHE PATH "H2O root")
set(NCBI_ThirdParty_GMP           ${NCBI_TOOLS_ROOT}/gmp-6.0.0a CACHE PATH "GMP root")
set(NCBI_ThirdParty_NETTLE        ${NCBI_TOOLS_ROOT}/nettle-3.1.1 CACHE PATH "NETTLE root")
set(NCBI_ThirdParty_GNUTLS        ${NCBI_TOOLS_ROOT}/gnutls-3.4.0 CACHE PATH "GNUTLS root")

set(NCBI_ThirdParty_THRIFT        ${NCBI_TOOLS_ROOT}/thrift-0.11.0 CACHE PATH "THRIFT root")
set(NCBI_ThirdParty_NLohmann_JSON ${NCBI_TOOLS_ROOT}/nlohmann-json-3.9.1 CACHE PATH "NLohmann_JSON root")
set(NCBI_ThirdParty_YAML_CPP      ${NCBI_TOOLS_ROOT}/yaml-cpp-0.6.3 CACHE PATH "YAML_CPP root")
set(NCBI_ThirdParty_OPENTRACING   ${NCBI_TOOLS_ROOT}/opentracing-cpp-1.6.0 CACHE PATH "OPENTRACING root")
set(NCBI_ThirdParty_JAEGER        ${NCBI_TOOLS_ROOT}/jaeger-client-cpp-0.7.0 CACHE PATH "JAEGER root")

#############################################################################
#############################################################################


#############################################################################
# in-house-resources
if (NOT NCBI_COMPONENT_in-house-resources_DISABLED)
    if (EXISTS "${NCBI_TOOLS_ROOT}/.ncbirc")
        if (EXISTS "/am/ncbiapdata/test_data")
            set(NCBITEST_TESTDATA_PATH "/am/ncbiapdata/test_data")
            set(NCBI_REQUIRE_in-house-resources_FOUND YES)
        elseif (EXISTS "/Volumes/ncbiapdata/test_data")
            set(NCBITEST_TESTDATA_PATH "/Volumes/ncbiapdata/test_data")
            set(NCBI_REQUIRE_in-house-resources_FOUND YES)
        endif()
    endif()
endif()
NCBIcomponent_report(in-house-resources)

#############################################################################
# NCBI_C
if(NOT NCBI_COMPONENT_NCBI_C_DISABLED)
    set(NCBI_C_ROOT "${NCBI_TOOLS_ROOT}/c++.by-date/production/20230304/C_TOOLKIT")

    get_directory_property(_foo_defs COMPILE_DEFINITIONS)
    if("${_foo_defs}" MATCHES NCBI_INT4_GI)
        set(NCBI_CTOOLKIT_PATH "${NCBI_C_ROOT}")
        set(USE_BIGINT_IDS 0)
    else()
        if (EXISTS "${NCBI_C_ROOT}/ncbi.gi64")
            set(NCBI_CTOOLKIT_PATH "${NCBI_C_ROOT}/ncbi.gi64")
        elseif (EXISTS "${NCBI_C_ROOT}.gi64")
            set(NCBI_CTOOLKIT_PATH "${NCBI_C_ROOT}.gi64")
        else ()
            set(NCBI_CTOOLKIT_PATH "${NCBI_C_ROOT}")
        endif ()
        set(USE_BIGINT_IDS 1)
    endif()

    if(EXISTS "${NCBI_CTOOLKIT_PATH}/include64" AND EXISTS "${NCBI_CTOOLKIT_PATH}/lib64")
        set(NCBI_C_INCLUDE  "${NCBI_CTOOLKIT_PATH}/include64")
        if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
            set(NCBI_C_LIBPATH  "${NCBI_CTOOLKIT_PATH}/altlib64")
        else()
            set(NCBI_C_LIBPATH  "${NCBI_CTOOLKIT_PATH}/lib64")
        endif()

        set(NCBI_C_ncbi     "ncbi")
        if (APPLE)
            set(NCBI_C_ncbi ${NCBI_C_ncbi} -Wl,-framework,ApplicationServices)
        endif ()
        set(HAVE_NCBI_C YES)
    else()
        set(HAVE_NCBI_C NO)
    endif()

    if(HAVE_NCBI_C)
        message("NCBI_C found at ${NCBI_C_INCLUDE}")
        set(NCBI_COMPONENT_NCBI_C_FOUND YES)
        set(NCBI_COMPONENT_NCBI_C_INCLUDE ${NCBI_C_INCLUDE})
        set(_c_libs  ncbiobj ncbimmdb ${NCBI_C_ncbi})
        set(NCBI_COMPONENT_NCBI_C_LIBS -L${NCBI_C_LIBPATH} ${_c_libs})
        set(NCBI_COMPONENT_NCBI_C_DEFINES HAVE_NCBI_C=1
            USE_BIGINT_IDS=${USE_BIGINT_IDS})
        set(NCBI_COMPONENT_NCBI_C_LIBPATH ${NCBI_C_LIBPATH})
    else()
        set(NCBI_COMPONENT_NCBI_C_FOUND NO)
        message("NOT FOUND NCBI_C")
    endif()
endif()
NCBIcomponent_report(NCBI_C)

#############################################################################
# BACKWARD, UNWIND
NCBI_define_Xcomponent(NAME BACKWARD)
if(NCBI_COMPONENT_BACKWARD_FOUND)
    set(HAVE_LIBBACKWARD_CPP YES)
    if(NOT NCBI_PTBCFG_USECONAN AND NOT NCBI_PTBCFG_PACKAGING AND NOT NCBI_PTBCFG_PACKAGED)
        NCBI_find_system_library(LIBDW_LIBS dw)
        if (LIBDW_LIBS)
            set(HAVE_LIBDW 1)
            set(NCBI_COMPONENT_BACKWARD_LIBS ${NCBI_COMPONENT_BACKWARD_LIBS} ${LIBDW_LIBS})
        endif()
    endif()
endif()
if(NOT NCBI_COMPONENT_UNWIND_FOUND)
    if(NOT CYGWIN OR (DEFINED NCBI_COMPONENT_UNWIND_DISABLED AND NOT NCBI_COMPONENT_UNWIND_DISABLED))
        check_include_file(libunwind.h HAVE_LIBUNWIND_H)
        if(HAVE_LIBUNWIND_H)
            NCBI_define_Xcomponent(NAME UNWIND MODULE libunwind LIB unwind)
        endif()
    endif()
endif()

############################################################################
# Kerberos 5 (via GSSAPI)
NCBI_define_Xcomponent(NAME KRB5 LIB gssapi_krb5 krb5 k5crypto com_err)
NCBIcomponent_report(KRB5)
if(NCBI_COMPONENT_KRB5_FOUND)
    set(KRB5_INCLUDE ${NCBI_COMPONENT_KRB5_INCLUDE})
    set(KRB5_LIBS ${NCBI_COMPONENT_KRB5_LIBS})
endif()

##############################################################################
# UUID
NCBI_define_Xcomponent(NAME UUID MODULE uuid LIB uuid)
NCBIcomponent_report(UUID)

##############################################################################
# CURL
NCBI_define_Xcomponent(NAME CURL MODULE libcurl PACKAGE CURL LIB curl)
NCBIcomponent_report(CURL)

#############################################################################
# LMDB
NCBI_define_Xcomponent(NAME LMDB LIB lmdb)
NCBIcomponent_report(LMDB)
if(NOT NCBI_COMPONENT_LMDB_FOUND)
    set(NCBI_COMPONENT_LMDB_FOUND ${NCBI_COMPONENT_LocalLMDB_FOUND})
    set(NCBI_COMPONENT_LMDB_INCLUDE ${NCBI_COMPONENT_LocalLMDB_INCLUDE})
    set(NCBI_COMPONENT_LMDB_NCBILIB ${NCBI_COMPONENT_LocalLMDB_NCBILIB})
endif()
set(HAVE_LIBLMDB ${NCBI_COMPONENT_LMDB_FOUND})

#############################################################################
# PCRE
NCBI_define_Xcomponent(NAME PCRE MODULE libpcre LIB pcre)
NCBIcomponent_report(PCRE)
if(NOT NCBI_COMPONENT_PCRE_FOUND)
    set(NCBI_COMPONENT_PCRE_FOUND ${NCBI_COMPONENT_LocalPCRE_FOUND})
    set(NCBI_COMPONENT_PCRE_INCLUDE ${NCBI_COMPONENT_LocalPCRE_INCLUDE})
    set(NCBI_COMPONENT_PCRE_NCBILIB ${NCBI_COMPONENT_LocalPCRE_NCBILIB})
endif()
set(HAVE_LIBPCRE ${NCBI_COMPONENT_PCRE_FOUND})

#############################################################################
# Z
NCBI_define_Xcomponent(NAME Z MODULE zlib PACKAGE ZLIB LIB z)
NCBIcomponent_report(Z)
if(NOT NCBI_COMPONENT_Z_FOUND)
    set(NCBI_COMPONENT_Z_FOUND ${NCBI_COMPONENT_LocalZ_FOUND})
    set(NCBI_COMPONENT_Z_INCLUDE ${NCBI_COMPONENT_LocalZ_INCLUDE})
    set(NCBI_COMPONENT_Z_NCBILIB ${NCBI_COMPONENT_LocalZ_NCBILIB})
endif()
set(HAVE_LIBZ ${NCBI_COMPONENT_Z_FOUND})

#############################################################################
# BZ2
NCBI_define_Xcomponent(NAME BZ2 PACKAGE BZip2 LIB bz2)
NCBIcomponent_report(BZ2)
if(NOT NCBI_COMPONENT_BZ2_FOUND)
    set(NCBI_COMPONENT_BZ2_FOUND ${NCBI_COMPONENT_LocalBZ2_FOUND})
    set(NCBI_COMPONENT_BZ2_INCLUDE ${NCBI_COMPONENT_LocalBZ2_INCLUDE})
    set(NCBI_COMPONENT_BZ2_NCBILIB ${NCBI_COMPONENT_LocalBZ2_NCBILIB})
endif()
set(HAVE_LIBBZ2 ${NCBI_COMPONENT_BZ2_FOUND})

#############################################################################
# LZO
NCBI_define_Xcomponent(NAME LZO LIB lzo2)
NCBIcomponent_report(LZO)

#############################################################################
# ZSTD
NCBI_define_Xcomponent(NAME ZSTD LIB zstd)
if(NCBI_COMPONENT_ZSTD_FOUND AND
    (DEFINED NCBI_COMPONENT_ZSTD_VERSION AND "${NCBI_COMPONENT_ZSTD_VERSION}" VERSION_LESS "1.4"))
    message("ZSTD: Version requirement not met (required at least v1.4)")
    set(NCBI_COMPONENT_ZSTD_FOUND NO)
    set(HAVE_LIBZSTD 0)
endif()
NCBIcomponent_report(ZSTD)

#############################################################################
# BOOST
if(NOT NCBI_COMPONENT_Boost_DISABLED AND NOT NCBI_COMPONENT_Boost_FOUND)
include(${NCBI_TREE_CMAKECFG}/CMakeChecks.boost.cmake)

#############################################################################
# Boost.Test.Included
if(Boost_FOUND)
    set(NCBI_COMPONENT_Boost.Test.Included_FOUND YES)
    set(NCBI_COMPONENT_Boost.Test.Included_INCLUDE ${Boost_INCLUDE_DIRS})
    set(NCBI_COMPONENT_Boost.Test.Included_DEFINES BOOST_TEST_NO_LIB)
else()
    set(NCBI_COMPONENT_Boost.Test.Included_FOUND NO)
endif()
NCBIcomponent_report(Boost.Test.Included)

#############################################################################
# Boost.Test
if(Boost_FOUND AND DEFINED Boost_UNIT_TEST_FRAMEWORK_LIBRARY)
    set(NCBI_COMPONENT_Boost.Test_FOUND YES)
    set(NCBI_COMPONENT_Boost.Test_INCLUDE ${Boost_INCLUDE_DIRS})
    set(NCBI_COMPONENT_Boost.Test_LIBS    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
else()
  set(NCBI_COMPONENT_Boost.Test_FOUND NO)
endif()
NCBIcomponent_report(Boost.Test)

#############################################################################
# Boost.Spirit
if(Boost_FOUND AND DEFINED Boost_THREAD_LIBRARY AND DEFINED Boost_SYSTEM_LIBRARY)
    set(NCBI_COMPONENT_Boost.Spirit_FOUND YES)
    set(NCBI_COMPONENT_Boost.Spirit_INCLUDE ${Boost_INCLUDE_DIRS})
    set(NCBI_COMPONENT_Boost.Spirit_LIBS    ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})
else()
    set(NCBI_COMPONENT_Boost.Spirit_FOUND NO)
endif()
NCBIcomponent_report(Boost.Spirit)

#############################################################################
# Boost.Thread
if(Boost_FOUND AND DEFINED Boost_THREAD_LIBRARY)
    set(NCBI_COMPONENT_Boost.Thread_FOUND YES)
    set(NCBI_COMPONENT_Boost.Thread_INCLUDE ${Boost_INCLUDE_DIRS})
    set(NCBI_COMPONENT_Boost.Thread_LIBS    ${Boost_THREAD_LIBRARY})
else()
    set(NCBI_COMPONENT_Boost.Thread_FOUND NO)
endif()
NCBIcomponent_report(Boost.Thread)

#############################################################################
#   Boost.Chrono
if(Boost_FOUND AND DEFINED Boost_CHRONO_LIBRARY)
    set(NCBI_COMPONENT_Boost.Chrono_FOUND YES)
    set(NCBI_COMPONENT_Boost.Chrono_INCLUDE ${Boost_INCLUDE_DIRS})
    set(NCBI_COMPONENT_Boost.Chrono_LIBS    ${Boost_CHRONO_LIBRARY})
else()
    set(NCBI_COMPONENT_Boost.Chrono_FOUND NO)
endif()

#   Boost.Filesystem
if(Boost_FOUND AND DEFINED Boost_FILESYSTEM_LIBRARY)
    set(NCBI_COMPONENT_Boost.Filesystem_FOUND YES)
    set(NCBI_COMPONENT_Boost.Filesystem_INCLUDE ${Boost_INCLUDE_DIRS})
    set(NCBI_COMPONENT_Boost.Filesystem_LIBS    ${Boost_FILESYSTEM_LIBRARY})
else()
    set(NCBI_COMPONENT_Boost.Filesystem_FOUND NO)
endif()

#   Boost.Iostreams
if(Boost_FOUND AND DEFINED Boost_IOSTREAMS_LIBRARY)
    set(NCBI_COMPONENT_Boost.Iostreams_FOUND YES)
    set(NCBI_COMPONENT_Boost.Iostreams_INCLUDE ${Boost_INCLUDE_DIRS})
    set(NCBI_COMPONENT_Boost.Iostreams_LIBS    ${Boost_IOSTREAMS_LIBRARY})
else()
    set(NCBI_COMPONENT_Boost.Iostreams_FOUND NO)
endif()

#   Boost.Regex
if(Boost_FOUND AND DEFINED Boost_REGEX_LIBRARY)
    set(NCBI_COMPONENT_Boost.Regex_FOUND YES)
    set(NCBI_COMPONENT_Boost.Regex_INCLUDE ${Boost_INCLUDE_DIRS})
    set(NCBI_COMPONENT_Boost.Regex_LIBS    ${Boost_REGEX_LIBRARY})
else()
    set(NCBI_COMPONENT_Boost.Regex_FOUND NO)
endif()

#   Boost.Serialization
if(Boost_FOUND AND DEFINED Boost_SERIALIZATION_LIBRARY)
    set(NCBI_COMPONENT_Boost.Serialization_FOUND YES)
    set(NCBI_COMPONENT_Boost.Serialization_INCLUDE ${Boost_INCLUDE_DIRS})
    set(NCBI_COMPONENT_Boost.Serialization_LIBS    ${Boost_SERIALIZATION_LIBRARY})
else()
    set(NCBI_COMPONENT_Boost.Serialization_FOUND NO)
endif()

#   Boost.System
if(Boost_FOUND AND DEFINED Boost_SYSTEM_LIBRARY)
    set(NCBI_COMPONENT_Boost.System_FOUND YES)
    set(NCBI_COMPONENT_Boost.System_INCLUDE ${Boost_INCLUDE_DIRS})
    set(NCBI_COMPONENT_Boost.System_LIBS    ${Boost_SYSTEM_LIBRARY})
else()
    set(NCBI_COMPONENT_Boost.System_FOUND NO)
endif()

#############################################################################
# Boost
if(Boost_FOUND)
    set(NCBI_COMPONENT_Boost_FOUND YES)
    set(NCBI_COMPONENT_Boost_INCLUDE ${Boost_INCLUDE_DIRS})
    set(NCBI_COMPONENT_Boost_LIBS
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_IOSTREAMS_LIBRARY}
        ${Boost_CONTEXT_LIBRARY}
        ${Boost_CHRONO_LIBRARY}
        ${Boost_DATE_TIME_LIBRARY}
        ${Boost_REGEX_LIBRARY}
        ${Boost_SERIALIZATION_LIBRARY}
        ${Boost_TIMER_LIBRARY}
    )
else()
    set(NCBI_COMPONENT_Boost_FOUND NO)
endif()

endif(NOT NCBI_COMPONENT_Boost_DISABLED AND NOT NCBI_COMPONENT_Boost_FOUND)
NCBIcomponent_report(Boost)

#############################################################################
# JPEG
NCBI_define_Xcomponent(NAME JPEG MODULE libjpeg PACKAGE JPEG LIB jpeg)
NCBIcomponent_report(JPEG)

#############################################################################
# PNG
NCBI_define_Xcomponent(NAME PNG MODULE libpng PACKAGE PNG LIB png)
NCBIcomponent_report(PNG)

#############################################################################
# GIF
NCBI_define_Xcomponent(NAME GIF PACKAGE GIF LIB gif)
NCBIcomponent_report(GIF)

#############################################################################
# TIFF
NCBI_define_Xcomponent(NAME TIFF MODULE libtiff-4 PACKAGE TIFF LIB tiff)
NCBIcomponent_report(TIFF)

#############################################################################
# FASTCGI
NCBI_define_Xcomponent(NAME FASTCGI LIB fcgi)
NCBIcomponent_report(FASTCGI)

#############################################################################
# FASTCGIPP
NCBI_define_Xcomponent(NAME FASTCGIPP LIB fastcgipp)
NCBIcomponent_report(FASTCGIPP)

#############################################################################
# SQLITE3
NCBI_define_Xcomponent(NAME SQLITE3 MODULE sqlite3 PACKAGE SQLite3 LIB sqlite3)
NCBIcomponent_report(SQLITE3)
if(NCBI_COMPONENT_SQLITE3_FOUND)
    check_symbol_exists(sqlite3_unlock_notify ${NCBI_COMPONENT_SQLITE3_INCLUDE}/sqlite3.h HAVE_SQLITE3_UNLOCK_NOTIFY)
    check_include_file(sqlite3async.h HAVE_SQLITE3ASYNC_H -I${NCBI_COMPONENT_SQLITE3_INCLUDE})
endif()

#############################################################################
# BerkeleyDB
NCBI_define_Xcomponent(NAME BerkeleyDB LIB db)
NCBIcomponent_report(BerkeleyDB)
if(NCBI_COMPONENT_BerkeleyDB_FOUND)
    set(HAVE_BERKELEY_DB 1)
    set(HAVE_BDB         1)
    set(HAVE_BDB_CACHE   1)
endif()

#############################################################################
# ODBC
set(NCBI_COMPONENT_ODBC_FOUND NO)
if(EXISTS  ${NCBITK_INC_ROOT}/dbapi/driver/odbc/unix_odbc)
    set(NCBI_COMPONENT_XODBC_FOUND YES)
    set(NCBI_COMPONENT_XODBC_INCLUDE ${NCBITK_INC_ROOT}/dbapi/driver/odbc/unix_odbc)
endif()

#############################################################################
# MySQL
NCBI_define_Xcomponent(NAME MySQL PACKAGE Mysql LIB mysqlclient LIBPATH_SUFFIX mysql INCLUDE mysql/mysql.h)
NCBIcomponent_report(MySQL)

#############################################################################
# Sybase

if (NOT NCBI_COMPONENT_Sybase_DISABLED)
    file(GLOB _files LIST_DIRECTORIES TRUE "${NCBI_ThirdParty_SybaseNetPath}/OCS*")
    if(_files)
        list(GET _files 0 NCBI_ThirdParty_Sybase)
    endif()
    #NCBI_define_component(Sybase sybdb64 sybblk_r64 sybct_r64 sybcs_r64 sybtcl_r64 sybcomn_r64 sybintl_r64 sybunic64)
    set(__StaticComponents ${NCBI_PTBCFG_COMPONENT_StaticComponents})
    set(NCBI_PTBCFG_COMPONENT_StaticComponents FALSE)
    NCBI_define_Xcomponent(NAME Sybase  LIB sybblk_r64 sybct_r64 sybcs_r64 sybtcl_r64 sybcomn_r64 sybintl_r64 sybunic64)
    set(NCBI_PTBCFG_COMPONENT_StaticComponents ${__StaticComponents})
endif()
NCBIcomponent_report(Sybase)
if (NCBI_COMPONENT_Sybase_FOUND)
    set(NCBI_COMPONENT_Sybase_DEFINES ${NCBI_COMPONENT_Sybase_DEFINES} SYB_LP64)
    set(SYBASE_PATH ${NCBI_ThirdParty_SybaseNetPath})
    set(SYBASE_LCL_PATH "${NCBI_ThirdParty_SybaseLocalPath}")
endif()

#############################################################################
# PYTHON
NCBI_define_Xcomponent(NAME PYTHON LIB python${NCBI_ThirdParty_PYTHON_VERSION} python3 INCLUDE python${NCBI_ThirdParty_PYTHON_VERSION})
NCBIcomponent_report(PYTHON)

#############################################################################
# VDB
if(NOT NCBI_COMPONENT_VDB_DISABLED AND NOT NCBI_COMPONENT_VDB_FOUND)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        NCBI_define_Xcomponent(NAME VDB LIB ncbi-vdb
            LIBPATH_SUFFIX linux/debug/x86_64/lib INCPATH_SUFFIX interfaces)
    else()
        NCBI_define_Xcomponent(NAME VDB LIB ncbi-vdb
            LIBPATH_SUFFIX linux/release/x86_64/lib INCPATH_SUFFIX interfaces)
    endif()
    if(NCBI_COMPONENT_VDB_FOUND)
        set(NCBI_COMPONENT_VDB_INCLUDE
            ${NCBI_COMPONENT_VDB_INCLUDE} 
            ${NCBI_COMPONENT_VDB_INCLUDE}/os/linux 
            ${NCBI_COMPONENT_VDB_INCLUDE}/os/unix
            ${NCBI_COMPONENT_VDB_INCLUDE}/cc/gcc/x86_64
            ${NCBI_COMPONENT_VDB_INCLUDE}/cc/gcc
        )
        set(HAVE_NCBI_VDB 1)
    endif()
endif()
NCBIcomponent_report(VDB)

##############################################################################
# wxWidgets
NCBI_define_Xcomponent(NAME GTK2 PACKAGE GTK2)
NCBI_define_Xcomponent(NAME FONTCONFIG MODULE fontconfig PACKAGE Fontconfig LIB fontconfig)
set(_wx_ver 3.1)
NCBI_define_Xcomponent(NAME wxWidgets LIB
    wx_gtk2_gl-${_wx_ver}
    wx_gtk2_richtext-${_wx_ver}
    wx_gtk2_aui-${_wx_ver}
    wx_gtk2_propgrid-${_wx_ver}
    wx_gtk2_xrc-${_wx_ver}
    wx_gtk2_html-${_wx_ver}
    wx_gtk2_qa-${_wx_ver}
    wx_gtk2_adv-${_wx_ver}
    wx_gtk2_core-${_wx_ver}
    wx_base_xml-${_wx_ver}
    wx_base_net-${_wx_ver}
    wx_base-${_wx_ver}
    wxscintilla-${_wx_ver}
    INCLUDE wx-${_wx_ver} ADD_COMPONENT FONTCONFIG GTK2
)
NCBIcomponent_report(wxWidgets)
if(NCBI_COMPONENT_wxWidgets_FOUND)
    list(GET NCBI_COMPONENT_wxWidgets_LIBS 0 _lib)
    get_filename_component(_libdir ${_lib} DIRECTORY)
    set(NCBI_COMPONENT_wxWidgets_INCLUDE ${_libdir}/wx/include/gtk2-ansi-${_wx_ver} ${NCBI_COMPONENT_wxWidgets_INCLUDE})
    set(NCBI_COMPONENT_wxWidgets_LIBS    ${NCBI_COMPONENT_wxWidgets_LIBS} -lXxf86vm -lSM -lexpat)
    if(BUILD_SHARED_LIBS)
        set(NCBI_COMPONENT_wxWidgets_DEFINES __WXGTK__  WXUSINGDLL wxDEBUG_LEVEL=0)
    else()
        set(NCBI_COMPONENT_wxWidgets_DEFINES __WXGTK__ wxDEBUG_LEVEL=0)
    endif()
endif()

##############################################################################
# GCRYPT
NCBI_define_Xcomponent(NAME GPG    LIB gpg-error)
NCBI_define_Xcomponent(NAME GCRYPT LIB gcrypt ADD_COMPONENT GPG)
NCBIcomponent_report(GCRYPT)

#############################################################################
# XML
NCBI_define_Xcomponent(NAME XML MODULE libxml-2.0 PACKAGE LibXml2 LIB xml2 INCLUDE libxml2)
NCBIcomponent_report(XML)

#############################################################################
# XSLT
NCBI_define_Xcomponent(NAME XSLT MODULE libxslt PACKAGE LibXslt LIB xslt ADD_COMPONENT XML)
NCBIcomponent_report(XSLT)
if(NCBI_COMPONENT_XSLT_FOUND)
    if(NOT DEFINED NCBI_XSLTPROCTOOL)
        if(LIBXSLT_XSLTPROC_EXECUTABLE)
            set(NCBI_XSLTPROCTOOL ${LIBXSLT_XSLTPROC_EXECUTABLE})
        elseif(EXISTS ${NCBI_ThirdParty_XSLT}/bin/xsltproc)
            set(NCBI_XSLTPROCTOOL ${NCBI_ThirdParty_XSLT}/bin/xsltproc)
        else()
            find_program(NCBI_XSLTPROCTOOL xsltproc)
        endif()
        if(NCBI_XSLTPROCTOOL)
            set(NCBI_REQUIRE_XSLTPROCTOOL_FOUND YES)
        endif()
    endif()
endif()

#############################################################################
# EXSLT
NCBI_define_Xcomponent(NAME EXSLT MODULE libexslt PACKAGE LibXslt LIB exslt ADD_COMPONENT XML GCRYPT)
NCBIcomponent_report(EXSLT)
if(NCBI_COMPONENT_EXSLT_FOUND)
    set(NCBI_COMPONENT_EXSLT_LIBS ${LIBXSLT_EXSLT_LIBRARIES} ${NCBI_COMPONENT_EXSLT_LIBS})
endif()

#############################################################################
# XLSXWRITER
NCBI_define_Xcomponent(NAME XLSXWRITER LIB xlsxwriter)
NCBIcomponent_report(XLSXWRITER)

#############################################################################
# LAPACK
NCBI_define_Xcomponent(NAME LAPACK PACKAGE LAPACK LIB lapack blas)
NCBIcomponent_report(LAPACK)
if(NCBI_COMPONENT_LAPACK_FOUND)
    check_include_file(lapacke.h HAVE_LAPACKE_H)
    check_include_file(lapacke/lapacke.h HAVE_LAPACKE_LAPACKE_H)
    check_include_file(Accelerate/Accelerate.h HAVE_ACCELERATE_ACCELERATE_H)
endif()

#############################################################################
# SAMTOOLS
NCBI_define_Xcomponent(NAME SAMTOOLS LIB bam)
NCBIcomponent_report(SAMTOOLS)

#############################################################################
# FreeType
NCBI_define_Xcomponent(NAME FreeType MODULE freetype2 PACKAGE Freetype LIB freetype INCLUDE freetype2)
NCBIcomponent_report(FreeType)

#############################################################################
# FTGL
NCBI_define_Xcomponent(NAME FTGL MODULE ftgl LIB ftgl INCLUDE FTGL)
NCBIcomponent_report(FTGL)

#############################################################################
# GLEW
#NCBI_define_Xcomponent(NAME GLEW MODULE glew LIB GLEW)
NCBI_define_Xcomponent(NAME GLEW LIB GLEW)
NCBIcomponent_report(GLEW)
if(NCBI_COMPONENT_GLEW_FOUND)
    foreach( _inc IN LISTS NCBI_COMPONENT_GLEW_INCLUDE)
        get_filename_component(_incdir ${_inc} DIRECTORY)
        get_filename_component(_incGL ${_inc} NAME)
        if("${_incGL}" STREQUAL "GL")
            set(NCBI_COMPONENT_GLEW_INCLUDE ${_incdir} ${NCBI_COMPONENT_GLEW_INCLUDE})
            break()
        endif()
    endforeach()
    set(saved_REQUIRED_DEFINITIONS  ${CMAKE_REQUIRED_DEFINITIONS})
    set(saved_REQUIRED_INCLUDES     ${CMAKE_REQUIRED_INCLUDES})
    set(saved_REQUIRED_LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS})
    set(CMAKE_REQUIRED_DEFINITIONS  "-DGLEW_MX")
    set(CMAKE_REQUIRED_INCLUDES     ${NCBI_COMPONENT_GLEW_INCLUDE})
    set(CMAKE_REQUIRED_LINK_OPTIONS ${NCBI_COMPONENT_GLEW_LDFLAGS})
    check_symbol_exists(glewContextInit "GL/glew.h" HAVE_GLEW_MX)
    set(CMAKE_REQUIRED_DEFINITIONS  ${saved_REQUIRED_DEFINITIONS})
    set(CMAKE_REQUIRED_INCLUDES     ${saved_REQUIRED_INCLUDES})
    set(CMAKE_REQUIRED_LINK_OPTIONS ${saved_REQUIRED_LINK_OPTIONS})
    if(HAVE_GLEW_MX)
        set(NCBI_COMPONENT_GLEW_DEFINES ${NCBI_COMPONENT_GLEW_DEFINES} GLEW_MX)
    endif()
    if(NOT BUILD_SHARED_LIBS)
        set(NCBI_COMPONENT_GLEW_DEFINES ${NCBI_COMPONENT_GLEW_DEFINES} GLEW_STATIC)
    endif()
endif()

##############################################################################
# OpenGL
set(OpenGL_GL_PREFERENCE LEGACY)
NCBI_define_Xcomponent(NAME OpenGL PACKAGE OpenGL LIB GLU GL)
NCBIcomponent_report(OpenGL)
if(NCBI_COMPONENT_OpenGL_FOUND)
    set(NCBI_COMPONENT_OpenGL_LIBS ${NCBI_COMPONENT_OpenGL_LIBS}  -lXmu -lXt -lXext -lX11)
endif()

##############################################################################
# OSMesa
NCBI_define_Xcomponent(NAME OSMesa LIB OSMesa ADD_COMPONENT OpenGL)
NCBIcomponent_report(OSMesa)

#############################################################################
# XERCES
NCBI_define_Xcomponent(NAME XERCES MODULE xerces-c PACKAGE XercesC LIB xerces-c)
NCBIcomponent_report(XERCES)

##############################################################################
# GRPC/PROTOBUF
if(NOT DEFINED NCBI_PROTOC_APP)
    set(NCBI_PROTOC_APP "${NCBI_ThirdParty_GRPC}/${NCBI_BUILD_TYPE}/bin/protoc")
endif()
if(NOT DEFINED NCBI_GRPC_PLUGIN)
    set(NCBI_GRPC_PLUGIN "${NCBI_ThirdParty_GRPC}/${NCBI_BUILD_TYPE}/bin/grpc_cpp_plugin")
endif()
if(NOT EXISTS "${NCBI_PROTOC_APP}")
    set(NCBI_PROTOC_APP)
    find_program(NCBI_PROTOC_APP protoc)
endif()
if(NOT EXISTS "${NCBI_GRPC_PLUGIN}")
    set(NCBI_GRPC_PLUGIN)
    find_program(NCBI_GRPC_PLUGIN grpc_cpp_plugin)
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    NCBI_define_Xcomponent(NAME PROTOBUF LIB protobufd)
endif()
if(NOT NCBI_COMPONENT_PROTOBUF_FOUND)
    NCBI_define_Xcomponent(NAME PROTOBUF MODULE protobuf PACKAGE Protobuf LIB protobuf)
endif()
NCBIcomponent_report(PROTOBUF)
NCBI_define_Xcomponent(NAME Boring LIB boringssl boringcrypto)
NCBI_define_Xcomponent(NAME GRPC MODULE grpc++ LIB
    grpc++ grpc address_sorting upb cares gpr absl_bad_optional_access absl_str_format_internal
    absl_strings absl_strings_internal absl_base absl_spinlock_wait absl_dynamic_annotations
    absl_int128 absl_throw_delegate absl_raw_logging_internal absl_log_severity
    )
NCBIcomponent_report(GRPC)
if(NCBI_COMPONENT_GRPC_FOUND)
    set(NCBI_COMPONENT_GRPC_LIBS  ${NCBI_COMPONENT_GRPC_LIBS} ${NCBI_COMPONENT_Boring_LIBS})
    if(NOT NCBI_PTBCFG_USECONAN AND NOT NCBI_PTBCFG_PACKAGING AND NOT NCBI_PTBCFG_PACKAGED AND
        "${NCBI_COMPONENT_GRPC_VERSION}" STRLESS "1.17.0")
        NCBI_define_Xcomponent(NAME CGRPC MODULE grpc LIB grpc)
        set(NCBI_COMPONENT_GRPC_LIBS ${NCBI_COMPONENT_GRPC_LIBS} ${NCBI_COMPONENT_CGRPC_LIBS})
    endif()
endif()

#############################################################################
# XALAN
NCBI_define_Xcomponent(NAME XALAN PACKAGE XalanC LIB xalan-c xalanMsg)
NCBIcomponent_report(XALAN)

##############################################################################
# PERL
if(NOT NCBI_COMPONENT_PERL_DISABLED)
    find_package(PerlLibs)
    if (PERLLIBS_FOUND)
        set(NCBI_COMPONENT_PERL_FOUND   YES)
        set(NCBI_COMPONENT_PERL_INCLUDE ${PERL_INCLUDE_PATH})
        set(NCBI_COMPONENT_PERL_LIBS    ${PERL_LIBRARY})

        if(NCBI_TRACE_COMPONENT_PERL OR NCBI_TRACE_ALLCOMPONENTS)
            message("PERL: include dir = ${NCBI_COMPONENT_PERL_INCLUDE}")
            message("PERL: libs = ${NCBI_COMPONENT_PERL_LIBS}")
        endif()
    endif()
endif()
NCBIcomponent_report(PERL)

#############################################################################
# OpenSSL
NCBI_define_Xcomponent(NAME OpenSSL MODULE openssl PACKAGE OpenSSL LIB ssl crypto)
NCBIcomponent_report(OpenSSL)

#############################################################################
# MSGSL  (Microsoft Guidelines Support Library)
NCBI_define_Xcomponent(NAME MSGSL)
NCBIcomponent_report(MSGSL)

#############################################################################
# SGE  (Sun Grid Engine)
NCBI_define_Xcomponent(NAME SGE LIB drmaa LIBPATH_SUFFIX lib/lx-amd64)
NCBIcomponent_report(SGE)

#############################################################################
# MONGOCXX
NCBI_define_Xcomponent(NAME MONGOC LIB mongoc bson)
NCBI_define_Xcomponent(NAME MONGOCXX MODULE libmongocxx LIB mongocxx bsoncxx INCLUDE mongocxx/v_noabi bsoncxx/v_noabi)
NCBIcomponent_report(MONGOCXX)
NCBIcomponent_add(MONGOCXX MONGOC)

#############################################################################
# LEVELDB
# only has cmake cfg
NCBI_define_Xcomponent(NAME LEVELDB LIB leveldb)
NCBIcomponent_report(LEVELDB)

#############################################################################
# WGMLST
if(NOT NCBI_COMPONENT_WGMLST_DISABLED)
    find_package(SKESA)
    if (WGMLST_FOUND)
        set(NCBI_COMPONENT_WGMLST_FOUND YES)
        set(NCBI_COMPONENT_WGMLST_INCLUDE ${WGMLST_INCLUDE_DIRS})
        set(NCBI_COMPONENT_WGMLST_LIBS    ${WGMLST_LIBPATH} ${WGMLST_LIBRARIES})
    endif()
endif()
NCBIcomponent_report(WGMLST)

#############################################################################
# GLPK
NCBI_define_Xcomponent(NAME GLPK LIB glpk)
NCBIcomponent_report(GLPK)

#############################################################################
# UV
NCBI_define_Xcomponent(NAME UV MODULE libuv LIB uv)
NCBIcomponent_report(UV)
if(NCBI_COMPONENT_UV_FOUND)
    set(NCBI_COMPONENT_UV_LIBS    ${NCBI_COMPONENT_UV_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

#############################################################################
# NGHTTP2
NCBI_define_Xcomponent(NAME NGHTTP2 MODULE libnghttp2 LIB nghttp2)
NCBIcomponent_report(NGHTTP2)

#############################################################################
# GL2PS
NCBI_define_Xcomponent(NAME GL2PS LIB gl2ps)
NCBIcomponent_report(GL2PS)

#############################################################################
# GMOCK
NCBI_define_Xcomponent(NAME GTEST MODULE gtest LIB gtest)
NCBI_define_Xcomponent(NAME GMOCK MODULE gmock LIB gmock ADD_COMPONENT GTEST)
NCBIcomponent_report(GMOCK)

#############################################################################
# CASSANDRA
NCBI_define_Xcomponent(NAME CASSANDRA LIB cassandra)
NCBIcomponent_report(CASSANDRA)

#############################################################################
# H2O
NCBI_define_Xcomponent(NAME H2O MODULE libh2o LIB h2o)
NCBIcomponent_report(H2O)

#############################################################################
# GMP
NCBI_define_Xcomponent(NAME GMP LIB gmp)
NCBIcomponent_report(GMP)

#############################################################################
# NETTLE
NCBI_define_Xcomponent(NAME NETTLE LIB hogweed nettle ADD_COMPONENT GMP)
NCBIcomponent_report(NETTLE)

#############################################################################
# GNUTLS
if(NOT NCBI_COMPONENT_GNUTLS_DISABLED)
    NCBI_find_Xlibrary(NCBI_COMPONENT_IDN_LIBS idn)
    if(NCBI_COMPONENT_IDN_LIBS)
        set(NCBI_COMPONENT_IDN_FOUND YES)
    endif()
    NCBI_define_Xcomponent(NAME GNUTLS LIB gnutls ADD_COMPONENT NETTLE IDN Z)
endif()
NCBIcomponent_report(GNUTLS)

#############################################################################
# THRIFT
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    NCBI_define_Xcomponent(NAME THRIFT LIB thriftd)
else()
    NCBI_define_Xcomponent(NAME THRIFT LIB thrift)
endif()
NCBIcomponent_report(THRIFT)

#############################################################################
# NLohmann_JSON
NCBI_define_Xcomponent(NAME NLohmann_JSON)
NCBIcomponent_report(NLohmann_JSON)

#############################################################################
# YAML_CPP
NCBI_define_Xcomponent(NAME YAML_CPP LIB yaml-cpp)
NCBIcomponent_report(YAML_CPP)

#############################################################################
# OPENTRACING
NCBI_define_Xcomponent(NAME OPENTRACING LIB opentracing)
NCBIcomponent_report(OPENTRACING)

#############################################################################
# JAEGER
NCBI_define_Xcomponent(NAME JAEGER LIB jaegertracing ADD_COMPONENT NLohmann_JSON OPENTRACING YAML_CPP THRIFT)
NCBIcomponent_report(JAEGER)
