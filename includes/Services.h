#ifndef SERVICES_H_SENTRY
#define SERVICES_H_SENTRY


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


void itoa(int number, char* num_buf, int max_buf_len);
char* get_date_str(char* current_date, int date_size);

#endif
