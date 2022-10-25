#ifndef LINUX_PLATFORM_H
#define LINUX_PLATFORM_H

#include "linux_internal.h"
#include "xlib_window.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <vulkan/vulkan.h>

void* platform_allocator_alloc(uint64_t size, uint64_t alignment, platform_allocation_callbacks_t* allocator) {
	if(allocator != NULL) {
		return allocator->alloc(allocator->user_data, size, alignment);
	}
	else {
		return aligned_alloc(alignment, size);
	}
}

void platform_allocator_free(void* addr, platform_allocation_callbacks_t* alloctor) {
	if(alloctor != NULL) {
		alloctor->free(alloctor->user_data, addr);
	}
	else {
		free(addr);
	}
}

platform_context_t* platform_create_context(const platform_context_settings_t* settings, platform_allocation_callbacks_t* allocator) {
	xlib_context_t xlib_contex;
	if(xlib_init_context(&xlib_contex) == 0) return NULL;
	platform_context_t* context = platform_allocator_alloc(sizeof(platform_context_t), 4, allocator);
	context->xlib = xlib_contex;
	context->window_functions = XLIB_WINDOW_FUNCTIONS;
	return context;
}
void platform_destroy_context(platform_context_t* context, platform_allocation_callbacks_t* allocator) {
	platform_allocator_free(context, allocator);
}


platform_window_t* platform_create_window(platform_context_t* context, const platform_window_create_info_t create_info, platform_allocation_callbacks_t* allocator) {
	return context->window_functions.create_window(context, create_info, allocator);
}
void platform_destroy_window(platform_context_t* context, platform_window_t* window, platform_allocation_callbacks_t* allocator) {
	context->window_functions.destroy_window(context, window, allocator);
}
void platform_get_window_position(const platform_context_t* context, const platform_window_t* window, int32_t* x, int32_t* y) {
	context->window_functions.get_window_position(context, window, x, y);
}
void platform_get_window_size(const platform_context_t* context, const platform_window_t* window, uint32_t* width, uint32_t* height) {
	context->window_functions.get_window_size(context, window, width, height);
}
void platform_set_window_position(const platform_context_t* context, platform_window_t* window, const int32_t x, const int32_t y) {
	context->window_functions.set_window_position(context, window, x, y);
}
void platform_set_window_size(const platform_context_t* context, platform_window_t* window, const uint32_t width, const uint32_t height) {
	context->window_functions.set_window_size(context, window, width, height);
}
void get_window_name(const platform_context_t* context, const platform_window_t* window, char* name, uint32_t max_len) {
	return context->window_functions.get_window_name(context, window, name, max_len);
}
void platform_set_window_name(const platform_context_t* context, platform_window_t* window, const char* name) {
	context->window_functions.set_window_name(context, window, name);
}

void platform_map_window(const platform_context_t* context, platform_window_t* window) {
	context->window_functions.map_window(context, window);
}
void platform_unmap_window(const platform_context_t* context, platform_window_t* window) {
	context->window_functions.unmap_window(context, window);
}

int8_t platform_window_should_close(const platform_context_t* context, const platform_window_t* window) {
	return context->window_functions.window_should_close(window);
}

char** platform_vulkan_required_extensions(const platform_context_t* context, uint32_t* extension_count) {
	return context->window_functions.vulkan_required_extensions(context, extension_count);
}

VkSurfaceKHR platform_vulkan_create_surface(platform_context_t* context, platform_window_t* window, VkInstance instance) {
	return context->window_functions.vulkan_create_surface(context, window, instance);
}

void platform_handle_events(const platform_context_t* context) {
	context->window_functions.handle_events(context);
}

// NOTE: add 10 to get background color
static const uint32_t color_table[] = {
	0,
	30, 31, 32, 33, 34, 35, 36, 37,
	90, 91, 92, 93, 94, 95, 96, 97
};


static inline void _terminal_print(const char* msg, const uint8_t forground, const uint8_t background, const uint8_t flags, FILE* stream) {
	uint32_t fg = color_table[forground];
	uint32_t bg = color_table[background] + 10;

	char properties[16] = {0};
	char* prop_str = properties;

	prop_str += sprintf(prop_str, "\033[");
	if(fg != 0) {
		prop_str += sprintf(prop_str, "%d", fg);
	}
	if(bg != 0) {
		if(fg != 0) prop_str += sprintf(prop_str, ";");
		prop_str += sprintf(prop_str, "%d", bg);
	}


	if((flags & PLATFORM_TEXT_BOLD) == PLATFORM_TEXT_BOLD) {
		if(fg != 0 || bg != 0) prop_str += sprintf(prop_str, ";");
		prop_str += sprintf(prop_str, "1");
	}
	if((flags & PLATFORM_TEXT_UNDERLINE) == PLATFORM_TEXT_UNDERLINE) {
		if(fg != 0 || bg != 0) prop_str += sprintf(prop_str, ";");
		prop_str += sprintf(prop_str, "4");
	}

	prop_str += sprintf(prop_str, "m");
	fputs(properties, stream);
	uint32_t index = 0;
	// before you ask, yes, I know this is slow
	while(msg[index] != '\0') {
		if(msg[index] == '\n') {
			// replacing newlines with "\033[0m\n" prevents
			// the background from extending past the text
			fputs("\033[0m\n", stream);
			fputs(properties, stream);
			index++;
		}
		else putc(msg[index++], stream);
	}
	fputs("\033[0m", stream);
	fflush(stream);
}

void platform_terminal_print(const char* msg, const uint8_t forground, const uint8_t background, const uint8_t flags) {
	_terminal_print(msg, forground, background, flags, stdout);
}
void platform_terminal_print_error(const char* msg, const uint8_t forground, const uint8_t background, const uint8_t flags) {
	_terminal_print(msg, forground, background, flags, stderr);
}


uint64_t platform_get_timestamp(void) {
	return 0;
}
void platform_sleep_miliseconds(const uint32_t miliseconds) {
	#if _BSD_SOURCE || (_XOPEN_SOURCE >= 500 || \
	    _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED) && \
	    !(_POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700)

		usleep(miliseconds * 1000);
	#elif _POSIX_C_SOURCE >= 199309L
		struct timespec sleep_time;
		struct timespec remaining;
		sleep_time.tv_sec = miliseconds / 1000;
		sleep_time.tv_nsec = (miliseconds - sleep_time.tv_sec * 1000) * 1000000;
		nanosleep(&sleep_time, &remaining);
	#endif
}

#endif // LINUX_PLATFORM_H