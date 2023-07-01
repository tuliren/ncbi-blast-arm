#############################################################################
# $Id: CMake.NCBIptb.grpc.cmake 649862 2022-05-11 15:15:11Z gouriano $
#############################################################################
#############################################################################
##
##  NCBI CMake wrapper extension
##  In NCBI CMake wrapper, adds support of GRPC/PROTOBUF code generation
##    Author: Andrei Gourianov, gouriano@ncbi
##


##############################################################################
function(NCBI_internal_process_proto_dataspec _variable _access _value)
    if(NOT "${_access}" STREQUAL "MODIFIED_ACCESS" OR "${_value}" STREQUAL "")
        return()
    endif()
    cmake_parse_arguments(DT "GENERATE" "DATASPEC;RETURN" "REQUIRES" ${_value})

    get_filename_component(_path     ${DT_DATASPEC} DIRECTORY)
    get_filename_component(_basename ${DT_DATASPEC} NAME_WE)
    get_filename_component(_ext      ${DT_DATASPEC} EXT)
    file(RELATIVE_PATH     _relpath  ${NCBI_SRC_ROOT} ${_path})

    set(_specfiles  ${DT_DATASPEC})
    set(_pb_srcfiles "")
    set(_pb_incfiles "")
    if (EXISTS "${NCBI_PROTOC_APP}" AND "PROTOBUF" IN_LIST DT_REQUIRES)
        set(_pb_srcfiles ${_path}/${_basename}.pb.cc)
        set(_pb_incfiles ${NCBI_INC_ROOT}/${_relpath}/${_basename}.pb.h)
    endif()
    set(_gr_srcfiles "")
    set(_gr_incfiles "")
    if (EXISTS "${NCBI_PROTOC_APP}" AND EXISTS "${NCBI_GRPC_PLUGIN}" AND "GRPC" IN_LIST DT_REQUIRES)
        set(_gr_srcfiles ${_path}/${_basename}.grpc.pb.cc)
        set(_gr_incfiles ${NCBI_INC_ROOT}/${_relpath}/${_basename}.grpc.pb.h)
    endif()
    if(NOT "${DT_RETURN}" STREQUAL "")
