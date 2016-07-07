#include <malloc.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "general.h"
#include "./jobs/jobs.h"
#include "./parses/parse.h"

char *path_alloc(unsigned char size, char *path)
{
	if (path != NULL)
		free(path);			/* Освобождаем старую строку */
	return (char *)malloc(PATHSIZE * size * sizeof(char));
}

char *get_curr_path(char *path)
{
	/* Для проверки возвращаемого 
	значения getwd */
	char *ret;	

	static unsigned char n = 1;

	path = path_alloc(n, path);
	ret = _GETWD(path);

	while (ret == NULL) {	/* Проверка на ошибки */
		switch (errno) {
		case EFAULT:
		case EINVAL:
			path = path_alloc(n, path);
			ret = _GETWD(path);
			break;
		case ERANGE:
			path = path_alloc(++n, path);
			ret = _GETWD(path);
			break;
		}
	}

	return path;
}

/* Инициализация главных структур данных оболочки */
int init_general()
{
	struct passwd *userinfo;

	sh_terminal = open("/dev/tty", O_ASYNC | O_RDWR);

	while (tcgetpgrp(sh_terminal) != (shell_pgid = GET_PGID(0)))
		kill(-shell_pgid, SIGTTIN);
	tcsetpgrp(sh_terminal, shell_pgid);
	tcgetattr(sh_terminal, &shell_tmodes);

	init_table(&past_path, 5);				/* Инициализация таблицы истории переходов */

	curr_path = get_curr_path(curr_path);	/* Получение текущей дериктории оболочки */

	home_path = _STR_DUP(getenv("HOME"));	/* Получение информации об домашней дериктории */
	userinfo = getpwuid(geteuid());			/* Получение информации об учётной записи пользователя */
	user_name = userinfo->pw_name;			/* Получаем имя пользователя */

	return 0;
}

void del_general()
{
	if(curr_path)
		free(curr_path);
	if(home_path)
		free(home_path);
	if(shell_name)
		free(shell_name);

	destroy_table(&past_path);
}
