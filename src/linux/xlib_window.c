#include "xlib_window.h"
#include "X11/Xatom.h"

int8_t xlib_init_context(xlib_context_t* context);
void xlib_cleanup_context(xlib_context_t* context);

struct platform_window_t {
	Window handle;
};

int8_t xlib_init_context(xlib_context_t* context) {
	context->dpy = XOpenDisplay(NULL);
	if(context->dpy == NULL) return 0;
	context->net_wm_name = XInternAtom(context->dpy, "NET_WM_NAME", 0);
	context->utf8_string = XInternAtom(context->dpy, "UTF8_STRING", 0);
	return 1;
}
void xlib_cleanup_context(xlib_context_t* context) {
	XCloseDisplay(context->dpy);
	context->dpy = NULL;
}

platform_window_t* xlib_create_window(platform_context_t* context, const platform_window_create_info_t create_info, platform_allocation_callbacks_t* allocator) {
	int scr = DefaultScreen(context->xlib.dpy);
	int depth = DefaultDepth(context->xlib.dpy, scr);
	Visual* visual = DefaultVisual(context->xlib.dpy, scr);

	uint64_t attributes_mask = CWBackPixel;
	XSetWindowAttributes attributes;
	attributes.background_pixel = WhitePixel(context->xlib.dpy, scr);

	Window parent_handle = create_info.parent != NULL ? create_info.parent->handle : RootWindow(context->xlib.dpy, scr);
	Window handle = XCreateWindow(context->xlib.dpy, parent_handle, create_info.x, create_info.y,
	                              create_info.width, create_info.height, 0, depth, InputOutput,
	                              visual, attributes_mask, &attributes);

	if(handle == BadWindow || handle == BadValue) return NULL;
	XMapRaised(context->xlib.dpy, handle);
	XFlush(context->xlib.dpy);

	platform_window_t* window = _allocator_alloc(sizeof(platform_window_t), 4, allocator);
	window->handle = handle;

	xlib_set_window_name(context, window, create_info.name);

	return window;
}
void xlib_destroy_window(platform_context_t* context, platform_window_t* window, platform_allocation_callbacks_t* allocator) {
	XDestroyWindow(context->xlib.dpy, window->handle);
	_allocator_free(window, allocator);
}
void xlib_get_window_position(const platform_context_t* context, const platform_window_t* window, int32_t* x, int32_t* y) {
	XWindowAttributes attributes;
	XGetWindowAttributes(context->xlib.dpy, window->handle, &attributes);
	if(x) *x = attributes.x;
	if(x) *y = attributes.y;
}
void xlib_get_window_size(const platform_context_t* context, const platform_window_t* window, uint32_t* width, uint32_t* height) {
	XWindowAttributes attributes;
	XGetWindowAttributes(context->xlib.dpy, window->handle, &attributes);
	if(width) *width = attributes.width;
	if(height) *height = attributes.height;
}
void xlib_set_window_position(const platform_context_t* context, const platform_window_t* window, const int32_t x, const int32_t y) {
	XMoveWindow(context->xlib.dpy, window->handle, x, y);
}
void xlib_set_window_size(const platform_context_t* context, const platform_window_t* window, const uint32_t width, const uint32_t height) {
	XResizeWindow(context->xlib.dpy, window->handle, width, height);
}
const char* xlib_get_window_name(const platform_context_t* context, const platform_window_t* window) {
	return NULL;
}
void xlib_set_window_name(const platform_context_t* context, platform_window_t* window, const char* name) {
	if(name == NULL) {
		XDeleteProperty(context->xlib.dpy, window->handle, context->xlib.net_wm_name);
		XDeleteProperty(context->xlib.dpy, window->handle, XA_WM_NAME);
	}
	else {
		uint32_t len = 0;
		while(name[len] != '\0') len++;
		XChangeProperty(context->xlib.dpy, window->handle, context->xlib.net_wm_name,
						context->xlib.utf8_string, 8, PropModeReplace, (const uint8_t*)name, len);
		XChangeProperty(context->xlib.dpy, window->handle, XA_WM_NAME,
						XA_STRING, 8, PropModeReplace, (const uint8_t*)name, len);
	}
	XFlush(context->xlib.dpy);
}
void xlib_handle_events(const platform_context_t* context) {
	uint32_t event_count = XPending(context->xlib.dpy);
	for(uint32_t i = 0; i < event_count; i++) {
		XEvent e;
		XNextEvent(context->xlib.dpy, &e);
	}
}