cmake_minimum_required(VERSION 3.13)

set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Nyas VERSION 0.1.0)

add_executable(nyas)

set_target_properties(nyas PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

target_compile_definitions(nyas PUBLIC
	_GLFW_X11
	NYAS_ELEM_SIZE_16
	NYAS_GL3
)

target_compile_options(nyas PUBLIC
	-Wall
	-Wextra
	-Wpedantic
)

target_include_directories(nyas PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/extern/include
)

target_sources(nyas PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src/nyas.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(nyas PRIVATE
	GL
	X11
	dl
	pthread
	m
)

add_subdirectory(extern)
