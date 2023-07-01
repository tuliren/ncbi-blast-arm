#############################################################################
# $Id: CMake.NCBIComponents.cmake 663360 2023-02-16 20:25:53Z ivanov $
#############################################################################

##
## NCBI CMake components description
##
##
## As a result, the following variables should be defined for component XXX
##  NCBI_COMPONENT_XXX_FOUND
##  NCBI_COMPONENT_XXX_INCLUDE
##  NCBI_COMPONENT_XXX_DEFINES
##  NCBI_COMPONENT_XXX_LIBS
##  HAVE_LIBXXX


#############################################################################
set(NCBI_ALL_COMPONENTS "")
set(NCBI_ALL_REQUIRES "")
set(NCBI_ALL_DISABLED "")
set(NCBI_ALL_LEGACY "")
set(NCBI_ALL_DISABLED_LEGACY "")

#############################################################################
macro(NCBIcomponent_report _name)
    if (NCBI_COMPONENT_${_name}_DISABLED)
        message("DISABLED ${_name}")
    endif()
    if (NOT ${_name} IN_LIST NCBI_ALL_COMPONENTS AND
        NOT ${_name} IN_LIST NCBI_ALL_REQUIRES AND
        NOT ${_name} IN_LIST NCBI_ALL_DISABLED)
        if(NOT DEFINED NCBI_COMPONENT_${_name}_FOUND AND NOT DEFINED NCBI_REQUIRE_${_name}_FOUND)
            set(NCBI_REQUIRE_${_name}_FOUND NO)
        endif()
        if(NCBI_COMPONENT_${_name}_FOUND)
            list(APPEND NCBI_ALL_COMPONENTS ${_name})
        elseif(NCBI_REQUIRE_${_name}_FOUND)
            list(APPEND NCBI_ALL_REQUIRES ${_name})
        else()
            list(APPEND NCBI_ALL_DISABLED ${_name})
        endif()
    endif()
endmacro()
macro(NCBIcomponent_deprecated_name _deprecated_name _newname)
    if(NCBI_COMPONENT_${_newname}_FOUND)
        list(APPEND NCBI_ALL_LEGACY ${_deprecated_name})
        set(NCBI_COMPONENT_${_deprecated_name}_FOUND ${_newname})
    else()
        list(APPEND NCBI_ALL_DISABLED_LEGACY ${_deprecated_name})
    endif()
endmacro()
#############################################################################

set(NCBI_REQUIRE_MT_FOUND YES)
list(APPEND NCBI_ALL_REQUIRES MT)
set(NCBI_PlatformBits 64)
set(NCBI_TOOLS_ROOT $ENV{NCBI})

if(BUILD_SHARED_LIBS)
    set(NCBI_REQUIRE_DLL_BUILD_FOUND YES)
    set(NCBI_REQUIRE_DLL_FOUND YES)
endif()
NCBIcomponent_report(DLL_BUILD)
NCBIcomponent_report(DLL)

if(UNIX)
    set(NCBI_REQUIRE_unix_FOUND YES)
    NCBIcomponent_report(unix)
    if(APPLE)
        set(NCBI_REQUIRE_MacOS_FOUND YES)
        NCBIcomponent_report(MacOS)
    elseif(CYGWIN)
        set(NCBI_REQUIRE_Cygwin_FOUND YES)
        NCBIcomponent_report(Cygwin)
    else()
        if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            set(NCBI_REQUIRE_Linux_FOUND YES)
            NCBIcomponent_report(Linux)
        elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
            set(NCBI_REQUIRE_FreeBSD_FOUND YES)
            NCBIcomponent_report(FreeBSD)
        endif()
    endif()
elseif(WIN32)
    set(NCBI_REQUIRE_MSWin_FOUND YES)
    string(REPLACE "\\" "/" NCBI_TOOLS_ROOT "${NCBI_TOOLS_ROOT}")
    NCBIcomponent_report(MSWin)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set(NCBI_REQUIRE_${NCBI_COMPILER}_FOUND YES)
list(APPEND NCBI_ALL_REQUIRES ${NCBI_COMPILER})

if(NOT "${NCBI_PTBCFG_PROJECT_COMPONENTS}" STREQUAL "")
    string(REPLACE "," ";" NCBI_PTBCFG_PROJECT_COMPONENTS "${NCBI_PTBCFG_PROJECT_COMPONENTS}")
    string(REPLACE " " ";" NCBI_PTBCFG_PROJECT_COMPONENTS "${NCBI_PTBCFG_PROJECT_COMPONENTS}")
    foreach(_comp IN LISTS NCBI_PTBCFG_PROJECT_COMPONENTS)
        if("${_comp}" STREQUAL "")
            continue()
        endif()
        string(SUBSTRING ${_comp} 0 1 _sign)
        if ("${_sign}" STREQUAL "-")
            string(SUBSTRING ${_comp} 1 -1 _comp)
            set(NCBI_COMPONENT_${_comp}_DISABLED YES)
        else()
            set(NCBI_COMPONENT_${_comp}_DISABLED NO)
        endif()
    endforeach()
