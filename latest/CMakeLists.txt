project(PhoenixAssets)

file(GLOB Pipelines ${CMAKE_CURRENT_LIST_DIR}/data/Pipelines/*.*)

set(ShaderDir ${CMAKE_CURRENT_LIST_DIR}/data/Shaders)
file(GLOB_RECURSE Shaders ${ShaderDir}/*.*)
file(GLOB_RECURSE ShadersToCompile ${ShaderDir}/*.vert ${ShaderDir}/*.frag ${ShaderDir}/*.comp)

# GLSLC will compile our GLSL shaders to SPIR-V.
find_program(GLSLC glslc)

# This will compile shader.frag -> frag.spv, shader.vert -> vert.spv, etc...

foreach(ShaderFile ${ShadersToCompile})
	get_filename_component(CurrentOutputDir ${ShaderFile} DIRECTORY)

	# Getting LAST_EXT also gives us the "." at the start, e.g. .vert
	get_filename_component(CurrentShaderType ${ShaderFile} LAST_EXT)
	string(SUBSTRING ${CurrentShaderType} 1 -1 NewShaderType)

	set(CurrentOutputPath ${CurrentOutputDir}/${NewShaderType}.spv)

	add_custom_command(
		OUTPUT ${CurrentOutputPath}
		COMMAND glslc -o ${CurrentOutputPath} ${ShaderFile}
		DEPENDS ${ShaderFile}
		IMPLICIT_DEPENDS CXX ${ShaderFile}
		VERBATIM
	)

	set_source_files_properties(${ShaderFile} PROPERTIES GENERATED TRUE)
	list(APPEND CompiledShaderFiles ${CurrentOutputPath})
endforeach()

add_custom_target(${PROJECT_NAME} ALL DEPENDS ${CompiledShaderFiles} SOURCES ${Pipelines} ${Shaders})
