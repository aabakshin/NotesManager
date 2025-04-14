#ifndef INPUT_C_SENTRY
#define INPUT_C_SENTRY


#include "../includes/Input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>


enum {   MAX_SYM_CODE_SIZE    =   10   };

static int get_str(char* buffer, int buffer_size);
static int handle_ascii_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch);
static int handle_ctrlw_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch);
static int handle_newline_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch);
static int handle_backspace_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch);
static int handle_arrow_left_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch);
static int handle_arrow_right_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch);
static int handle_del_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch);

/* UTF-16LE */
const char rus_alpha_codes[] = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя";

int get_any_key(void)
{
	struct termios t1, t2;
	tcgetattr(0, &t1);
	memcpy(&t2, &t1, sizeof(t1));

	t1.c_lflag &= ~ICANON;
	t1.c_lflag &= ~ISIG;
	t1.c_lflag &= ~ECHO;
	t1.c_cc[VMIN] = 0;
	t1.c_cc[VTIME] = 0;

	tcsetattr(0, TCSANOW, &t1);

	char read_sym[MAX_SYM_CODE_SIZE] = { 0 };
	while ( 1 )
	{
		int rc = read(0, read_sym, 6);	/* 6 - макс. размер в байтах кода клавиши на клавиатуре(F1-F12) */

		if ( rc < 1 )
			continue;

		break;
	}

	tcsetattr(0, TCSANOW, &t2);

	return 1;
}

int input(char* buffer, int buffer_size)
{
	/* Выключение канонического режима терминала */
	struct termios t1, t2;
	tcgetattr(0, &t1);
	memcpy(&t2, &t1, sizeof(t1));

	t1.c_lflag &= ~ICANON;
	t1.c_lflag &= ~ISIG;
	t1.c_lflag &= ~ECHO;
	t1.c_cc[VMIN] = 0;
	t1.c_cc[VTIME] = 0;

	tcsetattr(0, TCSANOW, &t1);

	int result = get_str(buffer, buffer_size);

	/* восстановление канонического режима */
	tcsetattr(0, TCSANOW, &t2);

	return result;
}

/* Обработка терминального ввода в ручном режиме при помощи termios
 * Реализация некоторых возможностей терминала(backspace, удаление последнего слова, del, стрелка влево-вправо)
 * Получение строки из станд.потока ввода через низкоуровненые функции и
 * обработка содержимого с учётом использования многобайтных символов
 * Кол-во вводимых с стандартного потока ввода символов должно быть не более buffer_size-2
 * Возвращает длину получившейся строки вместе с символом line feed */
