project(volk)

set(src
    volk.c
)
 
set(headers
  volk.h
)


add_library(${PROJECT_NAME} STATIC volk.h volk.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../vulkan)
