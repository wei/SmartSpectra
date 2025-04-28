###########################################################
# SmartSpectra_version.cmake
# Created by Greg on 9/4/2024.
# Copyright (C) 2024 Presage Security, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
###########################################################
function(SmartSpectra_get_version_from_py_file PREFIX PY_FILE_PATH)
    file(READ "${PY_FILE_PATH}" VERSION_FILE_CONTENTS)

    string(REGEX MATCH "[\"']([0-9]+)[.]([0-9]+)[.]([0-9]+)(-rc[.]([0-9]+))?[\"']" MATCHES ${VERSION_FILE_CONTENTS})

    set(${PREFIX}_VERSION_MAJOR ${CMAKE_MATCH_1} PARENT_SCOPE)
    set(${PREFIX}_VERSION_MINOR ${CMAKE_MATCH_2} PARENT_SCOPE)
    set(${PREFIX}_VERSION_PATCH ${CMAKE_MATCH_3} PARENT_SCOPE)

    if (DEFINED CMAKE_MATCH_5)
        set(${PREFIX}_VERSION_PRERELEASE ${CMAKE_MATCH_5})
        set(${PREFIX}_VERSION_PRERELEASE_DEBIAN_POSTFIX "~rc${CMAKE_MATCH_5}")
    else()
        set(${PREFIX}_VERSION_PRERELEASE "")
        set(${PREFIX}_VERSION_PRERELEASE_DEBIAN_POSTFIX "")
    endif()
    set(${PREFIX}_VERSION_PRERELEASE ${${PREFIX}_VERSION_PRERELEASE} PARENT_SCOPE)
    set(${PREFIX}_VERSION_PRERELEASE_DEBIAN_POSTFIX ${${PREFIX}_VERSION_PRERELEASE_DEBIAN_POSTFIX} PARENT_SCOPE)

    execute_process(COMMAND git branch --show-current RESULT_VARIABLE GIT_RESULT OUTPUT_VARIABLE GIT_BRANCH)

    if(NOT GIT_RESULT EQUAL 0)
        set(UNKNOWN_VERSION_BRANCH "-unknown")
        message(WARNING "Failed to get git branch; error code: ${GIT_RESULT}; \
                assuming either not in git repository or git is unavailable and setting \
                ${PREFIX}_VERSION_BRANCH to \"${UNKNOWN_VERSION_BRANCH}\"")
        set(${PREFIX}_VERSION_BRANCH ${UNKNOWN_VERSION_BRANCH})
    else ()
        set(${PREFIX}_VERSION_BRANCH "-dev")
        if ("${GIT_BRANCH}" STREQUAL "main\n")
            set(${PREFIX}_VERSION_BRANCH "")
        elseif ("${GIT_BRANCH}" STREQUAL "test\n")
            set(${PREFIX}_VERSION_BRANCH "-test")
        endif()
    endif()
    set(${PREFIX}_VERSION_BRANCH ${${PREFIX}_VERSION_BRANCH} PARENT_SCOPE)

    set(${PREFIX}_VERSION_PLAIN "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}" PARENT_SCOPE)
    set(${PREFIX}_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}${${PREFIX}_VERSION_PRERELEASE_DEBIAN_POSTFIX}${${PREFIX}_VERSION_BRANCH}")
    set(${PREFIX}_VERSION ${${PREFIX}_VERSION} PARENT_SCOPE)

    message(STATUS "${PREFIX}_VERSION: ${${PREFIX}_VERSION}")
endfunction()
