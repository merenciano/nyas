add_library(nyas_extern OBJECT)

target_include_directories(nyas_extern PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(nyas_extern PRIVATE
	_GLFW_X11
)

target_sources(nyas_extern PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/vulkan.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/posix_thread.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/monitor.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/init.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/x11_window.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/posix_time.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/context.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/glx_context.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/x11_init.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/osmesa_context.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/xkb_unicode.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/window.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/input.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/linux_joystick.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/egl_context.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/GLFW/x11_monitor.c
)
