#ifndef LINUX_INTERNAL_H
#define LINUX_INTERNAL_H

#include "platform/platform.h"
#include <X11/Xlib.h>

typedef struct {
	platform_window_t* (*create_window)(platform_context_t* context, const platform_window_create_info_t create_info, platform_allocation_callbacks_t* allocator);
	void (*destroy_window)(platform_context_t* context, platform_window_t* window, platform_allocation_callbacks_t* allocator);
	void (*get_window_position)(const platform_context_t* context, const platform_window_t* window, int32_t* x, int32_t* y);
	void (*get_window_size)(const platform_context_t* context, const platform_window_t* window, uint32_t* width, uint32_t* height);
	void (*set_window_position)(const platform_context_t* context, const platform_window_t* window, const int32_t x, const int32_t y);
	void (*set_window_size)(const platform_context_t* context, const platform_window_t* window, const uint32_t width, const uint32_t height);
	const char* (*get_window_name)(const platform_context_t* context, const platform_window_t* window);
	void (*set_window_name)(const platform_context_t* context, platform_window_t* window, const char* name);
	void (*handle_events)(const platform_context_t* context);
} linux_window_functions_t;

typedef struct {
	Display* dpy;
} xlib_context_t;

struct platform_context_t {
	union {
		xlib_context_t xlib;
	};
	linux_window_functions_t window_functions;
};


#endif // LINUX_INTERNAL_H