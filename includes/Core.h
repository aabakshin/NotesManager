#ifndef CORE_H_SENTRY
#define CORE_H_SENTRY


#include "Services.h"


FILE* open_table_file(const char* filename);
int running(FILE* fd);

#endif
