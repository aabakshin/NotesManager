#ifndef MENU_H_SENTRY
#define MENU_H_SENTRY

enum
{
			INSERT_NEW_NOTE				=			1,
			REMOVE_EXIST_NOTE,
			PRINT_SPECIFIC_NOTE,
			PRINT_TABLE,
			EXIT_FROM_APP
};

void show_menu(void);
int choose_menu_option(int* mode);

#endif
