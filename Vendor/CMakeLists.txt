project(PhoenixVendor)

add_subdirectory(imgui-docking)
add_subdirectory(lodepng)
add_subdirectory(pugixml)
add_subdirectory(volk)

#### SDL2 Configuration
# Stop SDL from building a shared library.
set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)

add_subdirectory(SDL2)
#### End SDL2 Configuration

add_library(${PROJECT_NAME} INTERFACE)

find_package(Threads REQUIRED)

# Link the interface library to the dependencies we require.
target_link_libraries(${PROJECT_NAME} INTERFACE ${CMAKE_THREAD_LIBS_INIT} imgui lodepng pugixml-static volk SDL2-static SDL2main)

# Add includes that are not already setup with system includes to disable warnings.
target_include_directories(${PROJECT_NAME} SYSTEM INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/glm
	${CMAKE_CURRENT_LIST_DIR}/imgui-docking
	${CMAKE_CURRENT_LIST_DIR}/lodepng
	${CMAKE_CURRENT_LIST_DIR}/pugixml
	${CMAKE_CURRENT_LIST_DIR}/SDL2/include
	${CMAKE_CURRENT_LIST_DIR}/volk
	${CMAKE_CURRENT_LIST_DIR}/vulkan
)

# Move dependency targets into folders for organisation (only supported by some IDEs).
set_target_properties(imgui lodepng pugixml-static volk SDL2-static SDL2main uninstall PROPERTIES FOLDER ${PROJECT_NAME})
