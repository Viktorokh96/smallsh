/*	Минималистичный командный интерпертатор smallsh (в будующем возможно "nanosh")
 *	Данное ПО создано исключительно в образовательных и некоммерческих целях.
 *	Оно не защищено какими-либо лицензиями на распространение или модификацию и является
 *	жертвой больного воображения автора. Существует в виде "как есть", без каких-либо гарантии.
 *	Поддерживаемая ОС - Linux с ядром версии от 2.6.11 и выше. 
 *	Дополнительные флаги при компиляции - скрещенные пальцы.
 *	Здесь есть пасхалка.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "defines.h"
#include "services/bits.h"
#include "services/list.h"
#include "./parses/parse.h"
#include "./jobs/jobs.h"
#include "./signal/signal.h"
#include "general.h"
#include "shell.h"

/* Инициализация служебных систем оболочки */
void init_shell(unsigned mode, char *argv[])
{	
	/* Здесь обрабатываюсь аргументы, полученные от пользователя при запуске оболчки */
	char *p;
	p = shell_name = strdup(argv[0]);
	shell_name += strlen(shell_name)-1;
	for(;(shell_name != p) && 		/* Получаем полное имя процесса оболочки */
		(*(shell_name-1) != CH_DIR_SEP); shell_name--);

	/* Поочерёдно иницииализируем все подсистем оболочки */
	/*ВАЖНО! Независимость систем друг от друга не гарантируется! */
	if (bit_seted(mode,SIGNAL)) {	
		init_signals();
	}
	if (bit_seted(mode,JOBS)) {
		init_jobs();
	}
	if (bit_seted(mode,GENERAL)) {
		init_general();
	}
	if (bit_seted(mode,LIST)) {
		arg_list = UNINIT;
	}
}

/* Запуск команды */
void exec_command()
{
	sing_exec *first;

	first = create_exec_queue();				/* Создание очереди на исполнение */
	exec(first);								/* и непосредственное исполнение */
}

/* Получаем команду от пользователя */
 
char *get_command(char *cmd)
{
	char *q = cmd;
get:q = fgets(q,CMD_SIZE,stdin);
    q += strlen(q)-1;
    for(;((q > cmd) && (*q != ESCAPING)); q--);
    if (*q == ESCAPING) {
    	printf("->");	
    	goto get; 											/* Хотите - ругайтесь, но выглядит */
    }														/* это достаточно элегантно. ИМХО */

    return cmd;
}

 
void clear_cmd_buff(char *cmd_buf)
{
	int i;
	for(i = 0; i < CMD_SIZE; i++) cmd_buf[i] = 0; 
}

int main(int argc, char *argv[])
{
	char command[CMD_SIZE];
	char *cmd;											/* Указатель на следующую команду */
	
	init_shell(SIGNAL | JOBS | GENERAL | LIST , argv);
	
	for(;;) {
		clear_cmd_buff(command);					 	/* Принудительная очистка */
		printf("%s:%s#|>",user_name,short_path(curr_path));
		cmd = get_command(command);			 			/* Выполнение команды */
		while((cmd = parse_cmd(cmd)) != NULL)			/* Здесь гарантируется выполнение команд разделённых ';' */
			exec_command();
		exec_command();									/* Выполнение последней команды */	
	}
	
	return 0;
}

void end_of_work()
{
	del_general();
	del_jobs();
	_exit(EXIT_SUCCESS);
}
