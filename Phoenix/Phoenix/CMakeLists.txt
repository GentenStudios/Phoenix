cmake_minimum_required(VERSION 3.0)

project(Phoenix)

FILE(GLOB src *.cpp)
FILE(GLOB headers *.hpp)

add_executable(${PROJECT_NAME} ${src} ${headers})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../)

target_link_libraries(${PROJECT_NAME} PRIVATE PhoenixVendor PhoenixGlobals PhoenixRenderer PhoenixWindowing PhoenixResourceManager)

# Force C++17 without custom compiler extensions.
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX  _d)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/latest)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/latest)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/latest)

set_target_properties(${PROJECT_NAME} PROPERTIES
	DEBUG_POSTFIX                    ${CMAKE_DEBUG_POSTFIX}
	RUNTIME_OUTPUT_DIRECTORY         ${CMAKE_SOURCE_DIR}/latest
	RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/latest
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/latest
	VS_DEBUGGER_WORKING_DIRECTORY    ${CMAKE_SOURCE_DIR}/latest
)

add_dependencies(${PROJECT_NAME} PhoenixAssets)