endif()
foreach( _comp IN ITEMS GNUTLS WGMLST)
    if(NOT DEFINED NCBI_COMPONENT_${_comp}_DISABLED)
        set(NCBI_COMPONENT_${_comp}_DISABLED YES)
    endif()
endforeach()

#############################################################################
include(${NCBI_TREE_CMAKECFG}/CMake.NCBIComponentsCheck.cmake)

#############################################################################
# ORIG_LIBS
if(UNIX)
    include(CheckLibraryExists)

    NCBI_find_system_library(DL_LIBS dl)
    if(DL_LIBS)
        set(HAVE_LIBDL 1)
    endif()
    set(THREAD_LIBS   ${CMAKE_THREAD_LIBS_INIT})
    NCBI_find_system_library(CRYPT_LIBS crypt)
    NCBI_find_system_library(MATH_LIBS m)

    if (APPLE)
        NCBI_find_system_library(NETWORK_LIBS resolv)
        NCBI_find_system_library(RT_LIBS c)
    elseif (NCBI_REQUIRE_FreeBSD_FOUND)
        NCBI_find_system_library(NETWORK_LIBS c)
        NCBI_find_system_library(RT_LIBS      rt)
    else ()
        NCBI_find_system_library(NETWORK_LIBS   resolv)
        if(NCBI_PTBCFG_COMPONENT_StaticComponents)
            NCBI_find_system_library(RT_LIBS        rt STATIC)
        else()
            NCBI_find_system_library(RT_LIBS        rt)
        endif()
    endif ()
    set(ORIG_LIBS   ${DL_LIBS} ${RT_LIBS} ${MATH_LIBS} ${CMAKE_THREAD_LIBS_INIT})

    if(NOT HAVE_LIBICONV)
        NCBI_find_system_library(ICONV_LIBS    iconv)
        if(ICONV_LIBS)
            set(HAVE_LIBICONV 1)
        endif()
    endif()
    if(HAVE_LIBICONV)
        set(NCBI_REQUIRE_Iconv_FOUND YES)
        list(APPEND NCBI_ALL_REQUIRES Iconv)
    endif()
elseif(WIN32)
    set(ORIG_LIBS ws2_32.lib dbghelp.lib)
endif()

#############################################################################
# TLS
set(NCBI_REQUIRE_TLS_FOUND YES)
list(APPEND NCBI_ALL_REQUIRES TLS)

#############################################################################
# local_lbsm
if(NOT NCBI_COMPONENT_local_lbsm_DISABLED AND NOT WIN32 AND EXISTS ${NCBITK_SRC_ROOT}/connect/ncbi_lbsm.c)
    set(NCBI_REQUIRE_local_lbsm_FOUND YES)
    set(HAVE_LOCAL_LBSM 1)
endif()
NCBIcomponent_report(local_lbsm)

#############################################################################
# LocalPCRE
if (NOT NCBI_COMPONENT_LocalPCRE_DISABLED AND EXISTS ${NCBITK_INC_ROOT}/util/regexp AND NOT NCBI_PTBCFG_PACKAGING)
    set(NCBI_COMPONENT_LocalPCRE_FOUND YES)
    set(NCBI_COMPONENT_LocalPCRE_INCLUDE ${NCBITK_INC_ROOT}/util/regexp)
    set(NCBI_COMPONENT_LocalPCRE_NCBILIB regexp)
endif()
NCBIcomponent_report(LocalPCRE)

#############################################################################
# LocalZ
if (NOT NCBI_COMPONENT_LocalZ_DISABLED AND EXISTS ${NCBITK_INC_ROOT}/util/compress/zlib AND NOT NCBI_PTBCFG_PACKAGING)
    set(NCBI_COMPONENT_LocalZ_FOUND YES)
    set(NCBI_COMPONENT_LocalZ_INCLUDE ${NCBITK_INC_ROOT}/util/compress/zlib)
    set(NCBI_COMPONENT_LocalZ_NCBILIB z)
endif()
NCBIcomponent_report(LocalZ)

