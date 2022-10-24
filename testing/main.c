#include <platform/platform.h>
#include <stdio.h>

#ifndef NULL
#define NULL (void*)0
#endif // NULL

int main(void) {
	platform_terminal_print("Program Start...\n", PLATFORM_COLOR_BLUE, 0, 0);
	platform_context_t* platform_context = platform_create_context(NULL, NULL);
	if(platform_context == NULL) return -1;

	platform_window_create_info_t create_info;
	create_info.name = "Cat Window";
	create_info.x = 100;
	create_info.y = 100;
	create_info.width = 500;
	create_info.height = 300;
	create_info.parent = NULL;
	create_info.flags = PLATFORM_WF_SPLASH;

	platform_window_t* window = platform_create_window(platform_context, create_info, NULL);
	if(window == NULL) return platform_destroy_context(platform_context, NULL), 0;

	uint32_t i = 0;
	while(1) {
		platform_handle_events(platform_context);
		char buffer[32] = {0};
		sprintf(buffer, "\rIteration: %d", i++);
		platform_terminal_print(buffer, PLATFORM_COLOR_BLUE, 0, PLATFORM_TEXT_BOLD);
		putchar('\r');
		platform_sleep_miliseconds(16);
	}

	platform_destroy_window(platform_context, window, NULL);
	platform_destroy_context(platform_context, NULL);

	return 0;
}