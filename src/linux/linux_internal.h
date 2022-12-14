#ifndef LINUX_INTERNAL_H
#define LINUX_INTERNAL_H

#include "platform/platform.h"
#include <X11/Xlib.h>
#include <vulkan/vulkan.h>

typedef struct {
	platform_window_t* (*create_window)(const platform_window_create_info_t create_info, platform_allocation_callbacks_t* allocator);
	void (*destroy_window)(platform_window_t* window, platform_allocation_callbacks_t* allocator);
	void (*get_window_position)(const platform_window_t* window, int32_t* x, int32_t* y);
	void (*get_window_size)(const platform_window_t* window, uint32_t* width, uint32_t* height);
	void (*set_window_position)(platform_window_t* window, const int32_t x, const int32_t y);
	void (*set_window_size)(platform_window_t* window, const uint32_t width, const uint32_t height);
	void (*get_window_name)(const platform_window_t* window, char* name, uint32_t max_len);
	void (*set_window_name)(platform_window_t* window, const char* name);
	void (*map_window)(platform_window_t* window);
	void (*unmap_window)(platform_window_t* window);
	int8_t (*window_should_close)(const platform_window_t* window);
	char** (*vulkan_required_extensions)(uint32_t* extension_count);
	VkSurfaceKHR (*vulkan_create_surface)(platform_window_t* window, VkInstance instance);
	void (*handle_events)(void);
} linux_window_functions_t;

typedef struct {
	Display* dpy;

	Atom wm_protocols;
	Atom wm_delete_window;

	Atom motif_wm_hints;
	Atom net_wm_name;
	Atom net_wm_icon_name;
	Atom utf8_string;
	Atom net_supported;
	Atom net_wm_window_type;
	// used to hopefully get floating windows on tiling window managers
	Atom net_wm_window_type_splash;
	Atom net_wm_window_type_dialog;
	Atom net_wm_window_type_menu;

	Atom net_wm_allowed_actions;
	Atom net_wm_action_resize;

	uint64_t supported_atom_count;
	Atom* supported_atoms;
} xlib_context_t;

typedef struct linux_context_t {
	union {
		xlib_context_t xlib;
	};
	linux_window_functions_t window_functions;
} linux_context_t;
extern linux_context_t linux_platform_context;

void* platform_allocator_alloc(uint64_t size, uint64_t alignment, platform_allocation_callbacks_t* allocator);
void platform_allocator_free(void* addr, platform_allocation_callbacks_t* alloctor);

#endif // LINUX_INTERNAL_H