#############################################################################
# LocalBZ2
if (NOT NCBI_COMPONENT_LocalBZ2_DISABLED AND EXISTS ${NCBITK_INC_ROOT}/util/compress/bzip2 AND NOT NCBI_PTBCFG_PACKAGING)
    set(NCBI_COMPONENT_LocalBZ2_FOUND YES)
    set(NCBI_COMPONENT_LocalBZ2_INCLUDE ${NCBITK_INC_ROOT}/util/compress/bzip2)
    set(NCBI_COMPONENT_LocalBZ2_NCBILIB bz2)
endif()
NCBIcomponent_report(LocalBZ2)

#############################################################################
# LocalLMDB
if (NOT NCBI_COMPONENT_LocalLMDB_DISABLED AND EXISTS ${NCBITK_INC_ROOT}/util/lmdb AND NOT CYGWIN AND NOT NCBI_PTBCFG_PACKAGING)
    set(NCBI_COMPONENT_LocalLMDB_FOUND YES)
    set(NCBI_COMPONENT_LocalLMDB_INCLUDE ${NCBITK_INC_ROOT}/util/lmdb)
    set(NCBI_COMPONENT_LocalLMDB_NCBILIB lmdb)
endif()
NCBIcomponent_report(LocalLMDB)

#############################################################################
# connext
if (NOT NCBI_COMPONENT_connext_DISABLED AND EXISTS ${NCBITK_SRC_ROOT}/connect/ext/CMakeLists.txt)
    set(NCBI_REQUIRE_connext_FOUND YES)
    set(HAVE_LIBCONNEXT 1)
endif()
NCBIcomponent_report(connext)

#############################################################################
# PubSeqOS
if (NOT NCBI_COMPONENT_PubSeqOS_DISABLED AND EXISTS ${NCBITK_SRC_ROOT}/objtools/data_loaders/genbank/pubseq/CMakeLists.txt)
    set(NCBI_REQUIRE_PubSeqOS_FOUND YES)
    set(HAVE_PUBSEQ_OS 1)
endif()
NCBIcomponent_report(PubSeqOS)

#############################################################################
# FreeTDS
if(NOT NCBI_COMPONENT_FreeTDS_DISABLED
        AND EXISTS ${NCBITK_INC_ROOT}/dbapi/driver/ftds100
        AND EXISTS ${NCBITK_INC_ROOT}/dbapi/driver/ftds100/freetds)
    set(NCBI_COMPONENT_FreeTDS_FOUND   YES)
    set(HAVE_LIBFTDS 1)
    set(FTDS100_INCLUDE ${NCBITK_INC_ROOT}/dbapi/driver/ftds100 ${NCBITK_INC_ROOT}/dbapi/driver/ftds100/freetds)
    set(NCBI_COMPONENT_FreeTDS_INCLUDE ${FTDS100_INCLUDE})
endif()
NCBIcomponent_report(FreeTDS)

#############################################################################
set(NCBI_COMPONENT_Boost.Test.Included_NCBILIB test_boost)
set(NCBI_COMPONENT_SQLITE3_NCBILIB sqlitewrapp)
set(NCBI_COMPONENT_Sybase_NCBILIB  ncbi_xdbapi_ctlib)
set(NCBI_COMPONENT_ODBC_NCBILIB    ncbi_xdbapi_odbc)
set(NCBI_COMPONENT_FreeTDS_NCBILIB ct_ftds100 ncbi_xdbapi_ftds)
set(NCBI_COMPONENT_connext_NCBILIB xconnext)

#############################################################################
if (CONANCOMPONENTS)
    include(${NCBI_TREE_CMAKECFG}/CMake.NCBIComponentsConan.cmake)
else()
    if(NCBI_PTBCFG_PACKAGING OR NCBI_PTBCFG_PACKAGED OR NCBI_PTBCFG_USECONAN)
        include(${NCBI_TREE_CMAKECFG}/CMake.NCBIComponentsPackage.cmake)
    endif()
    if(NOT NCBI_PTBCFG_PACKAGING OR NCBI_PTBCFG_USELOCAL)
        if (MSVC)
            include(${NCBI_TREE_CMAKECFG}/CMake.NCBIComponentsMSVC.cmake)
        elseif (APPLE)
            include(${NCBI_TREE_CMAKECFG}/CMake.NCBIComponentsXCODE.cmake)
        else()
            include(${NCBI_TREE_CMAKECFG}/CMake.NCBIComponentsUNIXex.cmake)
        endif()
    endif()
endif()

