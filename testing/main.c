#include <platform/platform.h>
#include <stdio.h>

#ifndef NULL
#define NULL (void*)0
#endif // NULL

int main(void) {
	platform_terminal_print("Program Start...\n", PLATFORM_COLOR_BLUE, 0, 0);
	if(!platform_init(NULL)) return -1;

	platform_window_create_info_t create_info;
	create_info.name = "Cat Window";
	create_info.x = 100;
	create_info.y = 100;
	create_info.width = 500;
	create_info.height = 300;
	create_info.parent = NULL;
	create_info.flags = PLATFORM_WF_NORMAL;

	platform_window_t* window = platform_create_window(create_info, NULL);
	if(window == NULL) return platform_shutdown(), 0;

	uint32_t i = 0;
	while(!platform_window_should_close(window)) {
		platform_handle_events();
		char buffer[32] = {0};
		sprintf(buffer, "\rIteration: %d", i++);
		platform_terminal_print(buffer, PLATFORM_COLOR_BLUE, 0, PLATFORM_TEXT_BOLD);
		putchar('\r'); // TODO: This works only when a newly printed line is longer than the Iteration counter line
		platform_sleep_miliseconds(16);
	}

	platform_destroy_window(window, NULL);
	platform_shutdown();

	return 0;
}