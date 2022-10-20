#ifndef LINUX_PLATFORM_H
#define LINUX_PLATFORM_H

#include "linux_internal.h"
#include "xlib_window.h"
#include <stdio.h>

platform_context_t* platform_create_context(const platform_context_settings_t* settings, platform_allocation_callbacks_t* allocator) {
	platform_context_t* context = NULL;
	return context;
}
void platform_destroy_context(platform_context_t* context, platform_allocation_callbacks_t* allocator) {

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
	context->window_functions.platform_get_window_size(context, window, width, height);
}
void platform_set_window_position(const platform_context_t* context, const platform_window_t* window, const int32_t x, const int32_t y) {
	context->window_functions.set_window_position(context, window, x, y);
}
void platform_set_window_size(const platform_context_t* context, const platform_window_t* window, const uint32_t width, const uint32_t height) {
	context->window_functions.set_window_size(context, window, width, height);
}
const char* platform_get_window_name(const platform_context_t* context, const platform_window_t* window) {
	return context->window_functions.get_window_name(context, window);
}
void platform_set_window_name(const platform_context_t* context, platform_window_t* window, const char* name) {
	context->window_functions.set_window_name(context, window, name);
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
	while(msg[index] != '\0') {
		if(msg[index] == '\n') {
			// replace newlines with this so that any background there is will not extend
			// past the text
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
	
}

#endif // LINUX_PLATFORM_H