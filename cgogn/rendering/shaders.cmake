
# execute make for all subfolders of shaders_sc folder
if(NOT EXISTS ${CMAKE_BINARY_DIR}/stage/bin/shaders_utils)
	file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/stage/bin/shaders_utils)
endif()
file(WRITE ${CMAKE_BINARY_DIR}/stage/bin/shaders_utils/makefile "
.PHONY: all
all: compile


SHADERC ?= DefaultString

INPUT ?= DefaultString

OUTPUT ?= DefaultString

INCLUDE1 ?= DefaultString

INCLUDE2 ?= DefaultString

SHADERTYPE ?= DefaultString

PROFILE ?= DefaultString

compile:\n
\t$(SHADERC) -f $(INPUT) -o $(OUTPUT) -i $(INCLUDE1) -i $(INCLUDE2)  --type $(SHADERTYPE) -p $(PROFILE)

")

file(GLOB shader_folders "${CMAKE_CURRENT_SOURCE_DIR}/shaders_sc/*")
if(NOT EXISTS ${CMAKE_BINARY_DIR}/stage/bin/shaders)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/stage/bin/shaders)
endif()

set(SHADERC ${CMAKE_BINARY_DIR}/stage/bin/shaderc)
set(INCLUDE1 ${CMAKE_SOURCE_DIR}/thirdparty/bgfx.cmake/bgfx/examples/thirdparty/thirdparty/common)
set(INCLUDE2 ${CMAKE_SOURCE_DIR}/thirdparty/bgfx.cmake/bgfx/src)


foreach(shader_folder ${shader_folders})
	

	# execute make command
	# strip makefile from path
	get_filename_component(shader_folder_name ${shader_folder} NAME)
	if(EXISTS ${CMAKE_BINARY_DIR}/stage/bin/shaders/${shader_folder_name})
		file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/stage/bin/shaders/${shader_folder_name})
	endif()
	file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/stage/bin/shaders/${shader_folder_name})
	file(GLOB FSHADERS "${shader_folder}/fs*.sc")
	file(GLOB VSHADERS "${shader_folder}/vs*.sc")

	foreach(shader ${FSHADERS})
		get_filename_component(shader_name ${shader} NAME)
		get_filename_component(shader_name_we ${shader} NAME_WE)
		set (output_shader ${CMAKE_BINARY_DIR}/stage/bin/shaders/${shader_folder_name}/${shader_name_we}.bin)
		execute_process(
			COMMAND
			make -C ${CMAKE_BINARY_DIR}/stage/bin/shaders_utils SHADERC=${SHADERC} INPUT=${shader} OUTPUT=${output_shader} INCLUDE1=${INCLUDE1} INCLUDE2=${INCLUDE2} SHADERTYPE=fragment PROFILE=150
		)
		#add_custom_command(
		#	OUTPUT ${CMAKE_BINARY_DIR}/stage/bin/shaders/${shader_folder_name}/out.txt
		#	COMMAND ${SHADERC} -f ${shader} -o ${output_shader} -i ${INCLUDE1} -i ${INCLUDE2}  --type fragment -p 150 
		#	DEPENDS ${shader}
		#)	 
	endforeach()

		foreach(shader ${VSHADERS})
		get_filename_component(shader_name ${shader} NAME)
		get_filename_component(shader_name_we ${shader} NAME_WE)
		set (output_shader ${CMAKE_BINARY_DIR}/stage/bin/shaders/${shader_folder_name}/${shader_name_we}.bin)
		execute_process(
			COMMAND
			make -C ${CMAKE_BINARY_DIR}/stage/bin/shaders_utils SHADERC=${SHADERC} INPUT=${shader} OUTPUT=${output_shader} INCLUDE1=${INCLUDE1} INCLUDE2=${INCLUDE2} SHADERTYPE=vertex PROFILE=150
			OUTPUT_QUIET
		)
	endforeach()
	
endforeach()
