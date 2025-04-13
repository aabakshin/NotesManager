#ifndef CORE_C_SENTRY
#define CORE_C_SENTRY


#include "../includes/Core.h"
#include "../includes/Input.h"
#include "../includes/Menu.h"
#include "../includes/Operations.h"


extern int (*hndls[HANDLERS_NUM])(FILE*, int);

FILE* open_table_file(const char* filename)
{
	FILE* fd = fopen(FILENAME, "rb+");
	if ( fd == NULL )
	{
		fprintf(stderr, "%s", "Table is not found! Creating new one.\n");
		fd = fopen(FILENAME, "wb+");
		if ( fd == NULL )
		{
			fprintf(stderr, "Unable to create file \"%s\". Maybe not enough permissions?\n", FILENAME);
			return NULL;
		}
	}

	return fd;
}

int running(FILE* fd)
{
	if ( fd == NULL )
	{
		return 0;
	}

	while ( 1 )
	{
		int records_count = get_records_num(fd);
		if ( records_count < 0 )
		{
			return 0;
		}

		int mode = -1;
		if ( !choose_menu_option(&mode) )
		{
			return 0;
		}

		switch ( mode )
		{
			case INSERT_NEW_NOTE:
				/* printf("%s", "\033[0d\033[2J"); */ /* clear the screen */
				printf("\033c"); /* clear the screen for VT100 terminal */
				fflush(stdout);
				if ( !hndls[INSERT_NEW_NOTE](fd, records_count) )
				{
					fprintf(stderr, "%s", "Unable to insert note in the table!\n");
					return 0;
				}
				printf("%s", "\nNote has successfully inserted in the table!\n"
						"Press any key for continue\n");
				fflush(stdout);
				get_any_key();
				break;
			case REMOVE_EXIST_NOTE:
				/* printf("%s", "\033[0d\033[2J"); */ /* clear the screen */
				printf("\033c"); /* clear the screen for VT100 terminal */
				fflush(stdout);
				if ( !hndls[REMOVE_EXIST_NOTE](fd, records_count) )
				{
					fprintf(stderr, "%s", "Unable to remove note from the table!\n");
					return 0;
				}
				printf("%s", "\nNote has successfully removed from the table!\n"
						"Press any key for continue\n");
				fflush(stdout);
				get_any_key();
				break;
			case PRINT_SPECIFIC_NOTE:
				/* printf("%s", "\033[0d\033[2J"); */ /* clear the screen */
				printf("\033c"); /* clear the screen for VT100 terminal */
				fflush(stdout);
				if ( !hndls[PRINT_SPECIFIC_NOTE](fd, records_count) )
				{
					fprintf(stderr, "%s", "Unable to print specific note!\n");
					return 0;
				}
				printf("%s", "\nPress any key for continue\n");
				fflush(stdout);
				get_any_key();
				break;
			case PRINT_TABLE:
				/* printf("%s", "\033[0d\033[2J"); */ /* clear the screen */
				printf("\033c"); /* clear the screen for VT100 terminal */
				fflush(stdout);
				if ( !hndls[PRINT_TABLE](fd, records_count) )
				{
					fprintf(stderr, "%s", "Unable to print the table!\n");
					return 0;
				}
				printf("%s", "\nPress any key for continue\n");
				fflush(stdout);
				get_any_key();
				break;
			case EXIT_FROM_APP:
				/* printf("%s", "\033[0d\033[2J"); */ /* clear the screen */
				printf("\033c"); /* clear the screen for VT100 terminal */
				fflush(stdout);
				return 1;
		}
	}
}


#endif
