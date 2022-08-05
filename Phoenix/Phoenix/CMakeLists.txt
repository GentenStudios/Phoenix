cmake_minimum_required(VERSION 3.0)

set(project_name Phoenix)
project(${project_name})

set(HAVE_LIBC TRUE)

FILE(GLOB src *.cpp)
FILE(GLOB headers *.hpp)

# Object Loader Lib
FILE(GLOB objloader_src ../../Vendor/objloader/*.cpp)
FILE(GLOB objloader_headers ../../Vendor/objloader/*.hpp)
list(APPEND src ${objloader_src})
list(APPEND headers ${objloader_headers})


# Load imgui
set(ext_src
    ../../Vendor/lodepng/lodepng.cpp
)
set(ext_headers
    ../../Vendor/lodepng/lodepng.h
)


list(APPEND src ${ext_src})
list(APPEND headers ${ext_headers})

source_group("Source Files\\External" FILES ${ext_src})
source_group("Header Files\\External" FILES ${ext_headers})




include_directories(./)
include_directories(../Renderer)
include_directories(../Windowing)
include_directories(../ResourceManager)
include_directories(../Globals)

include_directories(../../Vendor/lodepng)
include_directories(../../Vendor/imgui-docking)
include_directories(../../Vendor/objloader)


if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX  _d)
endif()

add_executable(${project_name} WIN32 ${src} ${headers})

set_target_properties(${project_name} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(${project_name} pugixml-static)

target_link_libraries(${project_name} SDL2main)
target_link_libraries(${project_name} SDL2-static)

target_link_libraries( ${project_name} Renderer)
target_link_libraries( ${project_name} Windowing)
target_link_libraries( ${project_name} ResourceManager)
target_link_libraries( ${project_name} Globals)

##IF (WIN32)
#### Warnings as errors
##target_compile_options(${project_name} PRIVATE
##  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
##  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
##)
##ENDIF()

if ( MSVC )
    #set_target_properties( ${project_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/latest )
    #set_target_properties( ${project_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/latest )
    #set_target_properties( ${project_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/latest )
    set_target_properties( ${project_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/latest )
    set_target_properties( ${project_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/latest )
    set_target_properties( ${project_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/latest )
    # etc for the other available configuration types (MinSizeRel, RelWithDebInfo)
endif ( MSVC )

set_property(TARGET ${project_name} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/latest")
set_target_properties( ${project_name}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/latest"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/latest"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/latest"
)