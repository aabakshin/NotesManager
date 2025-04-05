#include "Menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define FILENAME "records_table.dat"

enum
{
			NOTE_SIZE				=			50,
			TIMESTAMP_SIZE			=			50
};

struct Note
{
	int id;
	char note[NOTE_SIZE];
	char timestamp[TIMESTAMP_SIZE];
};
typedef struct Note Note;


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


int main(void)
{
	struct stat file_info;
	memset(&file_info, 0, sizeof(file_info));


	int create_file_flag = 0;
	FILE* fd = fopen(FILENAME, "rb+");
	if ( fd == NULL )
	{
		fprintf(stderr, "%s", "Table is not found! Creating new one.\n");
		fd = fopen(FILENAME, "wb+");
		if ( fd == NULL )
		{
			fprintf(stderr, "Unable to create file \"%s\". Maybe not enough permissions?\n", FILENAME);
			return 1;
		}
		create_file_flag = 1;
	}

	if ( !create_file_flag )
	{
		if ( lstat(FILENAME, &file_info) == -1 )
		{
			perror("fstat");
			fclose(fd);
			return 1;
		}

		long size = file_info.st_size;
		if ( (size % sizeof(struct Note)) != 0 )
		{
			fprintf(stderr, "File \"%s\" has invalid structure!\n", FILENAME);
			fclose(fd);
			return 1;
		}
	}
	
	while ( 1 )
	{
		int mode = -1;
		if ( !choose_menu_option(&mode) )
		{
			fclose(fd);
			return 1;
		}

		int success_operation = 0;
		switch ( mode )
		{
			case INSERT_NEW_NOTE:

				break;
			case REMOVE_EXIST_NOTE:

				break;
			case PRINT_SPECIFIC_NOTE:

				break;
			case PRINT_TABLE:

				break;
			case EXIT_FROM_APP:
				fclose(fd);
				exit(0);
		}
	}

	fclose(fd);

	return 0;
}

