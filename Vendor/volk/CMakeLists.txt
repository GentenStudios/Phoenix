cmake_minimum_required(VERSION 2.6)

set(project_name volk)
project(${project_name})

set(HAVE_LIBC TRUE)
set(src
    volk.c
)
 
set(headers
  volk.h
)


add_library(${project_name} STATIC ${src} ${headers})