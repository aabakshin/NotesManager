#include "Input.h"
#include "Menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>


int main(void)
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


	/* восстановление канонического режима */
	tcsetattr(0, TCSANOW, &t2);

	return 0;
}