static int get_str(char* buffer, int buffer_size)
{
	if ( (buffer == NULL) || (buffer_size < 2) )
		return -1;

	/* буфер под прочитанный символ */
	char read_sym[MAX_SYM_CODE_SIZE] = { 0 };

	/* текущая длина buffer */
	int i = 0;

	/* смещение слева относительно конца строки */
	int left_offset = 0;

	while ( 1 )
	{
		int rc = read(0, read_sym, 6);	/* 6 - макс. размер в байтах кода клавиши на клавиатуре(F1-F12) */

		if ( rc < 1 )
			continue;

		if ( rc == 1 )
		{
			if ( read_sym[0] == 3 ) /* Ctrl-C */
			{
				/* завершение программы */
				return -1;
			}
			else if ( (read_sym[0] == 4) || (read_sym[0] == '\n') ) /* 4 => EOF или Ctrl-D */
			{
				handle_newline_key(buffer, buffer_size, &i, &left_offset, read_sym[0]);

				break;
			}
			else if ( (read_sym[0] == '\b') || (read_sym[0] == 127) ) /* Обработка backspace */
			{
				handle_backspace_key(buffer, buffer_size, &i, &left_offset, read_sym[0]);

				continue;
			}
			else if ( read_sym[0] == 23 ) /* Ctrl-W удаление последнего слова */
			{
				handle_ctrlw_key(buffer, buffer_size, &i, &left_offset, read_sym[0]);

				continue;
			}

			/* обработка 1-байтного ascii символа */
			if ( !handle_ascii_key(buffer, buffer_size, &i, &left_offset, read_sym[0]) )
			{
				break;
			}
		}
		else if ( rc == 2 )
		{
			/* Обработка 2-байтных значений кириллицы */
			int is_cyrilic_sym = 0;
			int len = strlen(rus_alpha_codes);
			int j;
			for ( j = 0; j < len; j++ )
			{
				if ( read_sym[0] == rus_alpha_codes[j*2] )
					if ( read_sym[1] == rus_alpha_codes[j*2+1] )
					{
						is_cyrilic_sym = 1;
						break;
					}
			}

			/* вывод кода текущего введённого символа */
			/*printf("\n--> %04x %04x\n", read_sym[0], read_sym[1]);
			fflush(stdout);*/

			if ( is_cyrilic_sym )
			{
				write(1, read_sym, 2);

				if ( (i+2) > buffer_size-2 )
				{
					buffer[buffer_size-2] = '\n';
					buffer[buffer_size-1] = '\0';

					i = buffer_size-1;

					break;
				}

				buffer[i] = read_sym[0];
				i++;
				buffer[i] = read_sym[1];
				i++;
			}
		}
		else if ( rc == 3 )
		{
			/* обработка клавиши ARROW_LEFT с 3-байтным кодом */
			if	(
						( read_sym[0] == 0x1b )		&&			/* 27 */
						( read_sym[1] == 0x5b )		&&			/* 91 */
						( read_sym[2] == 0x44 )					/* 68 */
				)
			{
				handle_arrow_left_key(buffer, buffer_size, &i, &left_offset, read_sym[0]);
			}

			/* обработка клавиши ARROW_RIGHT с 3-байтным кодом */
			else if (
							( read_sym[0] == 0x1b )		&&		/* 27 */
							( read_sym[1] == 0x5b )		&&		/* 91 */
							( read_sym[2] == 0x43 )				/* 67 */
					)
			{
				handle_arrow_right_key(buffer, buffer_size, &i, &left_offset, read_sym[0]);
			}
		}
		else if ( rc == 4 )
		{
			/* обработка клавиши DEL с 4-х байтным кодом */
			if
				(
						( read_sym[0] == 0x1b )		&&		/* 27 */
						( read_sym[1] == 0x5b )		&&		/* 91 */
						( read_sym[2] == 0x33 )		&&		/* 51 */
						( read_sym[3] == 0x7e )				/* 126 */
				)
			{
				handle_del_key(buffer, buffer_size, &i, &left_offset, read_sym[0]);
			}
		}
	}

	/* длина строки buffer */
	return i;
}

static int handle_ascii_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch)
{
	int save_pos = 0;

	if ( *left_offset > 0 )   /* добавляем не в конец */
	{
		char buf[buffer_size];
		int x, j = 0;
		int last_ch = *i - 1;

		*i -= *left_offset;
		save_pos = *i;
		for ( x = *i; x <= last_ch; x++ )
		{
			buf[j] = buffer[x];
			j++;
		}
		buf[j] = '\0';

		buffer[*i] = ch;
		(*i)++;

		for ( x = 0; buf[x]; x++ )
		{
			if ( *i < buffer_size-1 )
			{
				buffer[*i] = buf[x];
				(*i)++;
			}
			else
				break;
		}
	}
	else
	{
		buffer[*i] = ch;
		(*i)++;
	}

	if ( *i > buffer_size-2 )
	{
		buffer[buffer_size-2] = '\n';
		buffer[buffer_size-1] = '\0';

		*i = buffer_size-1;

		return 0;
	}

	if ( *left_offset > 0 )	/* вывод символа добавленного не в конец */
	{
		int last_ch_pos = *i-1;

		for ( int cur_pos = save_pos; cur_pos <= last_ch_pos; cur_pos++ )
		{
			putchar(buffer[cur_pos]);
		}
		fflush(stdout);

		for ( int x = 1; x <= last_ch_pos - save_pos; x++ )
		{
			putchar('\b');
		}
		fflush(stdout);
	}
	else
	{
		write(1, &ch, 1);
	}

	return 1;
}

