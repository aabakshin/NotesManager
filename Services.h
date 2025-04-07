#ifndef SERVICES_H_SENTRY
#define SERVICES_H_SENTRY

#define FILENAME "records_table.dat"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>


FILE* open_table_file(const char* filename);
int running(FILE* fd);

#endif
