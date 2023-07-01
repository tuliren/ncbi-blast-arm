#############################################################################
# $Id: CMakeChecks.cmake 661781 2023-01-19 13:28:57Z ivanov $
#############################################################################
#############################################################################
##
##  NCBI CMake wrapper
##    Author: Andrei Gourianov, gouriano@ncbi
##
##  Checks and configuration


############################################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_RUNTIME}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_SHARED}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_ARCHIVE}")

if(EXISTS ${NCBI_TREE_ROOT}/CMake.CustomConfig.txt)
	include(${NCBI_TREE_ROOT}/CMake.CustomConfig.txt)
endif()

############################################################################
# OS-specific settings
include(${NCBI_TREE_CMAKECFG}/CMakeChecks.os.cmake)

#############################################################################
# Build configurations and compiler definitions
include(${NCBI_TREE_CMAKECFG}/CMakeChecks.compiler.cmake)

#############################################################################
if(NOT DEFINED NCBI_COMPILER_PREBUILT)
    if ("${NCBI_COMPILER}" STREQUAL "MSVC")
        set(NCBI_COMPILER_PREBUILT "VS")
        if ("${NCBI_COMPILER_VERSION}" LESS "1900")
            set(NCBI_COMPILER_PREBUILT ${NCBI_COMPILER_PREBUILT}2015)
        elseif ("${NCBI_COMPILER_VERSION}" LESS "1924")
            set(NCBI_COMPILER_PREBUILT ${NCBI_COMPILER_PREBUILT}2017)
        else()
            set(NCBI_COMPILER_PREBUILT ${NCBI_COMPILER_PREBUILT}2019)
        endif()
    elseif(XCODE)
        set(NCBI_COMPILER_PREBUILT "Xcode${NCBI_COMPILER_VERSION}")
    else()
        set(NCBI_COMPILER_PREBUILT "${NCBI_COMPILER}${NCBI_COMPILER_VERSION}")
    endif()
endif()
if(NOT DEFINED NCBI_DIRNAME_PREBUILT)
    set(NCBI_DIRNAME_PREBUILT "CMake-${NCBI_COMPILER_PREBUILT}")
    if ("${NCBI_COMPILER}" STREQUAL "MSVC" OR XCODE)
        set(NCBI_DIRNAME_PREBUILT ${NCBI_DIRNAME_PREBUILT}${NCBI_PTBCFG_INSTALL_SUFFIX})
        if (BUILD_SHARED_LIBS)
            set(NCBI_DIRNAME_PREBUILT ${NCBI_DIRNAME_PREBUILT}-DLL)
        endif()
    else()
        set(NCBI_DIRNAME_PREBUILT ${NCBI_DIRNAME_PREBUILT}-${STD_BUILD_TYPE})
        set(NCBI_DIRNAME_PREBUILT ${NCBI_DIRNAME_PREBUILT}${NCBI_PTBCFG_INSTALL_SUFFIX})
        if (BUILD_SHARED_LIBS)
            set(NCBI_DIRNAME_PREBUILT ${NCBI_DIRNAME_PREBUILT}DLL)
        endif()
    endif()
endif()

set(_tk_includedir      ${NCBITK_INC_ROOT})
set(_tk_incinternal     ${NCBITK_INC_ROOT}/${NCBI_DIRNAME_INTERNAL})
set(_inc_dirs)
foreach( _inc IN ITEMS ${includedir} ${incinternal} ${_tk_includedir} ${_tk_incinternal})
    if (IS_DIRECTORY ${_inc})
        list(APPEND _inc_dirs ${_inc})
    endif()
endforeach()
list(REMOVE_DUPLICATES _inc_dirs)
include_directories(${incdir} ${_inc_dirs})
include_regular_expression("^.*[.](h|hpp|c|cpp|inl|inc)$")
if(OFF)
    message("include_directories(${incdir} ${_inc_dirs})")
endif()

if (DEFINED NCBI_EXTERNAL_TREE_ROOT)
    set(NCBI_EXTERNAL_BUILD_ROOT  ${NCBI_EXTERNAL_TREE_ROOT}/${NCBI_DIRNAME_PREBUILT})
    if (NOT EXISTS ${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}/${NCBI_PTBCFG_INSTALL_EXPORT}.cmake)
        message(FATAL_ERROR "${NCBI_PTBCFG_INSTALL_EXPORT} was not found in ${NCBI_EXTERNAL_BUILD_ROOT}/${NCBI_DIRNAME_EXPORT}")
    endif()
endif()