static int handle_ctrlw_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch)
{
	if ( *i < 1 )
		return 0;

	int last_ch = *i - 1;
	int cur_pos = *i - *left_offset;
	int pos = cur_pos;

	char buf[buffer_size];
	if ( cur_pos > 0 )
	{
		if ( buffer[cur_pos-1] == ' ' )
			while ( (cur_pos > 0) && (buffer[cur_pos-1] == ' ')  )
				cur_pos--;

		if ( cur_pos > 0 )
			while ( (cur_pos > 0) && (buffer[cur_pos-1] != ' ') )
				cur_pos--;

		*i = cur_pos;
		int save_i = *i;

		int k;
		for ( k = 1; k <= pos-cur_pos; k++ )
		{
			printf("\b \b");
			fflush(stdout);
		}

		int x = 0;
		for ( k = pos; k <= last_ch; k++ )
		{
			buf[x] = buffer[k];
			x++;
		}
		buf[x] = '\0';

		for ( x = 0; buf[x]; x++ )
		{
			buffer[*i] = buf[x];
			putchar(buffer[*i]);
			(*i)++;
		}
		fflush(stdout);

		if ( buf[0] != '\0' )
		{
			for ( x = *i; x <= last_ch; x++ )
				putchar(' ');
			for ( ; x > save_i; x-- )
				putchar('\b');
			fflush(stdout);
		}
	}

	return 1;
}

static int handle_newline_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch)
{
	const char nl = '\n';
	write(1, &nl, 1);

	if ( *i < buffer_size-1 )
	{
		buffer[*i] = '\n';
		(*i)++;
		buffer[*i] = '\0';
		return 0;
	}
	buffer[buffer_size-2] = '\n';
	buffer[buffer_size-1] = '\0';

	*i = buffer_size-1;

	return 1;
}

static int handle_backspace_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch)
{
	if ( *left_offset < 1 )
	{
		if ( *i > 0 )
		{
			(*i)--;
			printf("%s", "\b \b");
			fflush(stdout);
		}
	}
	else
	{
		*i -= *left_offset;
		if ( *i < 1 )
		{
			*i += *left_offset;
			return 0;
		}

		char buf[buffer_size];
		int x, z = 0;

		for ( x = *i; x < *i + *left_offset; x++, z++ )
			buf[z] = buffer[x];
		buf[z] = '\0';

		(*i)--;
		putchar('\b');
		for ( x = 0; buf[x]; x++ )
		{
			buffer[*i] = buf[x];
			putchar(buf[x]);
			(*i)++;
		}
		putchar(' ');

		for ( ; x >= 0; x-- )
			putchar('\b');
		fflush(stdout);
	}

	return 1;
}

static int handle_arrow_left_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch)
{
	if ( *left_offset < *i )	/* если текущая позиция буфера в не начале строки - перемещать курсор влево */
	{
		putchar('\b');
		fflush(stdout);
		(*left_offset)++;
	}

	return 1;
}

static int handle_arrow_right_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch)
{
	if ( *left_offset > 0 )	/* если не конец строки - перемещать курсор вправо */
	{
		putchar(buffer[*i - *left_offset]);
		fflush(stdout);
		(*left_offset)--;
	}

	return 1;
}

static int handle_del_key(char* buffer, int buffer_size, int* i, int* left_offset, char ch)
{
	if ( *left_offset > 0 )   /* если не конец строки */
	{
		char buf[buffer_size];
		int last_ch_pos = *i - 1;
		int cur_pos = *i - *left_offset;
		int x = 0;
		int k;
		for ( k = cur_pos+1; k <= last_ch_pos; k++ )
		{
			buf[x] = buffer[k];
			x++;
		}
		buf[x] = '\0';

		x = 0;
		for ( k = cur_pos; buf[x]; x++, k++ )
		{
			buffer[k] = buf[x];
			putchar(buffer[k]);
			fflush(stdout);
		}
		putchar(' ');
		fflush(stdout);

		for ( ; k >= cur_pos; k-- )
			putchar('\b');
		fflush(stdout);

		if ( *i > 0 )
			(*i)--;

		(*left_offset)--;
	}

	return 1;
}

#endif
