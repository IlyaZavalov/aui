cmake_minimum_required(VERSION 3.10)

AUI_Module(AUI.Views)

AUI_Compile_Assets(AUI.Views)

if(MSVC)
	set(GLEW_USE_STATIC_LIBS true)
endif()

find_package(Freetype)

if(NOT ANDROID)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
	include_directories (/usr/include/freetype2)


	if (UNIX)
		FIND_PACKAGE(PkgConfig REQUIRED)
		PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

		include_directories (${GTK3_INCLUDE_DIRS})
		link_directories (${GTK3_LIBRARY_DIRS})
		target_link_libraries(AUI.Views gtk-3)
	endif (UNIX)
endif()


add_definitions(-DGLEW_STATIC)

target_include_directories(AUI.Views PRIVATE ${GLEW_INCLUDE_DIRS})
target_include_directories(AUI.Views PRIVATE ${FREETYPE_INCLUDE_DIRS})

target_link_libraries(AUI.Views PUBLIC AUI.Core)

if(ANDROID)
	target_link_libraries(AUI.Views PRIVATE EGL GLESv2 GLESv3)
	target_link_libraries(AUI.Views PRIVATE freetype)
else()
	target_link_libraries(AUI.Views PRIVATE Freetype::Freetype)
	target_link_libraries(AUI.Views PRIVATE OpenGL::GL)

	if(MSVC)
		target_link_libraries(AUI.Views PRIVATE GLEW::glew_s)
		target_link_libraries(AUI.Views PRIVATE dwmapi)
		target_link_libraries(AUI.Views PRIVATE Winmm)
	else()
		target_link_libraries(AUI.Views PRIVATE GLEW::glew)
	endif()
endif()
