cmake_minimum_required(VERSION 3.15)
project(simpleICP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(Eigen3 REQUIRED NO_MODULE)

if(Eigen3_FOUND)
    if(Eigen3_VERSION VERSION_LESS 3.3.7 OR Eigen3_VERSION VERSION_GREATER 3.4.0)
        message("Eigen3 version found: ${Eigen3_VERSION}")
        message(FATAL_ERROR "Eigen3 version must be between 3.3.7 and 3.4.0")
    endif()
else()
	message(FATAL_ERROR "Eigen3 not found")

endif()
set(HEADER_FILES
        simpleicp.h
        pointcloud.h
        corrpts.h
        nanoflann.hpp
)

set(SOURCE_FILES
        simpleicp.cpp
        pointcloud.cpp
        corrpts.cpp
)

add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/cgogn/thirdparty>
)

target_link_libraries(${PROJECT_NAME} PUBLIC Eigen3::Eigen)

if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE "/bigobj")
endif()

install(FILES ${HEADER_FILES} DESTINATION "include/cgogn/thirdparty")

cgogn_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER thirdparty)

