################################################################################
# Copyright (c) 2020 Vladislav Trifochkin
#
# This file is part of [portable-target](https://github.com/semenovf/portable-target).
#
# Changelog:
#      2020.06.25 Initial version
################################################################################
include(CMakeParseArguments)

function (find_directory DIRNAME)
    set(boolparm)
    set(singleparm FILENAME RELATIVE)
    set(multiparm)

    # parse the macro arguments
    cmake_parse_arguments(_arg "${boolparm}" "${singleparm}" "${multiparm}" ${ARGN})

    if (NOT _arg_FILENAME)
        message(FATAL_ERROR "find_directory: FILENAME must be specified")
    endif()

    if (NOT _arg_RELATIVE)
        message(FATAL_ERROR "find_directory: RELATIVE directory must be specified")
    endif()

    file(GLOB_RECURSE _dirname RELATIVE ${_arg_RELATIVE} ${_arg_FILENAME})
    list(LENGTH _dirname _dirname_count)

    if (NOT "${_dirname_count}" EQUAL "0")
        list(GET _dirname 0 _dirname)
        get_filename_component(_dirname "${_dirname}" DIRECTORY)
        get_filename_component(_dirname "${_dirname}/../.." ABSOLUTE)

        set(${DIRNAME} ${_dirname} PARENT_SCOPE)
    endif()
endfunction()
