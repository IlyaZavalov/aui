set(_gradle_project_dir ${CMAKE_CURRENT_BINARY_DIR}/gradle_project)
message("\n"
        "     ###############################################################################################\n"
        "     # AUI runs in Android build mode which means that AUI will generate Android Gradle project.   #\n"
        "     # Define your application with aui_app command.                                               #\n"
        "     # To build an apk, run the apks target.                                                       #\n"
        "     # To develop in Android Studio environment, open the gradle project dir (path is below).      #\n"
        "     ###############################################################################################\n"
        )
message(STATUS "Android gradle project dir: ${_gradle_project_dir}")

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${AUI_BUILD_AUI_ROOT}/platform/android/project ${_gradle_project_dir})

add_custom_target(apks ALL)


set(AUI_ANDROID_SDK_ROOT $ENV{ANDROID_SDK_ROOT} CACHE PATH "Path to Android SDK")

if (NOT AUI_ANDROID_SDK_ROOT)
    set(_probe "$ENV{HOME}/Android/Sdk")
    if (EXISTS ${_probe})
        message(STATUS "Found Android SDK at ${_probe}")
        set(AUI_ANDROID_SDK_ROOT ${_probe})
    endif()
endif()

if (AUI_ANDROID_SDK_ROOT)
    message(STATUS "Android SDK: ${AUI_ANDROID_SDK_ROOT}")
else()
    message(WARNING "ANDROID_SDK_ROOT may be required. You can set Android SDK location by defining AUI_ANDROID_SDK_ROOT "
            "CMake variable (i.e. -DAUI_ANDROID_SDK_ROOT=~/Android/Sdk) or by defining ANDROID_SDK_ROOT environment "
            "variable.")
endif()


function(_aui_android_app)
    if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(_gradle_exe "gradlew.bat")
    else()
        set(_gradle_exe "./gradlew")
    endif()

    add_custom_target(${APP_TARGET}.apk
                      COMMAND ${_gradle_exe} build
                      WORKING_DIRECTORY ${_gradle_project_dir}
            )

    set(_main "${_gradle_project_dir}/app/src/main")
    set(AUI_ANDROID_JAVA_SRC "${AUI_BUILD_AUI_ROOT}/platform/android/lib/src/java")
    string(REPLACE "." "/" _package_to_path ${APP_ANDROID_PACKAGE})

    configure_file(${AUI_BUILD_AUI_ROOT}/platform/android/settings.gradle.in ${_gradle_project_dir}/settings.gradle @ONLY)
    configure_file(${AUI_BUILD_AUI_ROOT}/platform/android/MainActivity.java.in ${_main}/java/${_package_to_path}/MainActivity.java @ONLY)
    configure_file(${AUI_BUILD_AUI_ROOT}/platform/android/AndroidManifest.xml.in ${_main}/AndroidManifest.xml @ONLY)
    configure_file(${AUI_BUILD_AUI_ROOT}/platform/android/app_build.gradle.in ${_gradle_project_dir}/app/build.gradle @ONLY)
    file(WRITE ${_gradle_project_dir}/local.properties "sdk.dir=${AUI_ANDROID_SDK_ROOT}")


    get_cmake_property(CACHE_VARS CACHE_VARIABLES)
    unset(ALL_CMAKE_ARGS)
    foreach(CACHE_VAR ${CACHE_VARS})
        if(CACHE_VAR MATCHES "^CMAKE_")
            continue()
        endif()
        get_property(_type CACHE ${CACHE_VAR} PROPERTY TYPE)
        if(_type STREQUAL INTERNAL)
            continue()
        endif()
        list(JOIN ${CACHE_VAR} "\;" _value)
        set(ALL_CMAKE_ARGS "${ALL_CMAKE_ARGS}set(${CACHE_VAR} ${_value} CACHE ${_type} \"\")\n")
    endforeach()
    file(WRITE ${_main}/CMakeLists.txt "cmake_minimum_required(VERSION 3.16)\n"
            "project(${APP_ANDROID_PACKAGE})\n"
            ${ALL_CMAKE_ARGS}
            "add_subdirectory(${CMAKE_SOURCE_DIR} build)")

    add_dependencies(apks ${APP_TARGET}.apk)
endfunction()