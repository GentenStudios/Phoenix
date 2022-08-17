project(PhoenixAssets)

file(GLOB Pipelines ${CMAKE_CURRENT_LIST_DIR}/data/Pipelines/*.*)

set(ShaderDir ${CMAKE_CURRENT_LIST_DIR}/data/Shaders)
file(GLOB_RECURSE Shaders ${ShaderDir}/*.*)
file(GLOB_RECURSE ShadersToCompile ${ShaderDir}/*.vert ${ShaderDir}/*.frag ${ShaderDir}/*.comp)

# Find Vulkan SDK
# From: https://github.com/krOoze/Hello_Triangle/ & their vk_sdk_lite repo.
if (NOT DEFINED {VULKAN_SDK})
	if (NOT DEFINED ENV{VULKAN_SDK})
		message(FATAL_ERROR "VULKAN_SDK not found!")
	endif()

	if (CYGWIN)
		execute_process(COMMAND cygpath "$ENV{VULKAN_SDK}" OUTPUT_VARIABLE VULKAN_SDK)
		string(STRIP ${VULKAN_SDK} VULKAN_SDK)
	else()
		set(VULKAN_SDK "$ENV{VULKAN_SDK}")
	endif()
endif()

message("Vulkan SDK path: " ${VULKAN_SDK})
set(GLSL_COMPILER ${VULKAN_SDK}/Bin/glslc)

# This will compile shader.frag -> frag.spv, shader.vert -> vert.spv, etc...

foreach(ShaderFile ${ShadersToCompile})
	get_filename_component(CurrentOutputDir ${ShaderFile} DIRECTORY)

	# Getting LAST_EXT also gives us the "." at the start, e.g. .vert
	get_filename_component(CurrentShaderType ${ShaderFile} LAST_EXT)
	string(SUBSTRING ${CurrentShaderType} 1 -1 NewShaderType)

	set(CurrentOutputPath ${CurrentOutputDir}/${NewShaderType}.spv)

	add_custom_command(
		OUTPUT ${CurrentOutputPath}
		COMMAND ${GLSL_COMPILER} -o ${CurrentOutputPath} ${ShaderFile}
		DEPENDS ${ShaderFile}
		IMPLICIT_DEPENDS CXX ${ShaderFile}
		VERBATIM
	)

	set_source_files_properties(${ShaderFile} PROPERTIES GENERATED TRUE)
	list(APPEND CompiledShaderFiles ${CurrentOutputPath})
endforeach()

add_custom_target(${PROJECT_NAME} ALL DEPENDS ${CompiledShaderFiles} SOURCES ${Pipelines} ${Shaders})