#        set(${DT_RETURN} DATASPEC ${_specfiles} SOURCES ${_pb_srcfiles} ${_gr_srcfiles} HEADERS ${_pb_incfiles} ${_gr_incfiles} INCLUDE ${NCBI_INC_ROOT}/${_relpath} PARENT_SCOPE)
        set(${DT_RETURN} DATASPEC ${_specfiles} SOURCES ${_pb_srcfiles} ${_gr_srcfiles} HEADERS ${_pb_incfiles} ${_gr_incfiles} PARENT_SCOPE)
    endif()

    if(NOT DT_GENERATE)
        return()
    endif()

    if (EXISTS "${NCBI_PROTOC_APP}" AND "PROTOBUF" IN_LIST DT_REQUIRES)
        set(_cmd ${NCBI_PROTOC_APP} --cpp_out=. -I. ${_relpath}/${_basename}${_ext})
        if("${NCBI_SRC_ROOT}" STREQUAL "${NCBI_INC_ROOT}")
            add_custom_command(
                OUTPUT ${_pb_srcfiles} ${_pb_incfiles}
                COMMAND ${_cmd} VERBATIM
                WORKING_DIRECTORY ${NCBI_SRC_ROOT}
                COMMENT "Generate PROTOC C++ classes from ${DT_DATASPEC}"
                DEPENDS ${DT_DATASPEC}
                VERBATIM
            )
        else()
            add_custom_command(
                OUTPUT ${_pb_srcfiles} ${_pb_incfiles}
                COMMAND ${_cmd} VERBATIM
                COMMAND ${CMAKE_COMMAND} -E make_directory ${NCBI_INC_ROOT}/${_relpath}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_path}/${_basename}.pb.h ${NCBI_INC_ROOT}/${_relpath} VERBATIM
                COMMAND ${CMAKE_COMMAND} -E remove -f ${_path}/${_basename}.pb.h VERBATIM
                WORKING_DIRECTORY ${NCBI_SRC_ROOT}
                COMMENT "Generate PROTOC C++ classes from ${DT_DATASPEC}"
                DEPENDS ${DT_DATASPEC}
                VERBATIM
            )
        endif()

        if(NOT NCBI_PTBCFG_PACKAGED AND NOT NCBI_PTBCFG_PACKAGING)
            if(WIN32)
                set(_app \"${NCBI_PROTOC_APP}\")
                set(_cmk \"${CMAKE_COMMAND}\")
            else()
                set(_app ${NCBI_PROTOC_APP})
                set(_cmk ${CMAKE_COMMAND})
            endif()
            file(APPEND ${NCBI_GENERATESRC_GRPC} "echo ${NCBI_SRC_ROOT}/${_relpath}/${_basename}${_ext}\n")
            file(APPEND ${NCBI_GENERATESRC_GRPC} "cd ${NCBI_SRC_ROOT}\n")
            file(APPEND ${NCBI_GENERATESRC_GRPC} "${_app} --cpp_out=. -I. ${_relpath}/${_basename}${_ext}\n")
            if(WIN32)
                file(APPEND ${NCBI_GENERATESRC_GRPC} "if errorlevel 1 (set GENERATESRC_RESULT=1)\n")
            else()
                file(APPEND ${NCBI_GENERATESRC_GRPC} "test $? -eq 0 || GENERATESRC_RESULT=1\n")
            endif()
            if(NOT "${NCBI_SRC_ROOT}" STREQUAL "${NCBI_INC_ROOT}")
                file(APPEND ${NCBI_GENERATESRC_GRPC} "${_cmk} -E make_directory ${NCBI_INC_ROOT}/${_relpath}\n")
                file(APPEND ${NCBI_GENERATESRC_GRPC} "${_cmk} -E copy_if_different ${_path}/${_basename}.pb.h ${NCBI_INC_ROOT}/${_relpath}\n")
                file(APPEND ${NCBI_GENERATESRC_GRPC} "${_cmk} -E remove -f ${_path}/${_basename}.pb.h\n")
            endif()
        endif()
    endif()
    if (EXISTS "${NCBI_PROTOC_APP}" AND EXISTS "${NCBI_GRPC_PLUGIN}" AND "GRPC" IN_LIST DT_REQUIRES)
        set(_cmd ${NCBI_PROTOC_APP} --grpc_out=generate_mock_code=true:. --plugin=protoc-gen-grpc=${NCBI_GRPC_PLUGIN} -I. ${_relpath}/${_basename}${_ext})
        if("${NCBI_SRC_ROOT}" STREQUAL "${NCBI_INC_ROOT}")
            add_custom_command(
                OUTPUT ${_gr_srcfiles} ${_gr_incfiles}
                COMMAND ${_cmd} VERBATIM
                WORKING_DIRECTORY ${NCBI_SRC_ROOT}
                COMMENT "Generate GRPC C++ classes from ${DT_DATASPEC}"
                DEPENDS ${DT_DATASPEC}
                VERBATIM
            )
        else()
            add_custom_command(
                OUTPUT ${_gr_srcfiles} ${_gr_incfiles}
                COMMAND ${_cmd} VERBATIM
                COMMAND ${CMAKE_COMMAND} -E make_directory ${NCBI_INC_ROOT}/${_relpath}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_path}/${_basename}.grpc.pb.h ${NCBI_INC_ROOT}/${_relpath} VERBATIM
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_path}/${_basename}_mock.grpc.pb.h ${NCBI_INC_ROOT}/${_relpath} VERBATIM
                COMMAND ${CMAKE_COMMAND} -E remove -f ${_path}/${_basename}.grpc.pb.h VERBATIM
                COMMAND ${CMAKE_COMMAND} -E remove -f ${_path}/${_basename}_mock.grpc.pb.h VERBATIM
                WORKING_DIRECTORY ${NCBI_SRC_ROOT}
                COMMENT "Generate GRPC C++ classes from ${DT_DATASPEC}"
                DEPENDS ${DT_DATASPEC}
                VERBATIM
            )
        endif()

        if(NOT NCBI_PTBCFG_PACKAGED AND NOT NCBI_PTBCFG_PACKAGING)
            if(WIN32)
                set(_app \"${NCBI_PROTOC_APP}\")
                set(_plg \"${NCBI_GRPC_PLUGIN}\")
                set(_cmk \"${CMAKE_COMMAND}\")
            else()
                set(_app ${NCBI_PROTOC_APP})
                set(_plg ${NCBI_GRPC_PLUGIN})
                set(_cmk ${CMAKE_COMMAND})
            endif()
            file(APPEND ${NCBI_GENERATESRC_GRPC} "echo ${NCBI_SRC_ROOT}/${_relpath}/${_basename}${_ext}\n")
            file(APPEND ${NCBI_GENERATESRC_GRPC} "cd ${NCBI_SRC_ROOT}\n")
            file(APPEND ${NCBI_GENERATESRC_GRPC} "${_app} --grpc_out=generate_mock_code=true:. --plugin=protoc-gen-grpc=${_plg} -I. ${_relpath}/${_basename}${_ext}\n")
            if(WIN32)
                file(APPEND ${NCBI_GENERATESRC_GRPC} "if errorlevel 1 (set GENERATESRC_RESULT=1)\n")
            else()
                file(APPEND ${NCBI_GENERATESRC_GRPC} "test $? -eq 0 || GENERATESRC_RESULT=1\n")
            endif()
            if(NOT "${NCBI_SRC_ROOT}" STREQUAL "${NCBI_INC_ROOT}")
                file(APPEND ${NCBI_GENERATESRC_GRPC} "${_cmk} -E make_directory ${NCBI_INC_ROOT}/${_relpath}\n")
                file(APPEND ${NCBI_GENERATESRC_GRPC} "${_cmk} -E copy_if_different ${_path}/${_basename}.grpc.pb.h ${NCBI_INC_ROOT}/${_relpath}\n")
                file(APPEND ${NCBI_GENERATESRC_GRPC} "${_cmk} -E copy_if_different ${_path}/${_basename}_mock.grpc.pb.h ${NCBI_INC_ROOT}/${_relpath}\n")
                file(APPEND ${NCBI_GENERATESRC_GRPC} "${_cmk} -E remove -f ${_path}/${_basename}.grpc.pb.h\n")
                file(APPEND ${NCBI_GENERATESRC_GRPC} "${_cmk} -E remove -f ${_path}/${_basename}_mock.grpc.pb.h\n")
            endif()
        endif()
    endif()
endfunction()

##############################################################################
function(NCBI_internal_adjust_proto_target _variable _access _value)
    if(NOT "${_access}" STREQUAL "MODIFIED_ACCESS" OR "${_value}" STREQUAL "")
        return()
    endif()
    if( (DEFINED NCBI_${NCBI_PROJECT}_REQUIRES AND (GRPC IN_LIST NCBI_${NCBI_PROJECT}_REQUIRES OR PROTOBUF IN_LIST NCBI_${NCBI_PROJECT}_REQUIRES)) OR
        (DEFINED NCBI_${NCBI_PROJECT}_COMPONENTS AND (GRPC IN_LIST NCBI_${NCBI_PROJECT}_COMPONENTS OR PROTOBUF IN_LIST NCBI_${NCBI_PROJECT}_COMPONENTS)))
        if(DEFINED NCBI_${NCBI_PROJECT}_DATASPEC)
            foreach(_spec IN LISTS NCBI_${NCBI_PROJECT}_DATASPEC)
                if("${_spec}" MATCHES "[.]proto$")
                    set_property(TARGET ${NCBI_PROJECT} PROPERTY CXX_STANDARD 14)
                    return()
                endif()
            endforeach()
        endif()
    endif()
endfunction()

#############################################################################
NCBI_register_hook(DATASPEC NCBI_internal_process_proto_dataspec ".proto")
#NCBI_register_hook(TARGET_ADDED NCBI_internal_adjust_proto_target)

if(NOT NCBI_PTBCFG_PACKAGED AND NOT NCBI_PTBCFG_PACKAGING)
    set(NCBI_GENERATESRC_GRPC   ${NCBI_BUILD_ROOT}/${NCBI_DIRNAME_BUILD}/CMakeFiles/generate_sources.grpc)
    if (EXISTS ${NCBI_GENERATESRC_GRPC})
        file(REMOVE ${NCBI_GENERATESRC_GRPC})
    endif()
endif()
