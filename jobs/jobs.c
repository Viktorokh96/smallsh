#include "jobs.h"
#include <string.h>
#include "handlers.h"
#include "../parses/parse.h"
#include "../signal/signal.h"
#include <wait.h>
#include <errno.h>

#define DEBUG 0

/* Добавление обработчика встроенной функции оболочки */
#define add_job(n,h) 				\
		do {						\
			job *job_sh = malloc(sizeof(job));	\
			job_sh -> name = (n); 		\
			job_sh -> handler = &(h); 	\
			table_add(job_sh,&sh_jobs); \
		} while(0)

/* Особождение памяти под исп. единицу */
void free_exec(sing_exec * ex)
{
	int i;
	if (ex != NULL) {
		if (ex->name != NULL)
			free(ex->name);
		if (ex->argv != NULL) {
			for (i = 0; ex->argv[i] != NULL;
			     free(ex->argv[i]), i++) ;
			free(ex->argv);
		}
		if (ex->file != NULL)
			free(ex->file);
		if (ex->next != NULL)
			free_exec(ex->next);
	}
}

/* Попытка запуска исполняемого фаайла */
int try_exec(char *path, sing_exec * ex)
{
	int state = 0;
	char *tmp;
	char *exec_path = _STR_DUP(path);

	while ((tmp = find_exec(&exec_path, ex->name)) != NULL) {
		if (exec_path == NULL)
			state = 1;
		if (!execve(exec_path, ex->argv, _ENVIRON))
			return 0;
		else
			state = 1;
		exec_path = tmp;
	}
	return state;
}

sing_exec *have_ex(task * tsk, pid_t pid)
{
	if (tsk == NULL)
		return NULL;

	sing_exec *ex = tsk->first;
	for (; ex != NULL; ex = ex->next)
		if (ex->pid == pid)
			return ex;
	return NULL;
}

void destroy_task(task * tsk)
{
	if (tsk->name != NULL)
		free(tsk->name);
	if (tsk->first != NULL)
		free_exec(tsk->first);
}

/* Проверка статуса с последующим выполнением (возвращает 1 если next = NULL) */
void exec_next(sing_exec * ex, int stat)
{
	if (WIFSTOPPED(stat))
		return;

	if (ex == NULL) {
		fprintf(stderr,
			"Ошибка exec_next: передан пустой аргумент ex!\n");
		return;
	}

	if (ex->next != NULL) {
		if (ex->ex_mode != NO_EX)
			if (((WEXITSTATUS(stat) == 0)
			     && (ex->ex_mode == AND_EX))
			    || ((WEXITSTATUS(stat) != 0)
				&& (ex->ex_mode == OR_EX))) {
				ex->ex_mode = NO_EX;	/* На случай если в стеке несколько уровней вызова одной команды */
				exec_cmd(ex->next, NORMAL_NEXT);
			}
	} else {
		ex->tsk->status = TSK_EXITED;
		if (ex->tsk->mode == RUN_ACTIVE)
			destroy_task(ex->tsk);
	}
}

void switch_io(sing_exec * ex)
{
	if (ex->file != NULL) {
		if (bit_seted(ex->ios, IO_OUT))
			if ((freopen(ex->file, "w+", stdout)) == NULL)
				perror("open file error!:");
		if (bit_seted(ex->ios, IO_IN))
			if ((freopen(ex->file, "r", stdin)) == NULL)
				perror("open file error!:");
	}
}

/* Разъединение управляющего терминала группе процессов */
void unset_task_from_term(task * tsk)
{
	tcsetpgrp(sh_terminal, shell_pgid);
	tcgetattr(sh_terminal, &tsk->tmodes);
	tcsetattr(sh_terminal, TCSADRAIN, &shell_tmodes);
}

/* Установка управляющего терминала группе процессов */
void set_task_to_term(task * tsk)
{
	tcsetpgrp(sh_terminal, tsk->pgid);
}

