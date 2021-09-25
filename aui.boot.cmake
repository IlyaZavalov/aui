#
# =====================================================================================================================
# Copyright (c) 2021 Alex2772
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Original code located at https://github.com/aui-framework/aui
# =====================================================================================================================
#

set(CMAKE_POLICY_DEFAULT_CMP0074 NEW)

if(WIN32 AND NOT CYGWIN)
    set(HOME_DIR $ENV{USERPROFILE})
else()
    set(HOME_DIR $ENV{HOME})
endif()

if (NOT CMAKE_CXX_COMPILER_ID)
    message(FATAL_ERROR "Please include aui.boot AFTER project() call.")
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(AUI_TARGET_PROCESSOR_NAME "x86_64")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(AUI_TARGET_PROCESSOR_NAME "x86")
    endif()
else ()
    set(AUI_TARGET_PROCESSOR_NAME ${CMAKE_SYSTEM_PROCESSOR})
endif ()


set(AUI_CACHE_DIR ${HOME_DIR}/.aui CACHE PATH "Path to AUI.Boot cache")
string(TOLOWER "${CMAKE_CXX_COMPILER_ID}-${AUI_TARGET_PROCESSOR_NAME}" _tmp)
set(AUI_TARGET_ABI "${_tmp}" CACHE STRING "COMPILER-PROCESSOR pair")
message(STATUS "AUI.Boot cache: ${AUI_CACHE_DIR}")
message(STATUS "AUI.Boot target ABI: ${AUI_TARGET_ABI}")

# create all required dirs
if (NOT EXISTS ${AUI_CACHE_DIR})
    file(MAKE_DIRECTORY ${AUI_CACHE_DIR})
endif()
if (NOT EXISTS ${AUI_CACHE_DIR}/prefix)
    file(MAKE_DIRECTORY ${AUI_CACHE_DIR}/prefix)
endif()
if (NOT EXISTS ${AUI_CACHE_DIR}/repo)
    file(MAKE_DIRECTORY ${AUI_CACHE_DIR}/repo)
endif()

# TODO add a way to provide file access to the repository
macro(auib_import AUI_MODULE_NAME URL TAG_OR_HASH)
    cmake_policy(SET CMP0087 NEW)
    cmake_policy(SET CMP0074 NEW)

    set(DEP_INSTALL_PREFIX "${AUI_CACHE_DIR}/prefix/${AUI_MODULE_NAME}/${TAG_OR_HASH}/${AUI_TARGET_ABI}/${CMAKE_BUILD_TYPE}")

    # append our location to module path
    #if (NOT "${DEP_INSTALL_PREFIX}" IN_LIST CMAKE_PREFIX_PATH)
    #    list(APPEND CMAKE_PREFIX_PATH ${DEP_INSTALL_PREFIX})
    #endif()

    set(${AUI_MODULE_NAME}_DIR ${DEP_INSTALL_PREFIX})

    string(REGEX REPLACE "[a-z]+:\\/\\/" "" URL_PATH ${URL})
    set(DEP_SOURCE_DIR "${AUI_CACHE_DIR}/repo/${URL_PATH}")
    if (NOT ${AUI_MODULE_NAME}_ROOT)
        include(FetchContent)
        # TODO add protocol check
        message(STATUS "Fetching ${AUI_MODULE_NAME}")

        FetchContent_Declare(${AUI_MODULE_NAME}_FC
                GIT_REPOSITORY "${URL}"
                GIT_TAG ${TAG_OR_HASH}
                GIT_PROGRESS TRUE # show progress of download
                USES_TERMINAL_DOWNLOAD TRUE # show progress in ninja generator
                SOURCE_DIR ${DEP_SOURCE_DIR}
                )

        FetchContent_Populate(${AUI_MODULE_NAME}_FC)

        FetchContent_GetProperties(${AUI_MODULE_NAME}_FC
                                   BINARY_DIR DEP_BINARY_DIR)


        message(STATUS "Compiling ${AUI_MODULE_NAME}")

        execute_process(COMMAND ${CMAKE_COMMAND} ${DEP_SOURCE_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX:PATH=${DEP_INSTALL_PREFIX} -G "${CMAKE_GENERATOR}"
                WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                RESULT_VARIABLE STATUS_CODE)

        if (NOT STATUS_CODE EQUAL 0)
            message(FATAL_ERROR "CMake configure failed: ${STATUS_CODE}")
        endif()



        message(STATUS "Installing ${AUI_MODULE_NAME}")
        execute_process(COMMAND ${CMAKE_COMMAND} --build ${DEP_BINARY_DIR} --target install
                WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                RESULT_VARIABLE ERROR_CODE)

        if (NOT STATUS_CODE EQUAL 0)
            message(FATAL_ERROR "CMake build failed: ${STATUS_CODE}")
        endif()

        set(${AUI_MODULE_NAME}_ROOT ${DEP_INSTALL_PREFIX} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} provided by AUI.Boot.")
    endif()
    find_package(${AUI_MODULE_NAME} REQUIRED)

    # add dependencies' folders to install
    # WHY CMAKE COULDN'T CHECK WHETHER FILE OR NOT???
    file(GLOB DEP_FILES_LIST LIST_DIRECTORIES false ${DEP_INSTALL_PREFIX}/*)
    file(GLOB DEP_FILES_DIRS_LIST LIST_DIRECTORIES true ${DEP_INSTALL_PREFIX}/*)
    foreach(_item ${DEP_FILES_DIRS_LIST})
        if (${_item} IN_LIST DEP_FILES_LIST)
            install(
                    FILES ${_item}
                    DESTINATION .
                    OPTIONAL
            )
        else()
            install(
                DIRECTORY ${_item}
                DESTINATION .
                OPTIONAL
            )
        endif()
    endforeach()

endmacro()