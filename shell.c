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
#include <malloc.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "defines.h"
#include "services/bits.h"
#include "./parses/parse.h"
#include "./jobs/jobs.h"
#include "./signal/signal.h"
#include "general.h"
#include "shell.h"

 	char shell_prompt[100];

/* Инициализация служебных систем оболочки */
void init_shell(char *argv[])
{
	/* Здесь обрабатываюсь аргументы, полученные от пользователя при запуске оболчки */
	char *p;
	p = shell_name = argv[0];
	shell_name += strlen(shell_name) - 1;
	for (; (shell_name != p) &&	/* Получаем полное имя процесса оболочки */
	     (*(shell_name - 1) != CH_DIR_SEP); shell_name--) ;
	shell_name = _STR_DUP(shell_name);


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

static inline void clear_cmd_buff(char *cmd_buf)
{
	int i;
	for (i = 0; i < CMD_SIZE; i++)
		cmd_buf[i] = 0;
}

int main(int argc, char *argv[])
{
	char *cmd;			/* Указатель на следующую команду */
	char *command;
	char *path;
	
	init_shell(argv);	/* Инициализация оболочки */

	snprintf(shell_prompt, sizeof(shell_prompt), "%s:%s $ ", getenv("USER"), curr_path);

	for (;;) {
		
		path = short_path(curr_path, home_path);
		snprintf(shell_prompt, sizeof(shell_prompt), "%s:%s#|>", 
			user_name, path);
		if(path != curr_path && path) free(path);

		command = readline(shell_prompt);
		if (*command == '\0')
            continue;
        
        /* автозавершение через табуляцию */
        rl_bind_key('\t', rl_complete);
        cmd = command;	/* Предотвращение потери исходной команды */
		
		do {
			cmd = parse_cmd(cmd, &arg_vec);
			exec_command();
		} while (cmd != NULL);
        
        /* Добавляем в историю команд */
        add_history(command);
        free(command);
	}

	return 0;
}

void end_of_work(int status)
{
	del_general();
	del_jobs();

	_exit(status);
}
