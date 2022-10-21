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

static inline void* _allocator_alloc(uint64_t size, uint64_t alignment, platform_allocation_callbacks_t* allocator) {
	if(allocator != NULL) {
		return allocator->alloc(allocator->user_data, size, alignment);
	}
	else {
		return _aligned_malloc(size, alignment);
	}
}

static inline void _allocator_free(void* addr, platform_allocation_callbacks_t* alloctor) {
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

	platform_context_t* context = _allocator_alloc(sizeof(platform_context_t), 4, allocator);
	context->instance = instance;
	context->class_name = DEFAULT_CLASS_NAME;
	return context;
}

void platform_destroy_context(platform_context_t* context, platform_allocation_callbacks_t* allocator) {
	UnregisterClassA(context->class_name, context->instance);
	_allocator_free(context, allocator);
}


platform_window_t* platform_create_window(platform_context_t* context, const platform_window_create_info_t create_info, platform_allocation_callbacks_t* allocator) {
	DWORD window_style = WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

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

	ShowWindow(handle, SHOW_OPENWINDOW);

	platform_window_t* window = _allocator_alloc(sizeof(platform_window_t), 4, allocator);
	window->handle = handle;
	return window;
}
void platform_destroy_window(platform_context_t* context, platform_window_t* window, platform_allocation_callbacks_t* allocator) {
	DestroyWindow(window->handle);
	_allocator_free(window, allocator);
}
void platform_get_window_position(const platform_context_t* context, const platform_window_t* window, int32_t* x, int32_t* y) {
	
}
void platform_get_window_size(const platform_context_t* context, const platform_window_t* window, uint32_t* width, uint32_t* height) {

}
void platform_set_window_position(const platform_context_t* context, const platform_window_t* window, const int32_t x, const int32_t y) {

}
void platform_set_window_size(const platform_context_t* context, const platform_window_t* window, const uint32_t width, const uint32_t height) {

}
const char* platform_get_window_name(const platform_context_t* context, const platform_window_t* window) {
	return NULL;
}
void platform_set_window_name(const platform_context_t* context, platform_window_t* window, const char* name) {

}

void platform_handle_events(void) {
	MSG msg;
	while(PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

static inline void _terminal_print(const char* msg, const uint8_t forground, const uint8_t background, const uint8_t flags, void* stream) {

}

void platform_terminal_print(const char* msg, const uint8_t forground, const uint8_t background, const uint8_t flags) {
	
}
void platform_terminal_print_error(const char* msg, const uint8_t forground, const uint8_t background, const uint8_t flags) {

}


uint64_t platform_get_timestamp(void) {
	return 0;
}
void platform_sleep_miliseconds(const uint32_t miliseconds) {
	timeBeginPeriod(1);
	Sleep(miliseconds);
	timeEndPeriod(1);
}