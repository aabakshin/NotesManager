#ifndef INPUT_C_SENTRY
#define INPUT_C_SENTRY

#include "Input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>


enum {   MAX_SYM_CODE_SIZE    =   10   };


static int get_str(char* buffer, int buffer_size);

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
	/*********************************************/	
	
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
	
	/* индекс текущей позиции в buffer */
	int i = 0;
	
	/**/
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
				write(1, &read_sym[0], 1);

				if ( i < buffer_size-1 )
				{
					buffer[i] = '\n';
					i++;
					buffer[i] = '\0';
					break;
				}
				buffer[buffer_size-2] = '\n';
				buffer[buffer_size-1] = '\0';
				
				i = buffer_size-1;
				
				break;
			}
			else if ( (read_sym[0] == '\b') || (read_sym[0] == 127) )	/* Обработка backspace */
			{
				if ( left_offset < 1 )
				{
					if ( i > 0 )
						i--;
					printf("%s", "\b \b");
					fflush(stdout);
				}
				else
				{
					i -= left_offset;
					if ( i < 1 )
					{
						i += left_offset;
						continue;
					}
					
					char buf[buffer_size];
					int x, z = 0;
					for ( x = i; x < i+left_offset; x++, z++ )
						buf[z] = buffer[x];
					buf[z] = '\0';

					i--;
					for ( x = 0; buf[x]; x++ )
					{
						buffer[i] = buf[x];
						i++;
					}

					int shift = 0;
					putchar('\b');
					for ( x = 0; buf[x]; x++ )
					{
						putchar(buf[x]);
						shift++;
					}
					putchar(' ');
					shift++;
					for ( x = 1; x <= shift; x++ )
						putchar('\b');
					fflush(stdout);
				}

				continue;
			}
			else if ( read_sym[0] == 23 ) /* Ctrl-W удаление последнего слова */
			{
				if ( i < 1 )
					continue;

				int last_ch = i-1;
				int cur_pos = i-left_offset;
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

					i = cur_pos;
					int save_i = i;

					int k;
					for ( k = 1; k <= (pos-cur_pos); k++ )
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
						buffer[i] = buf[x];
						putchar(buffer[i]);
						i++;
					}
					fflush(stdout);

					if ( buf[0] != '\0' )
					{
						for ( x = i; x <= last_ch; x++ )
							putchar(' ' );
						for ( ; x > save_i; x-- )
							putchar('\b');
						fflush(stdout);
					}
				}
				continue;
			}
			
			/* обработка 1-байтного ascii символа */
			int spec_flag = 0;
			int save_pos = 0; 

			if ( left_offset > 0 )
			{
				spec_flag = 1;
				char buf[buffer_size];
				int x, j = 0;
				int last_ch = i-1;

				i -= left_offset;
				save_pos = i;
				for ( x = i; x <= last_ch; x++ )
				{
					buf[j] = buffer[x];
					j++;
				}
				buf[j] = '\0';

				buffer[i] = read_sym[0];
				i++;
				
				for ( x = 0; buf[x]; x++ )
				{
					if ( i < buffer_size-1 )
					{
						buffer[i] = buf[x];
						i++;
					}
					else
						break;
				}
			}
			else
			{
				buffer[i] = read_sym[0];
				i++;
			}

			if ( i > buffer_size-2 )
			{
				buffer[buffer_size-2] = '\n';
				buffer[buffer_size-1] = '\0';
				
				i = buffer_size-1;

				break;
			}
			
			if ( spec_flag )
			{
				spec_flag = 0;
				int l_char = i-1;
				int cur_pos = i;
				
				while ( cur_pos >= 0 )
				{
					putchar('\b');
					fflush(stdout);
					cur_pos--;
				}

				for ( cur_pos = 0; cur_pos <= l_char; cur_pos++ )
					write(1, &buffer[cur_pos], 1);

				while ( cur_pos > save_pos+1 )
				{
					putchar('\b');
					fflush(stdout);
					cur_pos--;
				}
			}
			else
			{
				write(1, &read_sym[0], 1);
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

			continue;
		}
		else if ( rc == 3 )
		{
			/* обработка клавиши ARROW_LEFT с 3-байтным кодом */
			if (
						( read_sym[0] == 0x1b )		&&			/* 27 */
						( read_sym[1] == 0x5b )		&&			/* 91 */
						( read_sym[2] == 0x44 )					/* 68 */
			   )
			{
				if ( left_offset < i )
				{
					putchar('\b');
					fflush(stdout);
					left_offset++;
				}
			}

			/* обработка клавиши ARROW_RIGHT с 3-байтным кодом */
			else if (
							( read_sym[0] == 0x1b )		&&		/* 27 */
							( read_sym[1] == 0x5b )		&&		/* 91 */
							( read_sym[2] == 0x43 )				/* 67 */
					)
			{
				if ( left_offset > 0 )
				{
					putchar(' ');
					putchar('\b');
					putchar(buffer[i-left_offset]);
					left_offset--;
				}
				fflush(stdout);
			}			
		}
		else if ( rc == 4 )
		{
			/* обработка клавиши DEL с 4-х байтным кодом */
			if (
						( read_sym[0] == 0x1b )		&&		/* 27 */
						( read_sym[1] == 0x5b )		&&		/* 91 */
						( read_sym[2] == 0x33 )		&&		/* 51 */
						( read_sym[3] == 0x7e )				/* 126 */
			   )
			{
				if ( left_offset > 0 )
				{
					char buf[buffer_size];
					int last_ch = i-1;
					int cur_pos = i-left_offset;
					int k;
					int x = 0;
					for ( k = cur_pos+1; k <= last_ch; k++ )
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
					}
					putchar(' ');
					fflush(stdout);

					for ( ; k >= cur_pos; k-- )
						putchar('\b');
					fflush(stdout);

					if ( i > 0 )
						i--;
					left_offset--;
				}
			}
		}
	}
	
	/* длина строки buffer */
	return i;
}

#endif
