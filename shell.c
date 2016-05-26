/*	Минималистичный командный интерпертатор smallsh (в будующем возможно "nanosh")
 *	Данное ПО создано исключительно в образовательных и некоммерческих целях.
 *	Оно не защищено какими-либо лицензиями на распространение или модификацию и является
 *	жертвой больного воображения автора. Существует в виде "как есть", без каких-либо гарантии.
 *	Поддерживаемая ОС - Linux с ядром версии от 2.6.11 и выше. 
 *	Дополнительные флаги при компиляции - скрещенные пальцы.
 *	Здесь есть пасхалка :)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "defines.h"
#include "services/bits.h"
#include "./parses/parse.h"
#include "./jobs/jobs.h"
#include "./signal/signal.h"
#include "general.h"
#include "shell.h"

#ifdef _READLINE_H_
 	char shell_prompt[100];
#endif

/* Инициализация служебных систем оболочки */
void init_shell(char *argv[])
{
	/* Здесь обрабатываюсь аргументы, полученные от пользователя при запуске оболчки */
	char *p;
	p = shell_name = strdup(argv[0]);
	shell_name += strlen(shell_name) - 1;
	for (; (shell_name != p) &&	/* Получаем полное имя процесса оболочки */
	     (*(shell_name - 1) != CH_DIR_SEP); shell_name--) ;

	init_general();
	init_jobs();
	init_signals();
}

/* Запуск команды */
void exec_command()
{
	task *tsk;		/* Исполняемая единица - абстракция единица задания */

	tsk = create_task();	/* Создание очереди на исполнение */

	if (tsk != NULL)
		exec_task(tsk);	/* и непосредственное исполнение */
}

/* Получаем команду от пользователя */

char *get_command(char *cmd)
{
	char *p, *q = cmd;
 get:	p = q = fgets(q, CMD_SIZE, stdin);
	while (*p)
		if (*p++ == '#') {
			*(p - 1) = '\0';
			return cmd;
		}
	q += strlen(q) - 1;
	for (; ((q > cmd) && (*q != ESCAPING)); q--) ;
	if (*q == ESCAPING) {	/* Учитываем эффект экранирования для некоторых символов */
		if (*(q + 1) != ';') {
			printf("->");	/* Хотите - ругайтесь, но выглядит */
			goto get;	/* это достаточно элегантно. ИМХО */
		}
	}
	
	return cmd;
}

static inline void clear_cmd_buff(char *cmd_buf)
{
	int i;
	for (i = 0; i < CMD_SIZE; i++)
		cmd_buf[i] = 0;
}

int main(int argc, char *argv[])
{
	char *cmd;		/* Указатель на следующую команду */

#ifdef _READLINE_H_
	char *command;
	snprintf(shell_prompt, sizeof(shell_prompt), "%s:%s $ ", getenv("USER"), getcwd(NULL, 1024));
#else	
	char command[CMD_SIZE];
#endif

	init_shell(argv);

	for (;;) {
#ifdef _READLINE_H_
		snprintf(shell_prompt, sizeof(shell_prompt), "%s:%s#|>", user_name, short_path(curr_path));
		command = readline(shell_prompt);
		if (*command == '\0')
            continue;
        /* автозавершение через табуляцию */
        rl_bind_key('\t', rl_complete);
        /* Добавляем в историю команд */
        cmd = command;
#else
		clear_cmd_buff(command);	/* Принудительная очистка буффера команд */
		printf("%s:%s#|>", user_name, short_path(curr_path));
		cmd = get_command(command);	/* Выполнение команды */
#endif
		do {
			cmd = parse_cmd(cmd);
			exec_command();
		} while (cmd != NULL);
#ifdef _READLINE_H_		
        add_history(command);
        free(command);
#endif
	}

	return 0;
}

void end_of_work(int status)
{
	del_general();
	del_jobs();
	_exit(status);
}
