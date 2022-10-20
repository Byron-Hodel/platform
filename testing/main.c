#include <platform/platform.h>

#ifndef NULL
#define NULL (void*)0
#endif // NULL

int main(void) {
	platform_terminal_print("Program Start...\n", PLATFORM_COLOR_BLUE, 0, 0);
	platform_context_t* platform_context = platform_create_context(NULL, NULL);
	if(platform_context == NULL) return -1;

	platform_terminal_print("Created Platform Context.\n", PLATFORM_COLOR_GREEN, 0, 0);

	platform_destroy_context(platform_context, NULL);

	return 0;
}