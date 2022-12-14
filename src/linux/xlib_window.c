#define VK_USE_PLATFORM_XLIB_KHR
#include "xlib_window.h"
#include "X11/Xatom.h"
#include "X11/Xutil.h"

typedef struct {
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
	long          inputMode;
	unsigned long status;
} motif_hints_t;

int8_t xlib_init_context(xlib_context_t* context);
void xlib_cleanup_context(xlib_context_t* context);

struct platform_window_t {
	Window   handle;
	uint32_t active_flags;
	void*    user_data;
	int8_t   mapped;
	int8_t   should_close;
};

static inline Atom atom_supported(Atom a, Atom* supported_atoms, uint32_t supported_atom_count) {
	for(int i = 0; i < supported_atom_count; i++) {
		if(supported_atoms[i] == a) return a;
	}
	return None;
}

int8_t xlib_init_context(xlib_context_t* context) {
	context->dpy = XOpenDisplay(NULL);
	if(context->dpy == NULL) return 0;
	context->wm_protocols = XInternAtom(context->dpy, "WN_PROTOCOLS", 0);
	context->wm_delete_window = XInternAtom(context->dpy, "WM_DELETE_WINDOW", 0);
	context->motif_wm_hints = XInternAtom(context->dpy, "_MOTIF_WM_HINTS", 0);
	context->net_wm_name = XInternAtom(context->dpy, "NET_WM_NAME", 0);
	context->net_wm_icon_name = XInternAtom(context->dpy, "_NET_WM_ICON_NAME", 0);
	context->utf8_string = XInternAtom(context->dpy, "UTF8_STRING", 0);
	context->net_supported = XInternAtom(context->dpy, "_NET_SUPPORTED", 0);
	context->net_wm_window_type = XInternAtom(context->dpy, "_NET_WM_WINDOW_TYPE", 0);
	context->net_wm_window_type_splash = XInternAtom(context->dpy, "_NET_WM_WINDOW_TYPE_SPLASH", 0);
	context->net_wm_window_type_dialog = XInternAtom(context->dpy, "_NET_WM_WINDOW_TYPE_DIALOG", 0);
	context->net_wm_window_type_menu = XInternAtom(context->dpy, "_NET_WM_WINDOW_TYPE_DIALOG", 0);
	context->net_wm_allowed_actions = XInternAtom(context->dpy, "_NET_WM_ALLOWED_ACTIONS", 0);
	context->net_wm_action_resize = XInternAtom(context->dpy, "_NET_WM_ACTION_RESIZE", 0);


	Window root_window = XRootWindow(context->dpy, XDefaultScreen(context->dpy));
	Atom type;
	uint64_t bytes_after = 0;
	int format;
	XGetWindowProperty(context->dpy, root_window, context->net_supported, 0, 32, 0, XA_ATOM, &type, &format,
	                   &context->supported_atom_count, &bytes_after, (uint8_t**)&context->supported_atoms);

	context->net_wm_window_type = atom_supported(context->net_wm_window_type,
	                                             context->supported_atoms, context->supported_atom_count);
	context->net_wm_window_type_splash = atom_supported(context->net_wm_window_type_splash,
	                                                    context->supported_atoms, context->supported_atom_count);
	context->net_wm_window_type_dialog = atom_supported(context->net_wm_window_type_dialog,
	                                                    context->supported_atoms, context->supported_atom_count);
	context->net_wm_window_type_menu = atom_supported(context->net_wm_window_type_menu,
	                                                  context->supported_atoms, context->supported_atom_count);
	context->net_wm_allowed_actions = atom_supported(context->net_wm_allowed_actions,
	                                                 context->supported_atoms, context->supported_atom_count);

	return 1;
}
void xlib_cleanup_context(xlib_context_t* context) {
	XCloseDisplay(context->dpy);
	context->dpy = NULL;
}

