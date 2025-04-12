#ifndef OPERATIONS_C_SENTRY
#define OPERATIONS_C_SENTRY


#include "Operations.h"
#include "Input.h"


enum
{
			ID_FIELD_WIDTH					=			10,
			NOTE_FIELD_WIDTH				=			48,
			TIMESTAMP_FIELD_WIDTH			=			24,
			FILL_BETWEEN_WIDTH				=			 3
};

extern const char rus_alpha_codes[];


static int get_record_index(FILE* fd);
static int get_record_by_index(FILE* fd, int index);
static int insert_new_note(FILE* fd, int records_count);
static int remove_exist_note(FILE* fd, int records_count);
static int print_record_strings(const char** strings, int* widths_fields_output, int strings_len);
static int print_specific_note(FILE* fd, int records_count);
static int print_table(FILE* fd, int records_count);


int (*hndls[HANDLERS_NUM])(FILE*, int) =
{
				NULL,
				insert_new_note,
				remove_exist_note,
				print_specific_note,
				print_table
};


/* кол-во непустых записей в таблице */
int get_records_num(FILE* fd)
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
		memset(&note, 0, sizeof(Note));
		fread(&note, sizeof(Note), 1, fd);

		cur_id = atoi(note.id);
		if ( cur_id > 0 )
			records_num++;
	}
	while ( !feof(fd) );

	return records_num;
}

/* получение индекса первой пустой записи таблицы */
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
		return records_num + 1;
	}

	return index;
}

/* есть ли в таблице запись с заданным index'ом */
static int get_record_by_index(FILE* fd, int index)
{
	if ( index < 1 )
	{
		return 0;
	}

	fseek(fd, 0, SEEK_SET);

	Note record;
	int success_flag = 0;

	do
	{
		memset(&record, 0, sizeof(Note));
		fread(&record, sizeof(Note), 1, fd);

		int table_id = atoi(record.id);
		if ( table_id == index )
		{
			success_flag = 1;
			break;
		}
	}
	while ( !feof(fd) );

	if ( !success_flag )
		return 0;

	return 1;
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

	if ( (buffer[0] == '\n') || (buffer[0] == '\0') || (buffer[0] == ' ') )
	{
		fprintf(stderr, "%s", "Empty string!\n");
		return 0;
	}

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
		int index = get_record_index(fd);
		if ( index <= 0 )
		{
			putchar('\n');
			return 0;
		}

		char cnt[10];
		itoa(index, cnt, 9);
		strcpy(record.id, cnt);
	}

	strcpy(record.note, buffer);
	get_date_str(record.timestamp, TIMESTAMP_SIZE);

	int index = atoi(record.id);
	fseek(fd, (index-1) * sizeof(Note), SEEK_SET);
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

	int input_id = atoi(buffer);
	if ( input_id < 1 )
	{
		fprintf(stderr, "%s", "\n\"id\" has invalid value!\n");
		return 0;
	}

	if ( !get_record_by_index(fd, input_id) )
	{
		fprintf(stderr, "\nUnable to find a note with id = %d in table!\n", input_id);
		return 0;
	}

	fseek(fd, (input_id-1)*sizeof(Note), SEEK_SET);

	Note zeroes;
	memset(&zeroes, 0, sizeof(Note));
	fwrite(&zeroes, sizeof(Note), 1, fd);

	return 1;
}

/* ручное форматирование строк для вывода в stdout */
static int print_record_strings(const char** strings, int* widths_fields_output, int strings_len)
{
	if ( (strings == NULL) || (*strings == NULL) )
	{
		return 0;
	}

	int* cyril_cnt = malloc(sizeof(int) * strings_len);
	if ( !cyril_cnt )
		return 0;

	for ( int i = 0; i < strings_len; i++ )
		cyril_cnt[i] = 0;

	int cyrillic_symbols = 0;

	int i;
	for ( i = 0; strings[i] != NULL; i++ )
	{
		int j;
		for ( j = 0; strings[i][j]; j++ )
		{
			int k;
			for ( k = 0; rus_alpha_codes[k]; k++ )
			{
				if ( strings[i][j] == rus_alpha_codes[k] )
				{
					cyrillic_symbols++;
					break;
				}
			}

			putchar(strings[i][j]);
			fflush(stdout);
		}
		cyril_cnt[i] = cyrillic_symbols;
		cyrillic_symbols = 0;

		int len = strlen(strings[i]);
		int spaces_cnt = widths_fields_output[i] - len + cyril_cnt[i]/2;

		/* printf("(%d-%d+%d)", widths_fields_output[i], len, cyril_cnt[i]); */

		if ( i < strings_len-1 )
		{
			spaces_cnt += FILL_BETWEEN_WIDTH;	/* 3 - кол-во пробелов между полями */
		}

		int l;
		for ( l = 1; l <= spaces_cnt; l++ )
		{
			putchar(' ');
			fflush(stdout);
		}
	}

	putchar('\n');
	fflush(stdout);

	if ( cyril_cnt )
		free(cyril_cnt);

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

	int input_id = atoi(buffer);
	if ( input_id < 1 )
	{
		fprintf(stderr, "%s", "\n\"id\" has invalid value!\n");
		return 0;
	}

	if ( !get_record_by_index(fd, input_id) )
	{
		fprintf(stderr, "\nUnable to find a note with id=%d in table!\n", input_id);
		return 0;
	}

	Note record;
	memset(&record, 0, sizeof(Note));

	fseek(fd, (input_id-1)*sizeof(Note), SEEK_SET);
	fread(&record, sizeof(Note), 1, fd);

	putchar('\n');

	const char* strings[] =
	{
				record.id,
				record.note,
				record.timestamp,
				NULL
	};
	int strings_len = sizeof(strings) / sizeof(char*);
	int fields_width[sizeof(strings) / sizeof(char*)] = { ID_FIELD_WIDTH, NOTE_FIELD_WIDTH, TIMESTAMP_FIELD_WIDTH };

	print_record_strings(strings, fields_width, strings_len-1);

	/*printf("\n%-10s   %-48s   %24s\n", record.id, record.note, record.timestamp);
	printf("%ld %ld %ld\n", strlen(record.id), strlen(record.note), strlen(record.timestamp));*/

	fseek(fd, 0, SEEK_SET);

	return 1;
}

static int print_table(FILE* fd, int records_count)
{
	if ( records_count <= 0 )
	{
		fprintf(stderr, "%s", "\nTable is empty file or file size has invalid value!\n");
		return 0;
	}

	fseek(fd, 0, SEEK_SET);
	Note record;
	do
	{
		memset(&record, 0, sizeof(Note));
		fread(&record, sizeof(Note), 1, fd);

		if ( feof(fd) )
			break;

		const char* strings[] =
		{
					record.id,
					record.note,
					record.timestamp,
					NULL
		};
		int strings_len = sizeof(strings) / sizeof(char*);
		int fields_width[sizeof(strings) / sizeof(char*)] = { ID_FIELD_WIDTH, NOTE_FIELD_WIDTH, TIMESTAMP_FIELD_WIDTH };

		if ( (record.id[0] != '0') && (record.note[0] != '\0') && (record.timestamp[0] != '\0') )
		{
			print_record_strings(strings, fields_width, strings_len-1);
			/*printf("%-10s   %-48s   %24s\n", record.id, record.note, record.timestamp);*/
		}
#ifdef DEBUG
		else
		{
			printf_record_strings(strings, fields_width, strings_len-1);
			/*printf("%-10s   %-48s   %24s\n", record.id, record.note, record.timestamp);*/
		}
#endif
	}
	while ( !feof(fd) );

	return 1;
}

#endif
