project(imgui LANGUAGES CXX)

file(GLOB_RECURSE sourceFiles imgui/*)

add_library(${PROJECT_NAME} STATIC ${sourceFiles})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
