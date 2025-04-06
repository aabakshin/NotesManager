#include "Input.h"
#include "Menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>


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

static char* get_date_str(char* current_date, int date_size)
{
	if ( current_date == NULL )
		return NULL;

	if ( date_size < TIMESTAMP_SIZE )
		date_size = TIMESTAMP_SIZE;

	time_t cur_time_in_secs = time(0);
	struct tm* cur_time = NULL;
	cur_time = localtime(&cur_time_in_secs);
	strftime(current_date, date_size, "%a %d %b %Y %H:%M:%S", cur_time);

	return current_date;
}

int insert_new_note(FILE* fd, int file_size)
{
	char buffer[NOTE_SIZE];
	int result = input(buffer, NOTE_SIZE);
	if ( result == -1 )
	{
		putchar('\n');
		return 0;
	}
	
	int len = strlen(buffer);
	if ( buffer[len-1] == '\n' )
		buffer[len-1] = '\0';

	int records_count = file_size / sizeof(Note);
	
	if ( records_count < 0 )
	{
		return 0;
	}

	Note record;
	memset(&record, 0, sizeof(Note));

	if ( records_count == 0 )
	{
		record.id = 1;
	}
	else
	{
		record.id = records_count + 1;
	}
	
	strcpy(record.note, buffer);
	get_date_str(record.timestamp, TIMESTAMP_SIZE);
	
	fseek(fd, 0, SEEK_END);
	fwrite(&record, sizeof(Note), 1, fd);

	return 1;
}

int remove_exist_note(FILE* fd, int file_size)
{
	return 1;
}

int print_specific_note(FILE* fd, int file_size)
{


}

int print_table(FILE* fd, int file_size)
{
}

int main(void)
{
	struct stat file_info;
	memset(&file_info, 0, sizeof(file_info));

	int (*hndls[HANDLERS_NUM])(FILE*, int) =
	{
				NULL,
				insert_new_note,
				remove_exist_note,
				print_specific_note,
				print_table
	};

	int create_file_flag = 0;
	long size = 0;
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

		size = file_info.st_size;
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
				printf("%s", "\nEnter a note: ");
				fflush(stdout);
				if ( !hndls[INSERT_NEW_NOTE](fd, size) )
				{
					fclose(fd);
					fprintf(stderr, "%s", "Unable to insert note in the table!\n");
					return 1;
				}
				printf("%s", "\nNote has successfully inserted in the table!\n");
				fflush(stdout);
				sleep(2);
				break;
			case REMOVE_EXIST_NOTE:
				printf("%s", "\nEnter note ID: ");
				fflush(stdout);
				if ( !hndls[REMOVE_EXIST_NOTE](fd, size) )
				{
					fclose(fd);
					fprintf(stderr, "%s", "Unable to remove note from the table!\n");
					return 1;
				}
				printf("%s", "\nNote has successfully removed from the table!\n");
				fflush(stdout);
				sleep(2);
				break;
			case PRINT_SPECIFIC_NOTE:
				printf("%s", "\nEnter note ID: ");
				fflush(stdout);
				if ( !hndls[PRINT_SPECIFIC_NOTE](fd, size) )
				{
					fclose(fd);
					fprintf(stderr, "%s", "Unable to print specific note!\n");
					return 1;
				}
				printf("%s", "\nPress any key for continue\n");
				fflush(stdout);
				getchar();
				break;
			case PRINT_TABLE:
				if ( !hndls[PRINT_TABLE](fd, size) )
				{
					fclose(fd);
					fprintf(stderr, "%s", "Unable to print the table!\n");
					return 1;
				}
				printf("%s", "\nPress any key for continue\n");
				fflush(stdout);
				getchar();
				break;
			case EXIT_FROM_APP:
				fclose(fd);
				exit(0);
		}
	}

	fclose(fd);

	return 0;
}

