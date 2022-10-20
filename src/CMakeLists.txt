
add_library(platform STATIC
	"${CMAKE_SOURCE_DIR}/include/platform/platform.h"
	linux/linux_internal.h
	linux/linux_platform.c
	linux/xlib_window.h
)

target_include_directories(platform PUBLIC "${CMAKE_SOURCE_DIR}/include/")
target_include_directories(platform PRIVATE "${CMAKE_SOURCE_DIR}/src/")

target_link_libraries(platform
	X11
)