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
	Window handle;
	uint32_t active_flags;
	void* user_data;
	int32_t x, y;
	int8_t is_mapped;
};

static Atom atom_supported(Atom a, Atom* supported_atoms, uint32_t supported_atom_count) {
	for(int i = 0; i < supported_atom_count; i++) {
		if(supported_atoms[i] == a) return a;
	}
	return None;
}

int8_t xlib_init_context(xlib_context_t* context) {
	context->dpy = XOpenDisplay(NULL);
	if(context->dpy == NULL) return 0;
	context->motif_wm_hints = XInternAtom(context->dpy, "_MOTIF_WM_HINTS", 1);
	context->net_wm_name = XInternAtom(context->dpy, "NET_WM_NAME", 0);
	context->utf8_string = XInternAtom(context->dpy, "UTF8_STRING", 0);
	context->net_supported = XInternAtom(context->dpy, "_NET_SUPPORTED", 0);
	context->net_wm_window_type = XInternAtom(context->dpy, "_NET_WM_WINDOW_TYPE", 0);
	context->net_wm_window_type_splash = XInternAtom(context->dpy, "_NET_WM_WINDOW_TYPE_SPLASH", 0);
	context->net_wm_window_type_dialog = XInternAtom(context->dpy, "_NET_WM_WINDOW_TYPE_DIALOG", 0);


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

	uint64_t event_mask = StructureNotifyMask | SubstructureNotifyMask |
	                      SubstructureRedirectMask | ResizeRedirectMask |
	                      ExposureMask | PropertyChangeMask;
	uint64_t attributes_mask = CWBackPixel | CWOverrideRedirect | CWEventMask;
	XSetWindowAttributes attributes = {0};
	attributes.background_pixel = BlackPixel(context->xlib.dpy, scr);
	attributes.override_redirect = 0;
	attributes.event_mask = event_mask;

	Window parent_handle = create_info.parent != NULL ? create_info.parent->handle : RootWindow(context->xlib.dpy, scr);
	Window handle = XCreateWindow(context->xlib.dpy, parent_handle, create_info.x, create_info.y,
	                              create_info.width, create_info.height, 0, depth, InputOutput,
	                              visual, attributes_mask, &attributes);

	if(handle == BadWindow || handle == BadValue) return NULL;

	if(create_info.flags & PLATFORM_WF_NO_BORDER && create_info.parent == NULL) {
		if(context->xlib.motif_wm_hints != None) {
			motif_hints_t h = {0};
			h.flags = 2;
			XChangeProperty(context->xlib.dpy, handle, context->xlib.motif_wm_hints,
			                context->xlib.motif_wm_hints, 32, PropModeReplace, (uint8_t*)&h, 5);
		}
		else if(context->xlib.net_wm_window_type_splash != None) {
			XChangeProperty(context->xlib.dpy, handle, context->xlib.net_wm_window_type, XA_ATOM, 32,
			                PropModeReplace, (const uint8_t*)&context->xlib.net_wm_window_type_splash, 1);
		}
		else {
			// last resort
			attributes.override_redirect = 1;
			XChangeWindowAttributes(context->xlib.dpy, handle, attributes_mask, &attributes);
		}
	}

	if(create_info.flags & PLATFORM_WF_POPUP && create_info.parent != NULL) {
		XSetTransientForHint(context->xlib.dpy, handle, create_info.parent->handle);
	}


	platform_window_t* window = platform_allocator_alloc(sizeof(platform_window_t), 4, allocator);
	window->handle = handle;
	window->active_flags = create_info.flags;
	window->user_data = NULL;
	window->x = create_info.x;
	window->y = create_info.y;
	window->is_mapped = (create_info.flags & PLATFORM_WF_UNMAPPED) == 0;
	xlib_set_window_name(context, window, create_info.name);

	if((create_info.flags & PLATFORM_WF_UNMAPPED) == 0) xlib_map_window(context, window);
	XSaveContext(context->xlib.dpy, handle, 0, (const char*)window);
	return window;
}
void xlib_destroy_window(platform_context_t* context, platform_window_t* window, platform_allocation_callbacks_t* allocator) {
	XDestroyWindow(context->xlib.dpy, window->handle);
	platform_allocator_free(window, allocator);
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
void xlib_set_window_position(const platform_context_t* context, platform_window_t* window, const int32_t x, const int32_t y) {
	window->x = x;
	window->y = y;
	XMoveWindow(context->xlib.dpy, window->handle, x, y);
}
void xlib_set_window_size(const platform_context_t* context, platform_window_t* window, const uint32_t width, const uint32_t height) {
	XSetWindowAttributes new_attributes;
	new_attributes.override_redirect = 1;
	XChangeWindowAttributes(context->xlib.dpy, window->handle, CWOverrideRedirect, &new_attributes);
	XResizeWindow(context->xlib.dpy, window->handle, width, height);
	new_attributes.override_redirect = 0;
	XChangeWindowAttributes(context->xlib.dpy, window->handle, CWOverrideRedirect, &new_attributes);
}
void xlib_get_window_name(const platform_context_t* context, const platform_window_t* window, char* name, uint32_t max_len) {
	
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
}

void xlib_map_window(const platform_context_t* context, platform_window_t* window) {
	XMapRaised(context->xlib.dpy, window->handle);
}
void xlib_unmap_window(const platform_context_t* context, platform_window_t* window) {
	
}

void xlib_handle_events(const platform_context_t* context) {
	uint32_t event_count = XPending(context->xlib.dpy);
	for(uint32_t i = 0; i < event_count; i++) {
		XEvent e;
		XNextEvent(context->xlib.dpy, &e);

		platform_window_t* window;
		int context_result = XFindContext(context->xlib.dpy, e.xany.window, 0, (XPointer*)&window);
		if(context_result != 0) continue;

		XWindowAttributes attributes;
		XGetWindowAttributes(context->xlib.dpy, window->handle, &attributes);

		switch (e.type)
		{
		case PropertyNotify:
			//platform_terminal_print("Property Notify Event.\n", 0, 0, 0);
			break;
		case ResizeRequest:
			// if the window manager attempts to resize an unmapped floating window, it is probally a tiling window manager
			if(window->is_mapped == 0) {
				if(context->xlib.net_wm_window_type_dialog != None) {
					XChangeProperty(context->xlib.dpy, window->handle, context->xlib.net_wm_window_type, XA_ATOM, 32,
									PropModeReplace, (const uint8_t*)&context->xlib.net_wm_window_type_dialog, 1);
				}
				else if(context->xlib.net_wm_window_type_splash != None) {
					XChangeProperty(context->xlib.dpy, window->handle, context->xlib.net_wm_window_type, XA_ATOM, 32,
									PropModeReplace, (const uint8_t*)&context->xlib.net_wm_window_type_splash, 1);
				}
				else {

				}
			}
			platform_terminal_print("Resize Request Event.\n", 0, 0, 0);
			xlib_set_window_size(context, window, e.xresizerequest.width, e.xresizerequest.height);
			break;
		case CirculateNotify:
			platform_terminal_print("Circulate Notify Event.\n", 0, 0, 0);
			break;
		case ConfigureNotify:
			window->x = e.xconfigure.x;
			window->y = e.xconfigure.y;
			break;
		case DestroyNotify:
			platform_terminal_print("Destroy Notify Event.\n", 0, 0, 0);
			break;
		case GravityNotify:
			platform_terminal_print("Gravity Notify Event.\n", 0, 0, 0);
			break;
		case MapNotify:
			platform_terminal_print("Map Notify Event.\n", 0, 0, 0);
			window->is_mapped = 1;
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

		case ClientMessage: break;
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