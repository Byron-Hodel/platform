#ifndef XLIB_WINDOW_H
#define XLIB_WINDOW_H

#include "linux_internal.h"
#include <X11/Xlib.h>

#define XLIB_WINDOW_FUNCTIONS (linux_window_functions_t) { \
	.create_window = xlib_create_window, \
	.destroy_window = xlib_destroy_window, \
	.get_window_position = xlib_get_window_position, \
	.get_window_size = xlib_get_window_size, \
	.set_window_position = xlib_set_window_position, \
	.set_window_size = xlib_set_window_size, \
	.get_window_name = xlib_get_window_name, \
	.set_window_name = xlib_set_window_name, \
	.map_window = xlib_map_window, \
	.unmap_window = xlib_unmap_window, \
	.window_should_close = xlib_window_should_close, \
	.vulkan_required_extensions = xlib_vulkan_required_extensions, \
	.vulkan_create_surface = xlib_vulkan_create_surface, \
	.handle_events = xlib_handle_events \
}

int8_t xlib_init_context(xlib_context_t* context);
void xlib_cleanup_context(xlib_context_t* context);

platform_window_t* xlib_create_window(const platform_window_create_info_t create_info, platform_allocation_callbacks_t* allocator);
void xlib_destroy_window(platform_window_t* window, platform_allocation_callbacks_t* allocator);
void xlib_get_window_position(const platform_window_t* window, int32_t* x, int32_t* y);
void xlib_get_window_size(const platform_window_t* window, uint32_t* width, uint32_t* height);
void xlib_set_window_position(platform_window_t* window, const int32_t x, const int32_t y);
void xlib_set_window_size(platform_window_t* window, const uint32_t width, const uint32_t height);
void xlib_get_window_name(const platform_window_t* window, char* name, uint32_t max_len);
void xlib_set_window_name(platform_window_t* window, const char* name);
void xlib_map_window(platform_window_t* window);
void xlib_unmap_window(platform_window_t* window);
int8_t xlib_window_should_close(const platform_window_t* window);
char** xlib_vulkan_required_extensions(uint32_t* extension_count);
VkSurfaceKHR xlib_vulkan_create_surface(platform_window_t* window, VkInstance instance);

void xlib_handle_events(void);

#endif // XLIB_WINDOW_H