#############################################################################
# NCBI_PYTHON_EXECUTABLE
if(NOT NCBI_PYTHON_EXECUTABLE)
    if(NOT "$ENV{VIRTUAL_ENV}" STREQUAL "")
        find_program(NCBI_PYTHON_EXECUTABLE python)
    endif()
    if(NOT NCBI_PYTHON_EXECUTABLE AND NCBI_COMPONENT_PYTHON_FOUND)
        if(DEFINED PYTHON_EXECUTABLE)
            set(NCBI_PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE})
        elseif(DEFINED Python3_EXECUTABLE)
            set(NCBI_PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
        elseif(EXISTS ${NCBI_ThirdParty_PYTHON}/bin/python${CMAKE_EXECUTABLE_SUFFIX})
            set(NCBI_PYTHON_EXECUTABLE ${NCBI_ThirdParty_PYTHON}/bin/python${CMAKE_EXECUTABLE_SUFFIX})
        elseif(EXISTS ${NCBI_ThirdParty_PYTHON}/python${CMAKE_EXECUTABLE_SUFFIX})
            set(NCBI_PYTHON_EXECUTABLE ${NCBI_ThirdParty_PYTHON}/python${CMAKE_EXECUTABLE_SUFFIX})
        endif()
    endif()
endif()
if(NOT NCBI_PYTHON_EXECUTABLE)
    find_package(PythonInterp 3)
    if (PYTHONINTERP_FOUND)
        set(NCBI_PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE})
    endif()
endif()

#############################################################################
# FreeTDS
set(FTDS100_INCLUDE ${NCBITK_INC_ROOT}/dbapi/driver/ftds100 ${NCBITK_INC_ROOT}/dbapi/driver/ftds100/freetds)

#############################################################################
# NCBILS2
if (NOT NCBI_COMPONENT_NCBILS2_DISABLED AND NCBI_COMPONENT_GCRYPT_FOUND AND EXISTS ${NCBITK_SRC_ROOT}/internal/ncbils2/CMakeLists.txt)
    set(NCBI_REQUIRE_NCBILS2_FOUND YES)
endif()
NCBIcomponent_report(NCBILS2)

#############################################################################
# PSGLoader
if(NOT NCBI_COMPONENT_PSGLoader_DISABLED AND NCBI_COMPONENT_UV_FOUND AND NCBI_COMPONENT_NGHTTP2_FOUND)
    set(HAVE_PSG_LOADER 1)
    set(NCBI_REQUIRE_PSGLoader_FOUND YES)
endif()
NCBIcomponent_report(PSGLoader)

#############################################################################
NCBIcomponent_deprecated_name(C-Toolkit NCBI_C)
NCBIcomponent_deprecated_name(Fast-CGI FASTCGI)
NCBIcomponent_deprecated_name(LIBXML XML)
NCBIcomponent_deprecated_name(LIBXSLT XSLT)
NCBIcomponent_deprecated_name(LIBEXSLT EXSLT)
NCBIcomponent_deprecated_name(LIBXLSXWRITER XLSXWRITER)
NCBIcomponent_deprecated_name(MESA OSMesa)

NCBIcomponent_deprecated_name(Xalan XALAN)
NCBIcomponent_deprecated_name(Xerces XERCES)
NCBIcomponent_deprecated_name(LIBUV UV)
NCBIcomponent_deprecated_name(OPENSSL OpenSSL)
NCBIcomponent_deprecated_name(MONGODB3 MONGOCXX)

#############################################################################
list(SORT NCBI_ALL_LEGACY)
list(APPEND NCBI_ALL_COMPONENTS ${NCBI_ALL_LEGACY})
list(SORT NCBI_ALL_COMPONENTS)
list(SORT NCBI_ALL_REQUIRES)
list(SORT NCBI_ALL_DISABLED)

#############################################################################
# verify
if(NOT "${NCBI_PTBCFG_PROJECT_COMPONENTS}" STREQUAL "")
    foreach(_comp IN LISTS NCBI_PTBCFG_PROJECT_COMPONENTS)
        if("${_comp}" STREQUAL "")
            continue()
        endif()
        NCBI_util_parse_sign( ${_comp} _value _negate)
        if (_negate)
            if(NCBI_COMPONENT_${_value}_FOUND OR NCBI_REQUIRE_${_value}_FOUND)
                message(SEND_ERROR "Component ${_value} is enabled, but not allowed")
            elseif(NOT DEFINED NCBI_COMPONENT_${_value}_FOUND AND NOT DEFINED NCBI_REQUIRE_${_value}_FOUND)
                message(SEND_ERROR "Component ${_value} is unknown")
            endif()
        else()
            if(NOT NCBI_COMPONENT_${_value}_FOUND AND NOT NCBI_REQUIRE_${_value}_FOUND)
                message(SEND_ERROR "Required component ${_value} was not found")
            endif()
        endif()
    endforeach()
endif()
