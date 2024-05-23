cmake_minimum_required(VERSION 3.13)

set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Nyas VERSION 0.1.0)

add_executable(nyas)

set_target_properties(nyas PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

target_compile_definitions(nyas PUBLIC
	_GLFW_X11
	NYAS_GL3
	MATHC_USE_UNIONS
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
	${CMAKE_CURRENT_SOURCE_DIR}/src/nyas.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/nyas_render.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/nyas.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/nyas_render_gl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/nyas_texture.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/nyas_shader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/nyas_mesh.cpp
)

target_link_libraries(nyas PRIVATE
	nyas_extern
	GL
	X11
	dl
	pthread
	m
)

add_subdirectory(extern)
