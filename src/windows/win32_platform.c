#include "platform/platform.h"

#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOATOM
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#define STRICT

#include <Windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <timeapi.h>
#include <malloc.h>

#define DEFAULT_CLASS_NAME "WIN32_PLATFORM_CLASS"

typedef struct win32_context_t {
	HINSTANCE instance;
	char* class_name;
} win32_context_t;

static win32_context_t context;

struct platform_window_t {
	HWND handle;
	int8_t should_close;
};

static inline void* platform_allocator_alloc(uint64_t size, uint64_t alignment, platform_allocation_callbacks_t* allocator) {
	if(allocator != NULL) {
		return allocator->alloc(allocator->user_data, size, alignment);
	}
	else {
		return _aligned_malloc(size, alignment);
	}
}

static inline void platform_allocator_free(void* addr, platform_allocation_callbacks_t* alloctor) {
	if(alloctor != NULL) {
		alloctor->free(alloctor->user_data, addr);
	}
	else {
		_aligned_free(addr);
	}
}

LRESULT __stdcall window_proc_setup(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);
LRESULT __stdcall window_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);

int8_t platform_init(const platform_settings_t* settings) {
	HINSTANCE instance = GetModuleHandleA(NULL);

	// check if gui applications can be created
	// https://learn.microsoft.com/en-us/windows/win32/winstation/window-stations
	HWINSTA station = GetProcessWindowStation();
	if(station == NULL) return 0;
	const char valid_station_name[8] = "WinSta0";
	char station_name[32];
	DWORD len_needed;
	GetUserObjectInformationA(station, UOI_NAME, (void*)station_name, 32, &len_needed);
	for(int i = 0; i < 8; i++) if(station_name[i] != valid_station_name[i]) return 0;

	WNDCLASSEXA class;
	class.cbSize = sizeof(WNDCLASSEXA);
	class.style = CS_OWNDC;
	class.lpfnWndProc = window_proc_setup;
	class.cbClsExtra = 0;
	class.cbWndExtra = 0;
	class.hInstance = instance;
	class.hIcon = NULL;
	class.hCursor = NULL;
	class.hbrBackground = NULL;
	class.lpszMenuName = NULL;
	class.lpszClassName = DEFAULT_CLASS_NAME;
	class.hIconSm = NULL;

	ATOM class_result = RegisterClassExA(&class);
	if(class_result == 0) return 0;

	context.instance = instance;
	context.class_name = DEFAULT_CLASS_NAME;
	return 1;
}

void platform_shutdown(void) {
	UnregisterClassA(context.class_name, context.instance);
}


platform_window_t* platform_create_window(const platform_window_create_info_t create_info, platform_allocation_callbacks_t* allocator) {
	DWORD window_style = WS_BORDER;
	if(create_info.flags == PLATFORM_WF_NORMAL) {
		window_style = WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_BORDER | WS_SIZEBOX;
	}
	else {
		if((create_info.flags & PLATFORM_WF_NO_BORDER) == 0 && (create_info.flags & PLATFORM_WF_SPLASH) == 0) {
			window_style |= WS_SYSMENU;
			if((create_info.flags & PLATFORM_WF_DIALOG) == 0) {
				window_style |= WS_MINIMIZEBOX;
				if(create_info.flags & PLATFORM_WF_RESIZABLE) window_style |= WS_MAXIMIZEBOX;
			}
		}
		else {
			window_style |= WS_POPUP; // this prevents the window title from showing on the window
		}
		if(create_info.flags & PLATFORM_WF_RESIZABLE) {
			window_style |= WS_SIZEBOX;
		}
	}

	RECT wr;
	wr.left = create_info.x;
	wr.top = create_info.y;
	wr.right = create_info.x + create_info.width;
	wr.bottom = create_info.y + create_info.height;

	AdjustWindowRect(&wr, window_style, FALSE);

	HWND parent = create_info.parent != NULL ? create_info.parent->handle : NULL;

	platform_window_t* window = platform_allocator_alloc(sizeof(platform_window_t), 4, allocator);
	HWND handle = CreateWindowA(context.class_name, create_info.name, window_style,
	                            wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top,
	                            parent, NULL, context.instance, (LPVOID)window);
	if(handle == NULL) {
		platform_allocator_free(window, allocator);
		return NULL;
	}

	if((create_info.flags & PLATFORM_WF_UNMAPPED) == 0) ShowWindow(handle, SW_NORMAL);

	window->handle = handle;
	window->should_close = 0;
	return window;
}
void platform_destroy_window( platform_window_t* window, platform_allocation_callbacks_t* allocator) {
	DestroyWindow(window->handle);
	platform_allocator_free(window, allocator);
}
void platform_get_window_position(const platform_window_t* window, int32_t* x, int32_t* y) {
	RECT client_rect;
	GetClientRect(window->handle, &client_rect);
	if(x) *x = client_rect.left;
	if(y) *y = client_rect.top;
}
void platform_get_window_size(const platform_window_t* window, uint32_t* width, uint32_t* height) {
	RECT cr;
	GetClientRect(window->handle, &cr);
	if(width) *width = cr.right - cr.left;
	if(width) *width = cr.bottom - cr.top;
}
void platform_set_window_position(platform_window_t* window, const int32_t x, const int32_t y) {
	RECT wr;
	GetClientRect(window->handle, &wr);
	uint32_t width = wr.right - wr.left;
	uint32_t height = wr.bottom - wr.top;
	wr.left = x;
	wr.top = y;
	wr.right = x + width;
	wr.bottom = y + height;
	LONG window_style = GetWindowLongA(window->handle, GWL_STYLE);
	MoveWindow(window->handle, wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top, FALSE);
}
void platform_set_window_size(platform_window_t* window, const uint32_t width, const uint32_t height) {
	RECT wr;
	GetClientRect(window->handle, &wr);
	wr.right = wr.left + width;
	wr.bottom = wr.top + height;
	LONG window_style = GetWindowLongA(window->handle, GWL_STYLE);
	MoveWindow(window->handle, wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top, FALSE);
}
void platform_get_window_name(const platform_window_t* window, char* name, uint32_t max_len) {
	GetWindowTextA(window->handle, name, max_len);
}
void platform_set_window_name(platform_window_t* window, const char* name) {
	SetWindowTextA(window->handle, name);
}

