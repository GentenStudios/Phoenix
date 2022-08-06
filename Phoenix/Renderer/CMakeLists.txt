project(PhoenixRenderer)

FILE(GLOB src *.cpp)
FILE(GLOB headers *.hpp)

add_library(${PROJECT_NAME} STATIC ${src} ${headers})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../Globals)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../ResourceManager)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../Windowing)

target_link_libraries(${PROJECT_NAME} PRIVATE PhoenixVendor PhoenixWindowing PhoenixResourceManager)
