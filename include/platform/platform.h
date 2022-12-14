#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

#define PLATFORM_TEXT_BOLD 1
#define PLATFORM_TEXT_UNDERLINE 2
#define PLATFORM_TEXT_NEGITIVE  4

#define PLATFORM_COLOR_NORMAL         0
#define PLATFORM_COLOR_BLACK          1
#define PLATFORM_COLOR_RED            2
#define PLATFORM_COLOR_GREEN          3
#define PLATFORM_COLOR_YELLOW         4
#define PLATFORM_COLOR_BLUE           5
#define PLATFORM_COLOR_MAGENTA        6
#define PLATFORM_COLOR_CYAN           7
#define PLATFORM_COLOR_WHITE          8
#define PLATFORM_COLOR_BRIGHT_BLACK   9
#define PLATFORM_COLOR_BRIGHT_RED     10
#define PLATFORM_COLOR_BRIGHT_GREEN   11
#define PLATFORM_COLOR_BRIGHT_YELLOW  12
#define PLATFORM_COLOR_BRIGHT_BLUE    13
#define PLATFORM_COLOR_BRIGHT_MAGENTA 14
#define PLATFORM_COLOR_BRIGHT_CYAN    15
#define PLATFORM_COLOR_BRIGHT_WHITE   16

#ifdef PLATFORM_VULKAN
#include <vulkan/vulkan.h>
#endif // PLATFORM_VULKAN

// WF = window flag

// Although the splash flag is very similar to the no border flag,
// a window with the splash flag should be a floating window while
// a window with the no border flag may or may not be floating
// depending on the window manager
// if a borderless window is what you want, dont use the splash flag since,
// the override redirect flag for Xlib_window is set if other options
// do not work

#define PLATFORM_WF_NORMAL    0
#define PLATFORM_WF_NO_BORDER 1  // not always floating
#define PLATFORM_WF_DIALOG    2
#define PLATFORM_WF_SPLASH    4  // should be floating
#define PLATFORM_WF_RESIZABLE 8  // window is resizable using cursor
#define PLATFORM_WF_UNMAPPED  16 // will start unmapped

typedef struct {
	char* app_name;
} platform_settings_t;

typedef struct platform_window_t platform_window_t;

typedef struct {
	char*              name;
	platform_window_t* parent;
	int32_t            x, y;
	uint32_t           width, height;
	uint32_t           flags;
} platform_window_create_info_t;

typedef struct {
	// NOTE: Data alignment is currently not needed, not sure if it will be, added just in case
	void* (*alloc)(void* user_data, uint64_t size, uint64_t alignment);
	void (*free)(void* user_data, void* addr);
	void* (*realloc)(void* user_data, void* addr, uint64_t size);
	void* user_data;
} platform_allocation_callbacks_t;


int8_t platform_init(const platform_settings_t* settings);
void platform_shutdown(void);


platform_window_t* platform_create_window(const platform_window_create_info_t create_info, platform_allocation_callbacks_t* allocator);
void platform_destroy_window(platform_window_t* window, platform_allocation_callbacks_t* allocator);
void platform_get_window_position(const platform_window_t* window, int32_t* x, int32_t* y);
void platform_get_window_size(const platform_window_t* window, uint32_t* width, uint32_t* height);
void platform_set_window_position(platform_window_t* window, const int32_t x, const int32_t y);
void platform_set_window_size(platform_window_t* window, const uint32_t width, const uint32_t height);
void platform_get_window_name(const platform_window_t* window, char* name, uint32_t max_len);
void platform_set_window_name(platform_window_t* window, const char* name);

void platform_map_window(platform_window_t* window);
void platform_unmap_window(platform_window_t* window);

int8_t platform_window_should_close(const platform_window_t* window);

#ifdef PLATFORM_VULKAN
char** platform_vulkan_required_extensions(uint32_t* extension_count);
VkSurfaceKHR platform_vulkan_create_surface(platform_window_t* window, VkInstance instance);
#endif // PLATFORM_VULKAN

void platform_handle_events(void);

void platform_terminal_print(const char* msg, const uint8_t forground, const uint8_t background, const uint8_t flags);
void platform_terminal_print_error(const char* msg, const uint8_t forground, const uint8_t background, const uint8_t flags);

void* platform_map_memory(void* addr_hint, uint64_t size);
int8_t platform_unmap_memory(void* addr, uint64_t size);

uint64_t platform_get_timestamp(void);
void platform_sleep_miliseconds(const uint32_t miliseconds);

#endif // PLATFORM_H