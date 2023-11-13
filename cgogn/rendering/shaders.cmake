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

		add_custom_command( 
			OUTPUT ${output_shader}
			COMMAND ${SHADERC} -f ${shader} -o ${output_shader} -i ${INCLUDE1} -i ${INCLUDE2}  --type fragment -p 150 
			DEPENDS ${shader} ${SHADERC}
		)	 
		set(CUSTOM_TARGET_NAME "${shader_folder_name}_${shader_name_we}")

		add_custom_target(${CUSTOM_TARGET_NAME} ALL DEPENDS ${output_shader})

		add_dependencies(${PROJECT_NAME} ${CUSTOM_TARGET_NAME})

	endforeach()

		foreach(shader ${VSHADERS})
		get_filename_component(shader_name ${shader} NAME)
		get_filename_component(shader_name_we ${shader} NAME_WE)
		set (output_shader ${CMAKE_BINARY_DIR}/stage/bin/shaders/${shader_folder_name}/${shader_name_we}.bin)

		add_custom_command( 
			OUTPUT ${output_shader}
			COMMAND ${SHADERC} -f ${shader} -o ${output_shader} -i ${INCLUDE1} -i ${INCLUDE2}  --type vertex -p 150 
			DEPENDS ${shader} ${SHADERC}
		)	 
		set(CUSTOM_TARGET_NAME "${shader_folder_name}_${shader_name_we}")

		add_custom_target(${CUSTOM_TARGET_NAME} ALL DEPENDS ${output_shader})

		add_dependencies(${PROJECT_NAME} ${CUSTOM_TARGET_NAME})

	endforeach()
	
endforeach()
