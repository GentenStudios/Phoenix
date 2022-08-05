cmake_minimum_required(VERSION 3.0)

set(project_name Windowing)
project(${project_name})

set(HAVE_LIBC TRUE)

include_directories(./)

FILE(GLOB src *.cpp)
FILE(GLOB headers *.hpp)

add_library(${project_name} STATIC ${src} ${headers})

target_link_libraries(${project_name} SDL2main)
target_link_libraries(${project_name} SDL2-static)

find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    #target_include_directories(${project_name} PRIVATE Vulkan::Vulkan)
    #target_link_libraries(${project_name} Vulkan::Vulkan)
endif()


##IF (WIN32)
#### Warnings as errors
##target_compile_options(${project_name} PRIVATE
##  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
##  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
##)
##ENDIF()