/* Исполнение команды */
int exec_cmd(sing_exec * ex, int8_t mode)
{
	int stat;

	if (ex == NULL)
		return EMPTY_EX;	/* Если пустая команда */

	task *tsk = ex->tsk;	/* Для краткой записи в дальнейшем */

	if (ex->handler != NULL) {
		stat = ex->handler(ex);
	} else {
		ex->pid = fork();
		if (ex->pid == 0) {	/* Дочерний процесс */
			switch_io(ex);	/* Если требуется перенаправление в/в */
			set_int_dfl();	/* Установка обработчиков сигналов */
			if ((stat = try_exec(getenv("PATH"), ex)) != 0) {
				if ((*(ex->name) == '.')
				    && ((stat = try_exec(getenv("PWD"), ex)) ==
					0)) {
					_exit(stat);
				}
				printf
				    ("\n%s: %s <- исполняемый файл не найден.\n",
				     shell_name, ex->name);
				_exit(stat);
			}
		} else {	/* Родитель (оболочка) */
			_SETPGID(ex->pid, ex->pid);
			tsk->pgid = ex->pid;
			tsk->status = TSK_RUNNING;	/* Задание выполняется */

			if (tsk->mode == RUN_ACTIVE
			    && tcgetpgrp(sh_terminal) != tsk->pgid)
				set_task_to_term(tsk);	/* Привязываем группу процесса к терминалу */

			tsk->current_ex = ex;	/* Установка текущей команды */

			/* Проверяем выполняется ли эта команда в фоновом задании */
			if (tsk->mode == RUN_BACKGR) {
				if (ex == tsk->first) {
					add_bg_task(tsk, TSK_RUNNING);
					printf("+1 background -> %d\n",
					       tsk->pgid);
				}
				return PASS_BACKGR;
			} else if (tsk->mode == RUN_ACTIVE) {
				/* Ожидаем завершение выполнения текущего процесса */
				stat = wait_child(ex);
			} else {
				fprintf(stderr,
					"Ошибка, режим запуска команды не установлен! \n");
				stat = -1;
			}
		}
	}

	if (mode == NORMAL_NEXT && ex->tsk->mode != RUN_BACKGR)
		exec_next(ex, stat);
	return (WIFEXITED(stat)) ? WEXITSTATUS(stat) : -1;
}

int wait_child(sing_exec * ex)
{
	int child_stat;

	waitpid(ex->pid, &child_stat, WUNTRACED);

	if (WIFSTOPPED(child_stat)) {	/* Процесс был остановлен */
		ex->tsk->current_ex = ex;	/* Был остановлен на этой команде */
		add_bg_task(ex->tsk, TSK_STOPPED);
		printf("%s: process %d \t %s \tstoped by signal :> %s\n",
		       shell_name, ex->pid, ex->name,
		       sys_siglist[WSTOPSIG(child_stat)]);
	}

	return child_stat;
}

void update_jobs()
{
	int i;
	task *tsk;

	for (i = 0; i < bg_jobs.elem_quant;) {
		tsk = (task *) table_get(i, &bg_jobs);
		waitpid(-(tsk->pgid), NULL, WNOHANG);
		errno = 0;	/* Обязательно обнулить errno от старого значения !!! */
		kill(-(tsk->pgid), 0);	/* Необходимо проверить работает ли задание */
		if (tsk->status == TSK_EXITED || errno == ESRCH) {	/* удостовериться что группа процессов мертва */
			if (tsk->status == TSK_EXITED)
				printf("Done -> %d 	%s\n",
				       tsk->pgid, tsk->name);
			else
				printf("Killed -> %d 	%s\n",
				       tsk->pgid, tsk->name);
			destroy_task(tsk);
			table_del(i, &bg_jobs);
		} else {	/* Если процесс таки не завершился */
			i++;
		}
	}
}

/* Создание нового задания */
task *create_task()
{
	task *tsk = (task *) malloc(sizeof(task));

	tsk->pgid = 0;
	tsk->status = 0;
	tsk->first = create_exec_queue(tsk);
	if(tsk->first != NULL && tsk->first->name != NULL) 
		tsk->name = _STR_DUP(tsk->first->name);
	tsk->current_ex = NULL;

	return (tsk->first == NULL) ? NULL : tsk;	/* Пустое задание - это ничего, => незачем возвращать что-то кроме NULL */
}

/* Запуск выполнения задания */
int exec_task(task * tsk)
{
	int stat = exec_cmd(tsk->first, NORMAL_NEXT);
	update_jobs();
	unset_task_from_term(tsk);
	return stat;
}

void check_ex_mode(sing_exec * ex, char *mode)
{
	if (!strcmp(mode, "&&"))
		ex->ex_mode = AND_EX;
	else if (!strcmp(mode, "||"))
		ex->ex_mode = OR_EX;
	else if (!strcmp(mode, "|"))
		ex->ex_mode = PIPE_EX;
	else
		ex->ex_mode = NO_EX;
}

int select_ex(sing_exec * ex, int *num)
{
	int i;

	if (*num < 0 || *num > arg_vec.elem_quant)
		return -1;

	for (i = *num; i < arg_vec.elem_quant; i++) {
		if (!strcmp((char *)table_get(i, &arg_vec), "&&")
		    || !strcmp((char *)table_get(i, &arg_vec), "||")
		    || !strcmp((char *)table_get(i, &arg_vec), "|")) {
			check_ex_mode(ex, (char *)table_get(i, &arg_vec));
			table_set(i, NULL, &arg_vec);
			return i + 1;
		}
	}

	ex->ex_mode = NO_EX;
	table_add(NULL, &arg_vec);	/* Список аргументов всегда завершает NULL */
	return *num;		/* Если не найдено ни одного выражения */
}

