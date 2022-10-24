#include "platform/platform.h"

#include <Windows.h>
#include <malloc.h>

#define DEFAULT_CLASS_NAME "WIN32_PLATFORM_CLASS"

struct platform_context_t {
	HINSTANCE instance;
	char* class_name;
};

struct platform_window_t {
	HWND handle;
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
		free(addr);
	}
}

platform_context_t* platform_create_context(const platform_context_settings_t* settings, platform_allocation_callbacks_t* allocator) {
	HINSTANCE instance = GetModuleHandleA(NULL);

	// check if gui applications can be created
	// https://learn.microsoft.com/en-us/windows/win32/winstation/window-stations
	HWINSTA station = GetProcessWindowStation();
	if(station == NULL) return NULL;
	const char valid_station_name[8] = "WinSta0";
	char station_name[32];
	DWORD len_needed;
	GetUserObjectInformationA(station, UOI_NAME, (void*)station_name, 32, &len_needed);
	for(int i = 0; i < 8; i++) if(station_name[i] != valid_station_name[i]) return NULL;

	WNDCLASSEXA class;
	class.cbSize = sizeof(WNDCLASSEXA);
	class.style = CS_OWNDC;
	class.lpfnWndProc = DefWindowProc;
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
	if(class_result == 0) return NULL;

	platform_context_t* context = platform_allocator_alloc(sizeof(platform_context_t), 4, allocator);
	context->instance = instance;
	context->class_name = DEFAULT_CLASS_NAME;
	return context;
}

void platform_destroy_context(platform_context_t* context, platform_allocation_callbacks_t* allocator) {
	UnregisterClassA(context->class_name, context->instance);
	platform_allocator_free(context, allocator);
}


platform_window_t* platform_create_window(platform_context_t* context, const platform_window_create_info_t create_info, platform_allocation_callbacks_t* allocator) {
	DWORD window_style = WS_SIZEBOX;
	if((create_info.flags & PLATFORM_WF_NO_BORDER) == 0 && (create_info.flags & PLATFORM_WF_SPLASH) == 0) {
		window_style |= WS_SYSMENU;
		if((create_info.flags & PLATFORM_WF_POPUP) == 0) {
			window_style |= WS_MINIMIZEBOX;
			window_style |= WS_MAXIMIZEBOX;
		}
	}
	else {
		window_style |= WS_POPUP; // this prevents the window title from showing on the window
	}
	if((create_info.flags & PLATFORM_WF_UNMAPPED) == 0) {
		window_style |= WS_VISIBLE;
	}
	else {
		window_style |= WS_ICONIC;
	}

	RECT wr;
	wr.left = create_info.x;
	wr.top = create_info.y;
	wr.right = create_info.x + create_info.width;
	wr.bottom = create_info.y + create_info.height;

	AdjustWindowRect(&wr, window_style, FALSE);

	HWND parent = create_info.parent != NULL ? create_info.parent->handle : NULL;

	HWND handle = CreateWindowA(context->class_name, create_info.name, window_style,
	                            wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top,
	                            parent, NULL, context->instance, NULL);
	if(handle == NULL) {
		return NULL;
	}

	platform_window_t* window = platform_allocator_alloc(sizeof(platform_window_t), 4, allocator);
	window->handle = handle;
	return window;
}
void platform_destroy_window(platform_context_t* context, platform_window_t* window, platform_allocation_callbacks_t* allocator) {
	DestroyWindow(window->handle);
	platform_allocator_free(window, allocator);
}
void platform_get_window_position(const platform_context_t* context, const platform_window_t* window, int32_t* x, int32_t* y) {
	RECT client_rect;
	GetClientRect(window->handle, &client_rect);
	if(x) *x = client_rect.left;
	if(y) *y = client_rect.top;
}
void platform_get_window_size(const platform_context_t* context, const platform_window_t* window, uint32_t* width, uint32_t* height) {
	RECT cr;
	GetClientRect(window->handle, &cr);
	if(width) *width = cr.right - cr.left;
	if(width) *width = cr.bottom - cr.top;
}
void platform_set_window_position(const platform_context_t* context, platform_window_t* window, const int32_t x, const int32_t y) {
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
void platform_set_window_size(const platform_context_t* context, platform_window_t* window, const uint32_t width, const uint32_t height) {
	RECT wr;
	GetClientRect(window->handle, &wr);
	wr.right = wr.left + width;
	wr.bottom = wr.top + height;
	LONG window_style = GetWindowLongA(window->handle, GWL_STYLE);
	MoveWindow(window->handle, wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top, FALSE);
}
void platform_get_window_name(const platform_context_t* context, const platform_window_t* window, char* name, uint32_t max_len) {
	GetWindowTextA(window->handle, name, max_len);
}
void platform_set_window_name(const platform_context_t* context, platform_window_t* window, const char* name) {
	SetWindowTextA(window->handle, name);
}

void platform_handle_events(const platform_context_t* context) {
	// while context is not needed here, it is needed by the linux platform
	MSG msg;
	while(PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
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


uint64_t platform_get_timestamp(void) {
	return 0;
}
void platform_sleep_miliseconds(const uint32_t miliseconds) {
	timeBeginPeriod(1);
	Sleep(miliseconds);
	timeEndPeriod(1);
}