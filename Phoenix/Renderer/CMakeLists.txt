cmake_minimum_required(VERSION 3.0)

set(project_name Renderer)
project(${project_name})

set(HAVE_LIBC TRUE)

FILE(GLOB src *.cpp)
FILE(GLOB headers *.hpp)

include_directories(./)
include_directories(../Globals)
include_directories(../Windowing)

add_library(${project_name} STATIC ${src} ${headers})

target_link_libraries(${project_name} volk)
target_link_libraries( ${project_name} Globals)
target_link_libraries( ${project_name} Windowing)

target_link_libraries(${project_name} SDL2main)
target_link_libraries(${project_name} SDL2-static)

find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    #target_include_directories(${project_name} PRIVATE Vulkan::Vulkan)
    #target_link_libraries(${project_name} Vulkan::Vulkan)
endif()


message(STATUS "Vulkan FOUND = ${Vulkan_FOUND}")
message(STATUS "Vulkan Lib Dir = ${Vulkan_LIBRARY_DIR}")
message(STATUS "Vulkan Lib = ${Vulkan_LIBRARY}")
message(STATUS "Vulkan Headers Include = ${VulkanHeaders_INCLUDE_DIR}")
message(STATUS "Vulkan Headers Registry = ${VulkanRegistry_DIR}")

#IF (WIN32)
### Warnings as errors
#target_compile_options(${project_name} PRIVATE
#  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
#  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
#)
#ENDIF()