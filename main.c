#include "Input.h"
#include "Menu.h"
#include <stdio.h>

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

	return 0;
}

