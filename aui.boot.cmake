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

# TODO add a way to provide file access to the repository
macro(auib_import AUI_MODULE_NAME URL)
    cmake_policy(SET CMP0087 NEW)
    set(DEP_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/dependencies)
    # append our location to module path
    if (NOT "${DEP_INSTALL_PREFIX}" IN_LIST CMAKE_PREFIX_PATH)
        list(APPEND CMAKE_PREFIX_PATH ${DEP_INSTALL_PREFIX})
    endif()

    find_package(${AUI_MODULE_NAME} QUIET)

    if (NOT ${AUI_MODULE_NAME}_FOUND)
        include(FetchContent)
        # TODO add protocol check
        message(STATUS "Fetching ${AUI_MODULE_NAME}")
        FetchContent_Declare(${AUI_MODULE_NAME}_FC
                GIT_REPOSITORY "https://${URL}"
                GIT_PROGRESS TRUE # show progress of download
                USES_TERMINAL_DOWNLOAD TRUE # show progress in ninja generator
                )

        FetchContent_Populate(${AUI_MODULE_NAME}_FC)

        FetchContent_GetProperties(${AUI_MODULE_NAME}_FC
                SOURCE_DIR DEP_SOURCE_DIR
                BINARY_DIR DEP_BINARY_DIR)


        message(STATUS "Compiling ${AUI_MODULE_NAME}")

        execute_process(COMMAND ${CMAKE_COMMAND} ${DEP_SOURCE_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX:PATH=${DEP_INSTALL_PREFIX} -G "${CMAKE_GENERATOR}" ${ARGV2}
                WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                RESULT_VARIABLE STATUS_CODE)

        if (NOT STATUS_CODE EQUAL 0)
            message(FATAL_ERROR "CMake configure failed: ${STATUS_CODE}")
        endif()



        message(STATUS "Installing ${AUI_MODULE_NAME}")
        execute_process(COMMAND ${CMAKE_COMMAND} --build ${DEP_BINARY_DIR} --target install --prefix ${DEP_INSTALL_PREFIX}
                WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                RESULT_VARIABLE ERROR_CODE)

        if (NOT STATUS_CODE EQUAL 0)
            message(FATAL_ERROR "CMake build failed: ${STATUS_CODE}")
        endif()

        find_package(${AUI_MODULE_NAME} REQUIRED)
    endif()

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