#ifndef SERVICES_C_SENTRY
#define SERVICES_C_SENTRY


#include "Services.h"
#include "Input.h"
#include "Menu.h"


enum
{
			ID_SIZE					=			20,
			NOTE_SIZE				=			50,
			TIMESTAMP_SIZE			=			50
};

enum
{
			HANDLERS_NUM			=			5
};

struct Note
{
	char id[ID_SIZE];
	char note[NOTE_SIZE];
	char timestamp[TIMESTAMP_SIZE];
};
typedef struct Note Note;


static void reverse(char* s);
static int num_digit_cnt(int number);
static void itoa(int number, char* num_buf, int max_buf_len);
static char* get_date_str(char* current_date, int date_size);
static int get_records_num(FILE* fd);
static int get_record_index(FILE* fd);
static int insert_new_note(FILE* fd, int records_count);
static int remove_exist_note(FILE* fd, int records_count);
static int print_specific_note(FILE* fd, int records_count);
static int print_table(FILE* fd, int records_count);


static int (*hndls[HANDLERS_NUM])(FILE*, int) =
{
				NULL,
				insert_new_note,
				remove_exist_note,
				print_specific_note,
				print_table
};


static void reverse(char* s)
{
	int i = 0;
	int j = strlen(s)-1;

	for ( ; i < j; i++, j-- )
	{
		char c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

static int num_digit_cnt(int number)
{
	int counter = 0;

	if ( number == 0 )
		return 1;

	if ( number < 0 )
		number *= -1;

	while ( number > 0 )
	{
		counter++;
		number /= 10;
	}

	return counter;
}

static void itoa(int number, char* num_buf, int max_buf_len)
{
	if ( number == 0 )
	{
		num_buf[0] = '0';
		num_buf[1] = '\0';
		return;
	}

	int cnt = num_digit_cnt(number);

	if ( cnt > (max_buf_len-1) )
		cnt = max_buf_len-1;

	int flag = 0;
	if ( number < 0 )
	{
		number *= -1;
		flag = 1;
	}

	int i = 0;
	while ( (number > 0) && (i < cnt) )
	{
		num_buf[i] = (number % 10) + '0';
		number /= 10;
		i++;
	}

	if ( flag )
	{
		num_buf[i] = '-';
		i++;
	}
	num_buf[i] = '\0';

	reverse(num_buf);
}

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

static int get_records_num(FILE* fd)
{
	struct stat st;
	memset(&st, 0, sizeof(struct stat));

	if ( lstat(FILENAME, &st) == -1 )
	{
		fprintf(stderr, "Unable to get size of \"%s\" file!\n", FILENAME);
		return -1;
	}
	
	if ( ((st.st_size % sizeof(Note)) != 0) || (st.st_size < 0) )
	{
		fprintf(stderr, "File \"%s\" has incorrect structure size!\n", FILENAME);
		return -1;
	}

	if ( st.st_size == 0 )
	{
		return 0;
	}

	int records_num = 0;

	fseek(fd, 0, SEEK_SET);

	Note note;
	int cur_id = 0;
	do
	{
		memset(&note, 0, sizeof(note));
		fread(&note, sizeof(Note), 1, fd);

		cur_id = atoi(note.id);
		if ( cur_id > 0 )
			records_num++;
	}
	while ( !feof(fd) );

	return records_num;
}

/* получение индекса первой свободной записи таблицы */
static int get_record_index(FILE* fd)
{
	int records_num = get_records_num(fd);

	if ( records_num < 0 )
	{
		return -1;
	}

	if ( records_num == 0 )
	{
		return 1;
	}
	
	fseek(fd, 0, SEEK_SET);

	Note note;
	int index = 0;
	do
	{
		memset(&note, 0, sizeof(note));
		fread(&note, sizeof(Note), 1, fd);
		index++;

		int id = atoi(note.id);
		if ( (id == 0) && (note.note[0] == '\0') && (note.timestamp[0] == '\0') )
		{
			break;
		}
	}
	while ( !feof(fd) );

	if ( index == records_num )
	{
		return 0;
	}
	
	return index;
}

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
				if ( !hndls[INSERT_NEW_NOTE](fd, records_count) )
				{
					fprintf(stderr, "%s", "Unable to insert note in the table!\n");
					return 0;
				}
				printf("%s", "\nNote has successfully inserted in the table!\n");
				printf("%s", "Press any key for continue\n");
				fflush(stdout);
				getchar();
				break;
			case REMOVE_EXIST_NOTE:
				if ( !hndls[REMOVE_EXIST_NOTE](fd, records_count) )
				{
					fprintf(stderr, "%s", "Unable to remove note from the table!\n");
					return 0;
				}
				printf("%s", "\nNote has successfully removed from the table!\n");
				printf("%s", "Press any key for continue\n");
				fflush(stdout);
				getchar();
				break;
			case PRINT_SPECIFIC_NOTE:
				if ( !hndls[PRINT_SPECIFIC_NOTE](fd, records_count) )
				{
					fprintf(stderr, "%s", "Unable to print specific note!\n");
					return 0;
				}
				printf("%s", "\nPress any key for continue\n");
				fflush(stdout);
				getchar();
				break;
			case PRINT_TABLE:
				if ( !hndls[PRINT_TABLE](fd, records_count) )
				{
					fprintf(stderr, "%s", "Unable to print the table!\n");
					return 0;
				}
				printf("%s", "\nPress any key for continue\n");
				fflush(stdout);
				getchar();
				break;
			case EXIT_FROM_APP:
				return 1;
		}
	}
}