#set(CMAKE_MODULE_PATH "${NCBI_SRC_ROOT}/build-system/cmake/" ${CMAKE_MODULE_PATH})
if(EXISTS "${NCBI_TREE_CMAKECFG}/modules")
    list(APPEND CMAKE_MODULE_PATH "${NCBI_TREE_CMAKECFG}/modules")
endif()

if(NOT NCBI_PTBCFG_COLLECT_REQUIRES)
#############################################################################
# Basic checks
include(${NCBI_TREE_CMAKECFG}/CMakeChecks.basic-checks.cmake)

#############################################################################
# External libraries
include(${NCBI_TREE_CMAKECFG}/CMake.NCBIComponents.cmake)

#############################################################################
# Generation of configuration files

# Stable components
# This sets a version to be used throughout our config process
# NOTE: Adjust as needed
#
set(NCBI_CPP_TOOLKIT_VERSION_MAJOR 27)
set(NCBI_CPP_TOOLKIT_VERSION_MINOR 0)
set(NCBI_CPP_TOOLKIT_VERSION_PATCH 0)
set(NCBI_CPP_TOOLKIT_VERSION_EXTRA "")
set(NCBI_CPP_TOOLKIT_VERSION
    ${NCBI_CPP_TOOLKIT_VERSION_MAJOR}.${NCBI_CPP_TOOLKIT_VERSION_MINOR}.${NCBI_CPP_TOOLKIT_VERSION_PATCH}${NCBI_CPP_TOOLKIT_VERSION_EXTRA})

#############################################################################
# Version control systems
# These are needed for some use cases
include(FindGit)
if (GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} -C ${top_src_dir} log -1 --format=%h
        OUTPUT_VARIABLE TOOLKIT_GIT_REVISION ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Git revision = ${TOOLKIT_GIT_REVISION}")
endif()

include(FindSubversion)
set(TOOLKIT_WC_REVISION 0)
if (Subversion_FOUND AND EXISTS ${top_src_dir}/.svn)
    NCBI_Subversion_WC_INFO(${top_src_dir} TOOLKIT)
elseif(NOT "$ENV{SVNREV}" STREQUAL "")
    set(TOOLKIT_WC_REVISION "$ENV{SVNREV}")
    set(TOOLKIT_WC_URL "$ENV{SVNURL}")
else()
    set(TOOLKIT_WC_URL "")
endif()
if(NOT "$ENV{NCBI_SUBVERSION_REVISION}" STREQUAL "")
    set(TOOLKIT_WC_REVISION "$ENV{NCBI_SUBVERSION_REVISION}")
endif()
set(NCBI_SUBVERSION_REVISION ${TOOLKIT_WC_REVISION})
message(STATUS "SVN revision = ${TOOLKIT_WC_REVISION}")
message(STATUS "SVN URL = ${TOOLKIT_WC_URL}")

if(NOT "${TOOLKIT_GIT_REVISION}" STREQUAL "")
    set(NCBI_REVISION ${TOOLKIT_GIT_REVISION})
    set(HAVE_NCBI_REVISION 1)
elseif(NOT "${TOOLKIT_WC_REVISION}" STREQUAL "")
    set(NCBI_REVISION ${TOOLKIT_WC_REVISION})
    set(HAVE_NCBI_REVISION 1)
endif()

if (Subversion_FOUND AND EXISTS ${top_src_dir}/src/build-system/.svn)
    NCBI_Subversion_WC_INFO(${top_src_dir}/src/build-system CORELIB)
else()
    set(CORELIB_WC_REVISION 0)
    set(CORELIB_WC_URL "")
endif()

if(NOT "$ENV{NCBI_SC_VERSION}" STREQUAL "")
    set(NCBI_SC_VERSION $ENV{NCBI_SC_VERSION})
else()
    set(NCBI_SC_VERSION 0)
    if (NOT "${CORELIB_WC_URL}" STREQUAL "")
        string(REGEX REPLACE ".*/production/components/infrastructure/([0-9]+)\\.[0-9]+/.*" "\\1" _SC_VER "${CORELIB_WC_URL}")
        string(LENGTH "${_SC_VER}" _SC_VER_LEN)
        if (${_SC_VER_LEN} LESS 10 AND NOT "${_SC_VER}" STREQUAL "")
            set(NCBI_SC_VERSION ${_SC_VER})
            message(STATUS "Stable Components Number = ${NCBI_SC_VERSION}")
        endif()
    endif()
endif()