void platform_map_window(platform_window_t* window) {
	ShowWindow(window->handle, SW_NORMAL);
}

void platform_unmap_window(platform_window_t* window) {
	ShowWindow(window->handle, SW_HIDE);
}

int8_t platform_window_should_close(const platform_window_t* window) {
	return window->should_close;
}

char** platform_vulkan_required_extensions(uint32_t* extension_count) {
	*extension_count = 2;
	static char* extensions[] = {
		"VK_KHR_surface",
		"VK_KHR_win32_surface"
	};
	return extensions;
}
VkSurfaceKHR platform_vulkan_create_surface(platform_window_t* window, VkInstance instance) {
	VkSurfaceKHR surface;
	VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR};
	create_info.hinstance = context.instance;
	create_info.hwnd = window->handle;
	VkResult surface_result = vkCreateWin32SurfaceKHR(instance, &create_info, NULL, &surface);
	if(surface_result != VK_SUCCESS) return NULL;
	return surface;
}

void platform_handle_events(void) {
	// while context is not needed here, it is needed by the linux platform
	MSG msg;
	while(PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

// based off of code written by ChiliTomatoNoodle (youtube channel)
LRESULT __stdcall window_proc_setup(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
	if(msg == WM_NCCREATE) {
		const CREATESTRUCTA* const pcreate = (CREATESTRUCTA*)l_param;
		SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)pcreate->lpCreateParams);
		SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)window_proc);
		return window_proc(hwnd, msg, w_param, l_param);
	}
	// use default window proc since user data has not been set yet
	return DefWindowProcA(hwnd, msg, w_param, l_param);
}

LRESULT __stdcall window_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
	platform_window_t* window = (platform_window_t*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

	switch(msg)
	{
	case WM_CLOSE: // window wants to close
		window->should_close = 1;
		return 0;
	case WM_DESTROY: break;
	case WM_SIZE:
		platform_terminal_print("Size Event.\n", 0, 0, 0);
		break;
	case WM_KEYDOWN: break;
	case WM_KEYUP: break;
	}

	return DefWindowProcA(hwnd, msg, w_param, l_param);
}

static const WORD forground_color_table[] = {
	0, 0,
	FOREGROUND_RED,
	FOREGROUND_GREEN,
	FOREGROUND_RED | FOREGROUND_GREEN,
	FOREGROUND_BLUE,
	FOREGROUND_RED | FOREGROUND_BLUE,
	FOREGROUND_GREEN | FOREGROUND_BLUE,
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	FOREGROUND_INTENSITY,
	FOREGROUND_RED | FOREGROUND_INTENSITY,
	FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
};
static const WORD background_color_table[] = {
	0, 0,
	BACKGROUND_RED,
	BACKGROUND_GREEN,
	BACKGROUND_RED | BACKGROUND_GREEN,
	BACKGROUND_BLUE,
	BACKGROUND_RED | BACKGROUND_BLUE,
	BACKGROUND_GREEN | BACKGROUND_BLUE,
	BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
	BACKGROUND_INTENSITY,
	BACKGROUND_RED | BACKGROUND_INTENSITY,
	BACKGROUND_GREEN | BACKGROUND_INTENSITY,
	BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY,
	BACKGROUND_BLUE | BACKGROUND_INTENSITY,
	BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
	BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
	BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
};

// TODO: write implimentation for virtual terminal
static inline void _terminal_print(const char* msg, const uint8_t forground, const uint8_t background, const uint8_t flags, HANDLE handle) {
	WORD attributes = 0;
	if(forground > 0) attributes |= forground_color_table[forground];
	if(background > 0) attributes |= background_color_table[background];
	if(flags & PLATFORM_TEXT_NEGITIVE) attributes |= COMMON_LVB_REVERSE_VIDEO;
	if(flags & PLATFORM_TEXT_UNDERLINE) attributes |= COMMON_LVB_UNDERSCORE;
	SetConsoleTextAttribute(handle, attributes);
	uint32_t msg_len = 0;
	DWORD bytes_written = 0;
	while(msg[msg_len] != '\0') msg_len++;
	WriteConsole(handle, msg, msg_len, &bytes_written, NULL);
	SetConsoleTextAttribute(handle, 0);
}

void platform_terminal_print(const char* msg, const uint8_t forground, const uint8_t background, const uint8_t flags) {
	HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	_terminal_print(msg, forground, background, flags, stdout_handle);
}
void platform_terminal_print_error(const char* msg, const uint8_t forground, const uint8_t background, const uint8_t flags) {
	HANDLE stderr_handle = GetStdHandle(STD_ERROR_HANDLE);
	_terminal_print(msg, forground, background, flags, stderr_handle);
}


void* platform_map_memory(void* addr_hint, uint64_t size) {
	return VirtualAlloc(addr_hint, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}
int8_t platform_unmap_memory(void* addr, uint64_t size) {
	return VirtualFree(addr, size, MEM_RELEASE) != 0;
}


uint64_t platform_get_timestamp(void) {
	return 0;
}
void platform_sleep_miliseconds(const uint32_t miliseconds) {
	timeBeginPeriod(1);
	Sleep(miliseconds);
	timeEndPeriod(1);
}