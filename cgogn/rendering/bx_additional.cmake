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