char **make_argv(int num)
{
	int count = 0;
	int i;
	char **argv;
	for (i = num; table_get(i, &arg_vec) != NULL; i++, count++) ;

	argv = malloc(sizeof(char *) * (count + 1));
	for (i = 0; i < count + 1; i++) {
		argv[i] = table_get(num + i, &arg_vec);
	}

	return argv;
}

void check_io(sing_exec * ex, int num)
{
	int i;
	for (i = num; table_get(i, &arg_vec) != NULL; i++) {
		if (!strcmp(">", (char *)table_get(i, &arg_vec))
		    && table_get(i + 1, &arg_vec) != NULL) {
			ex->file = _STR_DUP((char *)table_get(i + 1, &arg_vec));
			ex->ios = IO_OUT;
			table_del(i, &arg_vec);
			table_del(i, &arg_vec);
			return;
		}
		if (!strcmp("<", (char *)table_get(i, &arg_vec))
		    && table_get(i + 1, &arg_vec) != NULL) {
			ex->file = _STR_DUP((char *)table_get(i + 1, &arg_vec));
			ex->ios = IO_IN;
			table_del(i, &arg_vec);
			table_del(i, &arg_vec);
			return;

		}
	}
}

sing_exec *make_sing_exec(task * tsk, int *num)
{
	if (*num < 0 || *num > arg_vec.elem_quant - 1)
		return NULL;
	int old_num;
	sing_exec *ex = NULL;
	ex = (sing_exec *) malloc(sizeof(sing_exec));
	ex->ios = 0;
	ex->name = _STR_DUP((char *)table_get(*num, &arg_vec));
	ex->file = NULL;	/* Временно */

	ex->handler = is_shell_cmd(ex->name);	/* Проверяем, встроена ли функция в оболочку */
	ex->tsk = tsk;		/* Указываем на принадлежность к заданию */
	ex->next = NULL;

	/* Выделяем исполняемую единицу из таблицы аргументов */
	old_num = *num;
	*num = select_ex(ex, num);
	check_io(ex, old_num);
	ex->argv = make_argv(old_num);

	return ex;
}

/* Создание очереди на исполнение */
sing_exec *create_exec_queue(task * tsk)
{
	sing_exec *ex, *past, *next;

	int num, p_num;

	if (arg_vec.elem_quant == 0)
		return NULL;

	tsk->mode = RUN_ACTIVE;	/* По умолчанию */

	if (!strcmp((char *)table_get(arg_vec.elem_quant - 1, &arg_vec), "&")) {
		tsk->mode = RUN_BACKGR;
		table_del(arg_vec.elem_quant - 1, &arg_vec);
	}

	ex = next = NULL;
	num = p_num = 0;

	/* Образование самого первого процесса в очереди процессов */
	ex = make_sing_exec(tsk, &num);

	if (ex->ex_mode != NO_EX) {	/* Участвует более одного процесса */
		past = ex;
		while (p_num != num) {
			p_num = num;
			next = make_sing_exec(tsk, &num);
			past->next = next;
			past = next;
		}
	}

	return ex;
}

/* Поиск встроенной команды оболочки */
int (*is_shell_cmd(char *cmd)) (void *) 
{
	int i;

	for (i = 0; i < sh_jobs.elem_quant; i++) {
		if (!strcmp(((job *) table_get(i, &sh_jobs))->name, cmd))
			return ((job *) table_get(i, &sh_jobs))->handler;
	}

	return NULL;
}

/* Инициализация встроенных обработчиков */
void init_jobs()
{
	init_table(&sh_jobs, 1);
	init_table(&bg_jobs, 5);
	init_table(&arg_vec, 5);

	add_job("exit", exit_handl);
	add_job("pwd", pwd_handl);
	add_job("cd", cd_handl);
	add_job("version", version_handl);
	add_job("meow", meow_handl);
	add_job("declare", declare_handl);
	add_job("kill", kill_handl);
	add_job("jobs", jobs_handl);
	add_job("fg", fg_handl);
	add_job("bg", bg_handl);
	add_job("ls", ls_handl);
}

void del_jobs()
{
	del_table(&arg_vec, FREE_CONT);
	del_table(&sh_jobs, FREE_CONT);
	del_table(&bg_jobs, FREE_CONT);
}