set(NCBI_TEAMCITY_BUILD_NUMBER 0)
if (NOT "$ENV{TEAMCITY_VERSION}" STREQUAL "")
    set(NCBI_TEAMCITY_BUILD_NUMBER   $ENV{BUILD_NUMBER})
    set(NCBI_TEAMCITY_PROJECT_NAME   $ENV{TEAMCITY_PROJECT_NAME})
    set(NCBI_TEAMCITY_BUILDCONF_NAME $ENV{TEAMCITY_BUILDCONF_NAME})
    if(EXISTS "$ENV{TEAMCITY_BUILD_PROPERTIES_FILE}")
        file(STRINGS "$ENV{TEAMCITY_BUILD_PROPERTIES_FILE}" _list)
        foreach( _item IN LISTS _list)
            if ("${_item}" MATCHES "teamcity.build.id")
                string(REPLACE "teamcity.build.id" "" _item ${_item})
                string(REPLACE " " "" _item ${_item})
                string(REPLACE "=" "" _item ${_item})
                set(NCBI_TEAMCITY_BUILD_ID ${_item})
                break()
            endif()
        endforeach()
    else()
        message("$ENV{TEAMCITY_BUILD_PROPERTIES_FILE} DOES NOT EXIST")
    endif()
    if ("${NCBI_TEAMCITY_BUILD_ID}" STREQUAL "")
        string(RANDOM _name)
        string(UUID NCBI_TEAMCITY_BUILD_ID NAMESPACE "73203eb4-80d3-4957-a110-8aae92c7e615" NAME ${_name} TYPE SHA1)
    endif()
    message(STATUS "TeamCity build number = ${NCBI_TEAMCITY_BUILD_NUMBER}")
    message(STATUS "TeamCity project name = ${NCBI_TEAMCITY_PROJECT_NAME}")
    message(STATUS "TeamCity build conf   = ${NCBI_TEAMCITY_BUILDCONF_NAME}")
    message(STATUS "TeamCity build ID     = ${NCBI_TEAMCITY_BUILD_ID}")
endif()

#############################################################################
cmake_host_system_information(RESULT _local_host_name  QUERY HOSTNAME)
if (WIN32 OR XCODE)
    set(HOST "${HOST_CPU}-${HOST_OS}")
else()
#    set(HOST "${HOST_CPU}-unknown-${HOST_OS}")
    set(HOST "${HOST_CPU}-${HOST_OS}")
endif()
set(FEATURES ${NCBI_ALL_COMPONENTS};${NCBI_ALL_REQUIRES})
string(REPLACE ";" " " FEATURES "${FEATURES}")

set(_tk_common_include "${NCBITK_INC_ROOT}/common")
if (WIN32 OR XCODE)
    foreach(_cfg ${NCBI_CONFIGURATION_TYPES})

        set(_file "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_cfg}")
        if (WIN32)
            string(REPLACE "/" "\\\\" _file ${_file})
        endif()
        set(c_ncbi_runpath "${_file}")
        if (WIN32)
            string(REPLACE "/" "\\\\" SYBASE_PATH "${SYBASE_PATH}")
        endif()

        set(NCBI_SIGNATURE "${NCBI_COMPILER}_${NCBI_COMPILER_VERSION}-${_cfg}--${HOST_CPU}-${HOST_OS_WITH_VERSION}-${_local_host_name}")
        set(NCBI_SIGNATURE_CFG "${NCBI_COMPILER}_${NCBI_COMPILER_VERSION}-\$<CONFIG>--${HOST_CPU}-${HOST_OS_WITH_VERSION}-${_local_host_name}")
        set(NCBI_SIGNATURE_${_cfg} "${NCBI_SIGNATURE}")
if(OFF)
        if (WIN32)
            configure_file(${NCBI_TREE_CMAKECFG}/ncbiconf_msvc_site.h.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/config/ncbiconf_msvc_site.h)
        elseif (XCODE)
            configure_file(${NCBI_TREE_CMAKECFG}/ncbiconf_msvc_site.h.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/config/ncbiconf_xcode_site.h)
        endif()
else()
        if (WIN32)
            configure_file(${NCBI_TREE_CMAKECFG}/config.cmake.h.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/config/ncbiconf_msvc.h)
        elseif (XCODE)
            configure_file(${NCBI_TREE_CMAKECFG}/config.cmake.h.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/config/ncbiconf_xcode.h)
        endif()
