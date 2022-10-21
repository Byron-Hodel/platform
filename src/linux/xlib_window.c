#include "xlib_window.h"

int8_t xlib_init_context(xlib_context_t* context);
void xlib_cleanup_context(xlib_context_t* context);

platform_window_t* xlib_create_window(platform_context_t* context, const platform_window_create_info_t create_info, platform_allocation_callbacks_t* allocator) {
	return NULL;
}
void xlib_destroy_window(platform_context_t* context, platform_window_t* window, platform_allocation_callbacks_t* allocator) {
 
}
void xlib_get_window_position(const platform_context_t* context, const platform_window_t* window, int32_t* x, int32_t* y) {

}
void xlib_get_window_size(const platform_context_t* context, const platform_window_t* window, uint32_t* width, uint32_t* height) {

}
void xlib_set_window_position(const platform_context_t* context, const platform_window_t* window, const int32_t x, const int32_t y) {

}
void xlib_set_window_size(const platform_context_t* context, const platform_window_t* window, const uint32_t width, const uint32_t height) {

}
const char* xlib_get_window_name(const platform_context_t* context, const platform_window_t* window) {
	return NULL;
}
void xlib_set_window_name(const platform_context_t* context, platform_window_t* window, const char* name) {

}
void xlib_handle_events(const platform_context_t* context) {

}