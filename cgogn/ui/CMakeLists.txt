project(cgogn_ui
	LANGUAGES CXX C
)
set(EXCLUDE_PLATFORM_H TRUE)

find_package(cgogn_core REQUIRED)
find_package(cgogn_rendering REQUIRED)
find_package(ImGUI REQUIRED)
find_package(synapse REQUIRED)


add_library(${PROJECT_NAME} STATIC "")
add_library(cgogn::ui ALIAS ${PROJECT_NAME})

set(src_list
		"${CMAKE_CURRENT_LIST_DIR}/app.h"
		"${CMAKE_CURRENT_LIST_DIR}/app.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/camera.h"
		"${CMAKE_CURRENT_LIST_DIR}/camera.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/gl_viewer.h"
		"${CMAKE_CURRENT_LIST_DIR}/gl_viewer.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/imgui_helpers.h"
		"${CMAKE_CURRENT_LIST_DIR}/inputs.h"
		"${CMAKE_CURRENT_LIST_DIR}/module.h"
		"${CMAKE_CURRENT_LIST_DIR}/module.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/moving_frame.h"
		"${CMAKE_CURRENT_LIST_DIR}/view.h"
		"${CMAKE_CURRENT_LIST_DIR}/view.cpp"
)
target_sources(${PROJECT_NAME} PRIVATE ${src_list})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${src_list})

set_target_properties(${PROJECT_NAME} PROPERTIES
	DEBUG_POSTFIX "_d"
	EXPORT_NAME ui
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}/thirdparty>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}/thirdparty/synapse/include>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}/thirdparty/bgfx.cmake/bgfx/include>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}/thirdparty/bgfx.cmake/bimg/include>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/cgogn/thirdparty>
)

# Build system specific configurations
if(MINGW)
	set(BX_COMPAT_PLATFORM mingw)
elseif(WIN32)
	set(BX_COMPAT_PLATFORM msvc)
elseif(APPLE) # APPLE is technically UNIX... ORDERING MATTERS!
	set(BX_COMPAT_PLATFORM osx)
elseif(UNIX)
	set(BX_COMPAT_PLATFORM linux)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${BX_DIR}/include> #
	$<BUILD_INTERFACE:${BX_DIR}/3rdparty> #
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> #
	$<BUILD_INTERFACE:${BX_DIR}/include/compat/${BX_COMPAT_PLATFORM}> #
$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/bx/compat/${BX_COMPAT_PLATFORM}> 
)

# All configurations
target_compile_definitions(${PROJECT_NAME} PUBLIC "BX_CONFIG_DEBUG=$<IF:$<CONFIG:Debug>,1,$<BOOL:${BX_CONFIG_DEBUG}>>")
target_compile_definitions(${PROJECT_NAME} PUBLIC "__STDC_LIMIT_MACROS")
target_compile_definitions(${PROJECT_NAME} PUBLIC "__STDC_FORMAT_MACROS")
target_compile_definitions(${PROJECT_NAME} PUBLIC "__STDC_CONSTANT_MACROS")

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
# (note: see bx\scripts\toolchain.lua for equivalent compiler flag)
target_compile_options(${PROJECT_NAME} PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>)


#	$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
#	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/thirdparty/bgfx.cmake/bx/include>
#	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
#	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/cgogn/thirdparty>	
#)

target_link_libraries(${PROJECT_NAME} cgogn::core cgogn::rendering ImGUI synapse)

if (CGOGN_GL43_DEBUG_MODE AND NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(${PROJECT_NAME} PRIVATE "-DCGOGN_GL43_DEBUG_MODE=1")
endif()

# Write out cgogn_ui_export.h to the current binary directory
generate_export_header(${PROJECT_NAME})

set(PKG_CONFIG_REQUIRES "cgogn_core cgogn_rendering ImGUI synapse")
configure_file(${PROJECT_SOURCE_DIR}/cgogn_ui.pc.in ${CMAKE_CURRENT_BINARY_DIR}/cgogn_ui.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cgogn_ui.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
	COMPONENT cgogn_ui_libraries
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cgogn/ui
	COMPONENT cgogn_ui_headers
	FILES_MATCHING PATTERN "*.h"
	REGEX "(examples|tests)" EXCLUDE
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cgogn_ui_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cgogn/ui
	COMPONENT cgogn_ui_headers
)

cgogn_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER cgogn)

#include(${CMAKE_SOURCE_DIR}/thirdparty/bgfx.cmake/bgfxConfig.cmake.in)


#find_package(bgfx REQUIRED)