platform_window_t* xlib_create_window(const platform_window_create_info_t create_info, platform_allocation_callbacks_t* allocator) {
	int scr = DefaultScreen(linux_platform_context.xlib.dpy);
	int depth = DefaultDepth(linux_platform_context.xlib.dpy, scr);
	Visual* visual = DefaultVisual(linux_platform_context.xlib.dpy, scr);

	uint64_t event_mask = StructureNotifyMask | SubstructureNotifyMask |
	                      SubstructureRedirectMask | ResizeRedirectMask |
	                      ExposureMask | PropertyChangeMask;
	uint64_t attributes_mask = CWBackPixel | CWEventMask;
	XSetWindowAttributes attributes = {0};
	attributes.background_pixel = BlackPixel(linux_platform_context.xlib.dpy, scr);
	attributes.event_mask = event_mask;

	Window parent_handle = create_info.parent != NULL ? create_info.parent->handle : RootWindow(linux_platform_context.xlib.dpy, scr);
	Window handle = XCreateWindow(linux_platform_context.xlib.dpy, parent_handle, create_info.x, create_info.y,
	                              create_info.width, create_info.height, 0, depth, InputOutput,
	                              visual, attributes_mask, &attributes);

	if(handle == BadWindow || handle == BadValue) return NULL;

	Atom protocols[1] = { linux_platform_context.xlib.wm_delete_window };

	XSetWMProtocols(linux_platform_context.xlib.dpy, handle, protocols, 1);

	if(create_info.flags & PLATFORM_WF_NO_BORDER && create_info.parent == NULL) {
		if(linux_platform_context.xlib.motif_wm_hints != None) {
			motif_hints_t h = {0};
			h.flags = 2;
			XChangeProperty(linux_platform_context.xlib.dpy, handle, linux_platform_context.xlib.motif_wm_hints,
			                linux_platform_context.xlib.motif_wm_hints, 32, PropModeReplace, (uint8_t*)&h, 5);
		}
		/* we dont want to force a floating window if the window manager is tiling
		// TODO: only change these properties if window manager is not tiling, if possible
		else if(context.xlib.net_wm_window_type_splash != None) {
			XChangeProperty(context.xlib.dpy, handle, context.xlib.net_wm_window_type, XA_ATOM, 32,
			                PropModeReplace, (const uint8_t*)&context.xlib.net_wm_window_type_splash, 1);
		}
		else {
			// last resort
			attributes.override_redirect = 1;
			XChangeWindowAttributes(context.xlib.dpy, handle, attributes_mask, &attributes);
		}
		*/
	}

	if(create_info.flags & PLATFORM_WF_DIALOG) {
		if(linux_platform_context.xlib.net_wm_window_type_dialog != None) {
			XChangeProperty(linux_platform_context.xlib.dpy, handle, linux_platform_context.xlib.net_wm_window_type, XA_ATOM, 32,
			                PropModeReplace, (const uint8_t*)&linux_platform_context.xlib.net_wm_window_type_dialog, 1);
		}
	}
	if(create_info.flags & PLATFORM_WF_DIALOG && create_info.parent != NULL) {
		XSetTransientForHint(linux_platform_context.xlib.dpy, handle, create_info.parent->handle);
	}

	if((create_info.flags & PLATFORM_WF_SPLASH) != 0) {
		if(linux_platform_context.xlib.net_wm_window_type_splash != None) {
			XChangeProperty(linux_platform_context.xlib.dpy, handle, linux_platform_context.xlib.net_wm_window_type, XA_ATOM, 32,
							PropModeReplace, (const uint8_t*)&linux_platform_context.xlib.net_wm_window_type_splash, 1);
		}
		else if(linux_platform_context.xlib.net_wm_window_type_menu != None) {
			XChangeProperty(linux_platform_context.xlib.dpy, handle, linux_platform_context.xlib.net_wm_window_type, XA_ATOM, 32,
			                PropModeReplace, (const uint8_t*)&linux_platform_context.xlib.net_wm_window_type_menu, 1);
		}
		else if(linux_platform_context.xlib.net_wm_window_type_dialog != None) {
			XChangeProperty(linux_platform_context.xlib.dpy, handle, linux_platform_context.xlib.net_wm_window_type, XA_ATOM, 32,
			                PropModeReplace, (const uint8_t*)&linux_platform_context.xlib.net_wm_window_type_dialog, 1);
			if((create_info.flags & PLATFORM_WF_NO_BORDER) == 0) {
				motif_hints_t h = {0};
				h.flags = 2;
				XChangeProperty(linux_platform_context.xlib.dpy, handle, linux_platform_context.xlib.motif_wm_hints,
				                linux_platform_context.xlib.motif_wm_hints, 32, PropModeReplace, (uint8_t*)&h, 5);
			}
		}
		else {
			attributes.override_redirect = 1;
			XChangeWindowAttributes(linux_platform_context.xlib.dpy, handle, attributes_mask, &attributes);
		}
	}

	XSizeHints size_hints;
	size_hints.flags = PPosition;
	size_hints.x = create_info.x;
	size_hints.y = create_info.y;
	if(create_info.flags & PLATFORM_WF_RESIZABLE && linux_platform_context.xlib.net_wm_allowed_actions != None) {
		Atom allowed_actions[1] = { linux_platform_context.xlib.net_wm_action_resize };
		XChangeProperty(linux_platform_context.xlib.dpy, handle, linux_platform_context.xlib.net_wm_allowed_actions,
						XA_ATOM, 32, PropModeAppend, (const uint8_t*)allowed_actions, 1);
	}
	XSetWMNormalHints(linux_platform_context.xlib.dpy, handle, &size_hints);

	platform_window_t* window = platform_allocator_alloc(sizeof(platform_window_t), 4, allocator);
	window->handle = handle;
	window->active_flags = create_info.flags;
	window->user_data = NULL;
	window->mapped = 0;
	window->should_close = 0;
	xlib_set_window_name(window, create_info.name);

	if((create_info.flags & PLATFORM_WF_UNMAPPED) == 0) xlib_map_window(window);
	XSaveContext(linux_platform_context.xlib.dpy, handle, 0, (const char*)window);
	return window;
}
void xlib_destroy_window(platform_window_t* window, platform_allocation_callbacks_t* allocator) {
	XDeleteContext(linux_platform_context.xlib.dpy, window->handle, 0);
	XDestroyWindow(linux_platform_context.xlib.dpy, window->handle);
	platform_allocator_free(window, allocator);
	XFlush(linux_platform_context.xlib.dpy);
}
void xlib_get_window_position(const platform_window_t* window, int32_t* x, int32_t* y) {
	XWindowAttributes attributes;
	XGetWindowAttributes(linux_platform_context.xlib.dpy, window->handle, &attributes);
	if(x) *x = attributes.x;
	if(x) *y = attributes.y;
}
void xlib_get_window_size(const platform_window_t* window, uint32_t* width, uint32_t* height) {
	XWindowAttributes attributes;
	XGetWindowAttributes(linux_platform_context.xlib.dpy, window->handle, &attributes);
	if(width) *width = attributes.width;
	if(height) *height = attributes.height;
}
void xlib_set_window_position(platform_window_t* window, const int32_t x, const int32_t y) {
	XMoveWindow(linux_platform_context.xlib.dpy, window->handle, x, y);
}
void xlib_set_window_size(platform_window_t* window, const uint32_t width, const uint32_t height) {
	XSetWindowAttributes new_attributes;
	new_attributes.override_redirect = 1;
	XChangeWindowAttributes(linux_platform_context.xlib.dpy, window->handle, CWOverrideRedirect, &new_attributes);
	XResizeWindow(linux_platform_context.xlib.dpy, window->handle, width, height);
	new_attributes.override_redirect = 0;
	XChangeWindowAttributes(linux_platform_context.xlib.dpy, window->handle, CWOverrideRedirect, &new_attributes);
}
void xlib_get_window_name(const platform_window_t* window, char* name, uint32_t max_len) {
	
}
void xlib_set_window_name(platform_window_t* window, const char* name) {
	if(name == NULL) {
		XDeleteProperty(linux_platform_context.xlib.dpy, window->handle, linux_platform_context.xlib.net_wm_name);
		XDeleteProperty(linux_platform_context.xlib.dpy, window->handle, XA_WM_NAME);
	}
	else {
		uint32_t len = 0;
		while(name[len] != '\0') len++;
		XChangeProperty(linux_platform_context.xlib.dpy, window->handle, linux_platform_context.xlib.net_wm_name,
		                linux_platform_context.xlib.utf8_string, 8, PropModeReplace, (const uint8_t*)name, len);
		XChangeProperty(linux_platform_context.xlib.dpy, window->handle, XA_WM_NAME,
		                XA_STRING, 8, PropModeReplace, (const uint8_t*)name, len);
		XChangeProperty(linux_platform_context.xlib.dpy, window->handle, XA_WM_ICON_NAME,
		                XA_STRING, 8, PropModeReplace, (const uint8_t*)name, len);
		XChangeProperty(linux_platform_context.xlib.dpy, window->handle, linux_platform_context.xlib.net_wm_icon_name,
		                linux_platform_context.xlib.utf8_string, 8, PropModeReplace, (const uint8_t*)name, len);
	}
}

