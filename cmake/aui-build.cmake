# CMake AUI building functions

cmake_minimum_required(VERSION 3.10)

ADD_DEFINITIONS(-DUNICODE)
add_custom_target(CompileAssets)

set(AUI_3RDPARTY_LIBS_DIR NOTFOUND CACHE PATH "")
if (AUI_3RDPARTY_LIBS_DIR)
    FILE(GLOB children RELATIVE ${AUI_3RDPARTY_LIBS_DIR} ${AUI_3RDPARTY_LIBS_DIR}/*)
    foreach(child ${children})
        if(IS_DIRECTORY ${AUI_3RDPARTY_LIBS_DIR}/${child})
            list(APPEND CMAKE_PREFIX_PATH ${AUI_3RDPARTY_LIBS_DIR}/${child})
        endif()
    endforeach()
    message("LIBRARY PATH: ${CMAKE_PREFIX_PATH}")
endif()

function(AUI_Add_Properties AUI_MODULE_NAME)
    if(MSVC)
        set_target_properties(${AUI_MODULE_NAME} PROPERTIES
                LINK_FLAGS "/force:MULTIPLE"
                COMPILE_FLAGS "/MP /utf-8")
    endif()

    if(NOT ANDROID)
        set_target_properties(${AUI_MODULE_NAME} PROPERTIES
                ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    endif()
endfunction(AUI_Add_Properties)

function(AUI_Tests TESTS_MODULE_NAME)
    add_executable(${ARGV})
    set_property(TARGET ${TESTS_MODULE_NAME} PROPERTY CXX_STANDARD 17)
    target_include_directories(${TESTS_MODULE_NAME} PUBLIC tests)
    find_package(Boost REQUIRED)
    add_definitions(-DBOOST_ALL_NO_LIB)
    target_include_directories(${TESTS_MODULE_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_directories(${TESTS_MODULE_NAME} PRIVATE ${Boost_LIBRARY_DIRS})
    target_link_libraries(${TESTS_MODULE_NAME} PRIVATE AUI.Core)
    target_link_libraries(${TESTS_MODULE_NAME} PRIVATE ${AUI_MODULE_NAME})
    AUI_Add_Properties(${TESTS_MODULE_NAME})
endfunction(AUI_Tests)

function(AUI_Common AUI_MODULE_NAME)
    set_property(TARGET ${AUI_MODULE_NAME} PROPERTY CXX_STANDARD 17)
    file(GLOB_RECURSE SRCS_TESTS_TMP tests/*.cpp tests/*.c tests/*.h)
    if (SRCS_TESTS_TMP)
        set_property(GLOBAL APPEND PROPERTY TESTS_DEPS ${AUI_MODULE_NAME})
        foreach(child ${SRCS_TESTS_TMP})
            set_property(GLOBAL APPEND PROPERTY TESTS_SRCS ${child})
        endforeach()
    endif()
    if(AUI_STATIC OR ANDROID)
        target_compile_definitions(${AUI_MODULE_NAME} INTERFACE AUI_STATIC)
    endif()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(${AUI_MODULE_NAME} INTERFACE AUI_DEBUG)
    else()
        target_compile_definitions(${AUI_MODULE_NAME} INTERFACE AUI_RELEASE)
    endif()
endfunction(AUI_Common)

function(AUI_Executable AUI_MODULE_NAME)
    project(${AUI_MODULE_NAME})

    file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp src/*.cpp src/*.c src/*.h)
    #message("ASSDIR ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp")
    if(ANDROID)
        add_library(${AUI_MODULE_NAME} SHARED ${SRCS})
    else()
        add_executable(${AUI_MODULE_NAME} ${SRCS})
    endif()

    target_include_directories(${AUI_MODULE_NAME} PRIVATE src)

    AUI_Add_Properties(${AUI_MODULE_NAME})

    AUI_Common(${AUI_MODULE_NAME})

    install(
            TARGETS ${AUI_MODULE_NAME}
            DESTINATION "bin"
    )

endfunction(AUI_Executable)

function(AUI_Executable_Advanced AUI_MODULE_NAME ADDITIONAL_SRCS)
    project(${AUI_MODULE_NAME})

    file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp src/*.cpp src/*.c src/*.h)
    #message("ASSDIR ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp")
    if(ANDROID)
        add_library(${AUI_MODULE_NAME} SHARED ${SRCS})
    else()
        add_executable(${AUI_MODULE_NAME} ${ADDITIONAL_SRCS} ${SRCS})
    endif()

    target_include_directories(${AUI_MODULE_NAME} PRIVATE src)

    AUI_Add_Properties(${AUI_MODULE_NAME})

    AUI_Common(${AUI_MODULE_NAME})
endfunction(AUI_Executable_Advanced)

function(AUI_Static_Link AUI_MODULE_NAME LIBRARY_NAME)
    target_include_directories(${AUI_MODULE_NAME} PRIVATE "3rdparty/${LIBRARY_NAME}")
    project(${LIBRARY_NAME})
    file(GLOB_RECURSE SRCS "3rdparty/${LIBRARY_NAME}/*.cpp" "3rdparty/${LIBRARY_NAME}/*.c" "3rdparty/${LIBRARY_NAME}/*.h")
    add_library(${LIBRARY_NAME} STATIC ${SRCS})
    target_link_libraries(${AUI_MODULE_NAME} PRIVATE ${LIBRARY_NAME})
endfunction(AUI_Static_Link)


function(AUI_Compile_Assets AUI_MODULE_NAME)
    set(TARGET_NAME "${AUI_MODULE_NAME}.Assets")

    if(ANDROID)
        set(TARGET_DIR ${AUI_SDK_BIN})
    else()
        get_target_property(TARGET_DIR ${AUI_MODULE_NAME} ARCHIVE_OUTPUT_DIRECTORY)
    endif()
    set(CMD_STRING ${TARGET_DIR}/AUI.Toolbox pack ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/autogen)
    add_custom_target(${TARGET_NAME} COMMAND ${CMD_STRING})
    add_dependencies(CompileAssets ${TARGET_NAME})
    add_dependencies(${AUI_MODULE_NAME} ${TARGET_NAME})
    if(NOT ANDROID)
        add_dependencies(${TARGET_NAME} AUI.Toolbox)
    endif()
endfunction(AUI_Compile_Assets)