static int insert_new_note(FILE* fd, int records_count)
{
	printf("%s", "\nEnter a note: ");
	fflush(stdout);

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

	Note record;
	memset(&record, 0, sizeof(Note));

	if ( records_count == 0 )
	{
		record.id[0] = '1';
		record.id[1] = '\0';
	}
	/* in other case: records_count > 0 */
	else
	{
		char cnt[10];
		//////!
		itoa(records_count + 1, cnt, 9);
		strcpy(record.id, cnt);
	}

	strcpy(record.note, buffer);
	get_date_str(record.timestamp, TIMESTAMP_SIZE);



	/////!
	fseek(fd, 0, SEEK_END);
	fwrite(&record, sizeof(Note), 1, fd);
	fseek(fd, 0, SEEK_SET);

	return 1;
}

static int remove_exist_note(FILE* fd, int records_count)
{
	if ( records_count <= 0 )
	{
		fprintf(stderr, "%s", "\nTable is empty file or file size has invalid value!\n");
		return 0;
	}

	printf("%s", "\nEnter note ID: ");
	fflush(stdout);

	char buffer[10];
	int result = input(buffer, 10);
	if ( result == -1 )
	{
		putchar('\n');
		return 0;
	}

	int len = strlen(buffer);
	if ( buffer[len-1] == '\n' )
		buffer[len-1] = '\0';

	int id = atoi(buffer);
	if ( id < 1 )
	{
		fprintf(stderr, "%s", "\n\"id\" has invalid value!\n");
		return 0;
	}

	Note record;
	int i;
	for ( i = 1; i <= records_count; i++ )
	{
		memset(&record, 0, sizeof(Note));
		fseek(fd, (i-1) * sizeof(Note), SEEK_SET);
		fread(&record, sizeof(Note), 1, fd);

		int rec_id = atoi(record.id);
		if ( id == rec_id )
			break;
	}

	if ( i > records_count )
	{
		fprintf(stderr, "\nUnable to find a note with id=%d in table!\n", id);
		return 0;
	}

	int idx = i;
	const char eof = EOF;

	/* Удаляемая запись последняя */
	if ( idx >= records_count )
	{
		fseek(fd, (idx-1) * sizeof(Note), SEEK_SET);
		fwrite(&eof, sizeof(char), 1, fd);
		fseek(fd, 0, SEEK_SET);
	}
	else
	{
		for ( i = idx; i < records_count; i++ )
		{
			memset(&record, 0, sizeof(Note));
			fseek(fd, i * sizeof(Note), SEEK_SET);
			fread(&record, sizeof(Note), 1, fd);

			fseek(fd, (i-1) * sizeof(Note), SEEK_SET);
			fwrite(&record, sizeof(Note), 1, fd);
		}
		fwrite(&eof, sizeof(char), 1, fd);
		fseek(fd, 0, SEEK_SET);
	}

	/* перенумеровать записи в таблице */


	return 1;
}

static int print_specific_note(FILE* fd, int records_count)
{
	if ( records_count <= 0 )
	{
		fprintf(stderr, "%s", "\nTable is empty file or file size has invalid value!\n");
		return 0;
	}

	printf("%s", "\nEnter note ID: ");
	fflush(stdout);

	char buffer[10];
	int result = input(buffer, 10);
	if ( result == -1 )
	{
		putchar('\n');
		return 0;
	}

	int len = strlen(buffer);
	if ( buffer[len-1] == '\n' )
		buffer[len-1] = '\0';

	int id = atoi(buffer);
	if ( id < 1 )
	{
		fprintf(stderr, "%s", "\n\"id\" has invalid value!\n");
		return 0;
	}

	Note record;
	int i;
	for ( i = 1; i <= records_count; i++ )
	{
		memset(&record, 0, sizeof(Note));
		fseek(fd, (i-1) * sizeof(Note), SEEK_SET);
		fread(&record, sizeof(Note), 1, fd);

		int rec_id = atoi(record.id);
		if ( id == rec_id )
			break;
	}

	if ( i > records_count )
	{
		fprintf(stderr, "\nUnable to find a note with id=%d in table!\n", id);
		return 0;
	}

	printf("\n(%s)     \"%s\"     [%s]\n", record.id, record.note, record.timestamp);

	return 1;
}

static int print_table(FILE* fd, int records_count)
{
	if ( records_count <= 0 )
	{
		fprintf(stderr, "%s", "\nTable is empty file or file size has invalid value!\n");
		return 0;
	}

	Note record;
	int i;
	for ( i = 1; i <= records_count; i++ )
	{
		memset(&record, 0, sizeof(Note));
		fseek(fd, (i-1) * sizeof(Note), SEEK_SET);
		fread(&record, sizeof(Note), 1, fd);

		printf("(%s)     \"%s\"     [%s]\n", record.id, record.note, record.timestamp);
	}

	return 1;
}

#endif
