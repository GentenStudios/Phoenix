cmake_minimum_required(VERSION 3.0)

set(project_name Globals)
project(${project_name})

set(HAVE_LIBC TRUE)

FILE(GLOB src *.cpp)
FILE(GLOB headers *.hpp)



add_library(${project_name} STATIC ${src} ${headers})
