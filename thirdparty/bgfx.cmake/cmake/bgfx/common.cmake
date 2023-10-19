cmake_minimum_required(VERSION 3.10.2) # version supported by android studio

project(common LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(
		GLOB_RECURSE
		COMMON_HEADERS
		${BGFX_DIR}/examples/common/*.h
		${BGFX_DIR}/examples/common/*.hpp
		${BGFX_DIR}/examples/common/*.inl
	)

	file(
		GLOB_RECURSE
		COMMON_SOURCES
		${BGFX_DIR}/examples/common/*.cpp
		${BGFX_DIR}/examples/common/*.c)

	# Add library target
	add_library(common STATIC
		${COMMON_HEADERS}
		${COMMON_SOURCES}
	)

	# Include directories for the library
	target_include_directories(
		common PUBLIC
		${BGFX_DIR}/examples/thirdparty
		${DEAR_IMGUI_INCLUDE_DIR}
		${MESHOPTIMIZER_INCLUDE_DIR}
	)

	# Link the common library with bgfx, bx, and bimg
	target_link_libraries(common PUBLIC
	bgfx
	bx
	bimg
	${DEAR_IMGUI_LIBRARIES} 
	${MESHOPTIMIZER_LIBRARIES}
	)


	#install under common
	install(FILES ${COMMON_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/thirdparty/common)