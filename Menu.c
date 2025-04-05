#ifndef MENU_C_SENTRY
#define MENU_C_SENTRY

static const double app_version = 0.1;
static const char* app_name = "NotesManager";

#include "Menu.h"
#include <stdio.h>

void show_menu(void)
{
	printf("%s", "\033[0d\033[2J"); /* clear the screen */

	printf("-========== %s version %.1lf ==========-\n", app_name, app_version);
	printf("%s",
			"     (1) - Insert a new note in the table.\n"
			"     (2) - Remove an existing note from the table.\n"
			"     (3) - Print a note with specific ID in stdout.\n"
			"     (4) - Print the table in stdout.\n");
	printf("-========== %s version %.1lf ==========-\n\n", app_name, app_version);

	printf("%s", ">>> ");

	fflush(stdout);
}

#endif
