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
	void (*get_window_name)(const platform_context_t* context, const platform_window_t* window, char* name, uint32_t max_len);
	void (*set_window_name)(const platform_context_t* context, platform_window_t* window, const char* name);
	void (*handle_events)(const platform_context_t* context);
} linux_window_functions_t;

typedef struct {
	Display* dpy;
	Atom net_wm_name;
	Atom utf8_string;
} xlib_context_t;

struct platform_context_t {
	union {
		xlib_context_t xlib;
	};
	linux_window_functions_t window_functions;
};

void* _allocator_alloc(uint64_t size, uint64_t alignment, platform_allocation_callbacks_t* allocator);
void _allocator_free(void* addr, platform_allocation_callbacks_t* alloctor);

#endif // LINUX_INTERNAL_H