endif()
        if (EXISTS ${NCBI_SRC_ROOT}/corelib/ncbicfg.c.in)
            configure_file(${NCBI_SRC_ROOT}/corelib/ncbicfg.c.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/config/ncbicfg.cfg.c)
        elseif (EXISTS ${NCBITK_SRC_ROOT}/corelib/ncbicfg.c.in)
            configure_file(${NCBITK_SRC_ROOT}/corelib/ncbicfg.c.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/config/ncbicfg.cfg.c)
        endif()
        configure_file(${_tk_common_include}/ncbi_build_ver.h.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/ncbi_build_ver.h)
        if (DEFINED NCBI_EXTERNAL_TREE_ROOT)
            configure_file(${_tk_common_include}/ncbi_revision.h.in ${NCBI_INC_ROOT}/common/ncbi_revision.h)
        else()
            if ($ENV{NCBI_AUTOMATED_BUILD})
                configure_file(${_tk_common_include}/ncbi_revision.h.in ${NCBI_CFGINC_ROOT}/${_cfg}/common/ncbi_revision.h)
            else()
                configure_file(${_tk_common_include}/ncbi_revision.h.in ${NCBITK_INC_ROOT}/common/ncbi_revision.h)
            endif()
        endif()
    endforeach()
    if(NOT EXISTS ${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_BUILD}/corelib/ncbicfg.c)
        file(WRITE ${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_BUILD}/corelib/ncbicfg.c "#include <common/config/ncbicfg.cfg.c>\n")
    endif()
    if (WIN32)
        if (BUILD_SHARED_LIBS)
            set(NCBITEST_SIGNATURE "${NCBI_COMPILER_ALT}-\$<CONFIG>MTdll64--${HOST_CPU}-win64-${_local_host_name}")
        else()
            set(NCBITEST_SIGNATURE "${NCBI_COMPILER_ALT}-\$<CONFIG>MTstatic64--${HOST_CPU}-win64-${_local_host_name}")
        endif()
    elseif (XCODE)
        if (BUILD_SHARED_LIBS)
            set(NCBITEST_SIGNATURE "${NCBI_COMPILER}_${NCBI_COMPILER_VERSION}-\$<CONFIG>MTdll64--${HOST_CPU}-${HOST_OS_WITH_VERSION}-${_local_host_name}")
        else()
            set(NCBITEST_SIGNATURE "${NCBI_COMPILER}_${NCBI_COMPILER_VERSION}-\$<CONFIG>MTstatic64--${HOST_CPU}-${HOST_OS_WITH_VERSION}-${_local_host_name}")
        endif()
    endif()
else()
#Linux
    set(c_ncbi_runpath ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(NCBI_TLS_VAR "__thread")

    set(NCBI_SIGNATURE "${NCBI_COMPILER}_${NCBI_COMPILER_VERSION}-${NCBI_BUILD_TYPE}--${HOST_CPU}-${HOST_OS_WITH_VERSION}-${_local_host_name}")
    if (BUILD_SHARED_LIBS)
        set(NCBITEST_SIGNATURE "${NCBI_COMPILER}_${NCBI_COMPILER_VERSION}-${CMAKE_BUILD_TYPE}MTdll64--${HOST_CPU}-${HOST_OS_WITH_VERSION}-${_local_host_name}")
    else()
        set(NCBITEST_SIGNATURE "${NCBI_COMPILER}_${NCBI_COMPILER_VERSION}-${CMAKE_BUILD_TYPE}MTstatic64--${HOST_CPU}-${HOST_OS_WITH_VERSION}-${_local_host_name}")
    endif()
    configure_file(${NCBI_TREE_CMAKECFG}/config.cmake.h.in ${NCBI_CFGINC_ROOT}/ncbiconf_unix.h)
    if (EXISTS ${NCBI_SRC_ROOT}/corelib/ncbicfg.c.in)
        configure_file(${NCBI_SRC_ROOT}/corelib/ncbicfg.c.in ${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_BUILD}/corelib/ncbicfg.c)
    elseif (EXISTS ${NCBITK_SRC_ROOT}/corelib/ncbicfg.c.in)
        configure_file(${NCBITK_SRC_ROOT}/corelib/ncbicfg.c.in ${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_BUILD}/corelib/ncbicfg.c)
    endif()

    configure_file(${_tk_common_include}/ncbi_build_ver.h.in ${NCBI_CFGINC_ROOT}/common/ncbi_build_ver.h)
    if (DEFINED NCBI_EXTERNAL_TREE_ROOT)
        configure_file(${_tk_common_include}/ncbi_revision.h.in ${NCBI_INC_ROOT}/common/ncbi_revision.h)
    else()
        if ($ENV{NCBI_AUTOMATED_BUILD})
            configure_file(${_tk_common_include}/ncbi_revision.h.in ${NCBI_CFGINC_ROOT}/common/ncbi_revision.h)
        else()
            configure_file(${_tk_common_include}/ncbi_revision.h.in ${NCBITK_INC_ROOT}/common/ncbi_revision.h)
        endif()
    endif()
endif()

endif(NOT NCBI_PTBCFG_COLLECT_REQUIRES)
