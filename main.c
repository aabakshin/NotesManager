#include "Core.h"

int main(void)
{
	FILE* fd = open_table_file(FILENAME);
	if ( fd == NULL )
		return 1;

	if ( !running(fd) )
	{
		fclose(fd);
		return 1;
	}

	fclose(fd);
	return 0;
}
