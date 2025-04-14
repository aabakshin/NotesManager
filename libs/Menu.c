#ifndef MENU_C_SENTRY
#define MENU_C_SENTRY

#include "../includes/Input.h"
#include "../includes/Menu.h"
#include <stdio.h>
#include <stdlib.h>


static const double app_version = 0.1;
static const char* app_name = "NotesManager";


#ifdef DEBUG
static void view_str(const char* str, int str_len);
#endif


void show_menu(void)
{
	/* printf("%s", "\033[0d\033[2J"); */ /* clear the screen */
	printf("\033c"); /* clear the screen for VT100 terminal */
	fflush(stdout);

	printf("-========== %s version %.1lf ==========-\n", app_name, app_version);
	printf("%s",
			"     (1) - Insert a new note in the table.\n"
			"     (2) - Remove an existing note from the table.\n"
			"     (3) - Print a note with specific ID in stdout.\n"
			"     (4) - Print the table in stdout.\n"
			"     (5) - Exit from app.\n");
	printf("-========== %s version %.1lf ==========-\n\n", app_name, app_version);

	printf("%s", ">>> ");
	fflush(stdout);
}

#ifdef DEBUG
static void view_str(const char* str, int str_len)
{
	printf("String length = %d\n", str_len);
	printf("String format: %s\n", str);

	printf("%s", "Decimal format:\n");
	int i;
	for ( i = 0; i < str_len; i++ )
	{
		printf("%03d ", str[i]);
		if ( ((i+1) % 10) == 0 )
			putchar('\n');
	}
	putchar('\n');

}
#endif

int choose_menu_option(int* mode)
{
	do
	{
		show_menu();

		char buffer[MAX_READ_BUF_SIZE];
		int result = input(buffer, MAX_READ_BUF_SIZE);

#ifdef DEBUG
		if ( result > -1 )
		{
			view_str(buffer, result);
		}
#endif
		if ( result == -1 )
		{
			putchar('\n');
			return 0;
		}

		*mode = atoi(buffer);
	}
	while ( (*mode < INSERT_NEW_NOTE) || (*mode > EXIT_FROM_APP) );

	return 1;
}

#endif
