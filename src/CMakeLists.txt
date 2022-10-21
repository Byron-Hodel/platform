
add_library(platform STATIC
	"${CMAKE_SOURCE_DIR}/include/platform/platform.h"
)

if(WIN32)
	target_sources(platform PRIVATE
		windows/win32_platform.c
	)
	target_link_libraries(platform
		Winmm.lib
		User32.lib
	)
	elseif(UNIX)
	target_sources(platform PRIVATE
		linux/linux_internal.h
		linux/linux_platform.c
		linux/xlib_window.h
		linux/xlib_window.c
	)
	target_link_libraries(platform
		X11
	)
elseif(APPLE)
	# Not Supported Yet
endif()

target_include_directories(platform PUBLIC "${CMAKE_SOURCE_DIR}/include/")
target_include_directories(platform PRIVATE "${CMAKE_SOURCE_DIR}/src/")