void xlib_map_window(platform_window_t* window) {
	XMapRaised(linux_platform_context.xlib.dpy, window->handle);
}
void xlib_unmap_window(platform_window_t* window) {
	XUnmapWindow(linux_platform_context.xlib.dpy, window->handle);
}

int8_t xlib_window_should_close(const platform_window_t* window) {
	return window->should_close;
}

char** xlib_vulkan_required_extensions(uint32_t* extension_count) {
	*extension_count = 2;
	static char* extensions[] = {
		"VK_KHR_surface",
		"VK_KHR_xlib_surface"
	};
	return extensions;
}

VkSurfaceKHR xlib_vulkan_create_surface(platform_window_t* window, VkInstance instance) {
	VkSurfaceKHR surface;
	VkXlibSurfaceCreateInfoKHR create_info = {
		VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		NULL,
		0,
		linux_platform_context.xlib.dpy,
		window->handle
	};
	VkResult surface_result = vkCreateXlibSurfaceKHR(instance, &create_info, NULL, &surface);
	if(surface_result != VK_SUCCESS) return NULL;
	return surface;
}

void xlib_handle_events(void) {
	uint32_t event_count = XPending(linux_platform_context.xlib.dpy);
	for(uint32_t i = 0; i < event_count; i++) {
		XEvent e;
		XNextEvent(linux_platform_context.xlib.dpy, &e);

		platform_window_t* window;
		int context_result = XFindContext(linux_platform_context.xlib.dpy, e.xany.window, 0, (XPointer*)&window);
		if(context_result != 0) continue;

		XWindowAttributes attributes;
		XGetWindowAttributes(linux_platform_context.xlib.dpy, window->handle, &attributes);

		switch (e.type)
		{
		case ClientMessage:
			if(e.xclient.data.l[0] == linux_platform_context.xlib.wm_delete_window) {
				window->should_close = 1;
			}
			break;
		case PropertyNotify:
			//platform_terminal_print("Property Notify Event.\n", 0, 0, 0);
			break;
		case ResizeRequest:
			xlib_set_window_size(window, e.xresizerequest.width, e.xresizerequest.height);
			break;
		case CirculateNotify:
			platform_terminal_print("Circulate Notify Event.\n", 0, 0, 0);
			break;
		case ConfigureNotify:
			break;
		case DestroyNotify:
			platform_terminal_print("Destroy Notify Event.\n", 0, 0, 0);
			break;
		case GravityNotify:
			platform_terminal_print("Gravity Notify Event.\n", 0, 0, 0);
			break;
		case MapNotify:
			// changing this property at window creation caused the window to
			// be floating in bspwm, placing here prevents that while making the
			// window not resizable, at least on ubuntu
			if((window->active_flags & PLATFORM_WF_RESIZABLE) == 0) {
				uint32_t w, h;
				xlib_get_window_size(window, &w, &h);
				XSizeHints size_hints;
				window->mapped = 1;
				size_hints.flags = PPosition;
				size_hints.min_width = size_hints.max_width = w;
				size_hints.min_height = size_hints.max_height = h;
				size_hints.flags |= PMinSize | PMaxSize;
				XSetWMNormalHints(linux_platform_context.xlib.dpy, window->handle, &size_hints);
			}

			platform_terminal_print("Map Notify Event.\n", 0, 0, 0);
			break;
		case ReparentNotify:
			platform_terminal_print("Reparent Notify Event.\n", 0, 0, 0);
			break;
		case UnmapNotify:
			platform_terminal_print("Unmap Notify Event.\n", 0, 0, 0);
			break;
		case CreateNotify:
			platform_terminal_print("Create Notify Event.\n", 0, 0, 0);
			break;
		case CirculateRequest:
			platform_terminal_print("Circulate Request Event.\n", 0, 0, 0);
			break;
		case ConfigureRequest:
			platform_terminal_print("Configure Request Event.\n", 0, 0, 0);
			break;
		case MapRequest:
			platform_terminal_print("Map Request Event.\n", 0, 0, 0);
			break;

		case MappingNotify: break;
		case SelectionClear: break;
		case SelectionNotify: break;
		case Expose:
			break;
		default:
			platform_terminal_print("Unkown Event.\n", 0, 0, 0);
		}
	}
}