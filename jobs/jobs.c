#include "jobs.h"
#include <string.h>
#include "handlers.h"
#include "../parses/parse.h"
#include "../signal/signal.h"
#include <wait.h>
#include <errno.h>

#define is_same(l,s)	(!compare_str((char *) list_entry((l)), s))

/* Доходим до специального символа или конца списка */
#define go_spec_symb(tmp,num,lid,size)	\
			do {						\
				for((tmp) = list_get_header((num),(lid)); (tmp) != get_head((lid)) &&  \
					(!is_same((tmp),"&&")) && (!is_same((tmp),"||"));			 \
					(tmp) = (tmp)->mnext) size++;								 \
			} while(0)

void free_exec(sing_exec *ex)
{
	int i;
	if (ex != NULL) {
		if(ex->name != NULL) free(ex->name);
		if(ex->argv != NULL) {
			for (i = 0; ex->argv[i] != NULL; free(ex->argv[i]), i++); 
			free(ex->argv);		
		}
		if(ex->file != NULL) free(ex->file);
		if(ex->next != NULL) free_exec(ex->next);
	}
}


/* Подготавливаем аргументы */
void *prepare_args(int num, sing_exec *ex, unsigned mode, list_id lid)
{
	char **argv;
	char *filename;
	int i, size, strsize;
	list *tmp;

	if (num < 0 || num > list_count(lid)) return NULL;


	switch(mode) {
		case FOR_ARGS:
			size = 1;
			go_spec_symb(tmp,num,lid,size);
			argv = malloc(size*sizeof(char *));
			for (i = num; i < num+size-1; i++) {
				strsize = strlen((char *)list_get(i,lid))+1;
				argv[i-num] = malloc(strsize*sizeof(char));
				memcpy(argv[i-num],(char *)list_get(i,lid),strsize);
			}

			/* Список всегда должен завершать NULL */
			argv[i-num] = NULL;

			return argv;

		case FOR_IO:
			size = 1;
			go_spec_symb(tmp,num,lid,size);
			for (i = num; i < num+size-1; i++) {
				if(!compare_str((char *)list_get(i,lid),"<") &&
			 		list_get(i+1,lid) != NULL) {
					filename = _STR_DUP((char *)list_get(i+1,lid));
					list_connect(i-1,i+2,lid);		/* Избавляемся от этих аргументов */
					set_bit(ex->ios,IO_IN);
					return filename;
				}
				if(!compare_str((char *)list_get(i,lid),">") &&
			 		list_get(i+1,lid) != NULL) {
					filename = _STR_DUP((char *)list_get(i+1,lid));
					list_connect(i-1,i+2,lid);		/* Избавляемся от этих аргументов */
					set_bit(ex->ios,IO_OUT);
					return filename;
				}
			}
			return NULL;
		default: return NULL;
	}
	return NULL;
}

/* Попытка запуска исполняемого фаайла */
int try_exec(char *path, sing_exec *ex) 
{
	int state = 0;
	char *tmp;
	char *exec_path = _STR_DUP(path);
	
	while((tmp = find_exec(&exec_path,
		ex->name)) != NULL) {
		if (exec_path == NULL) state = 1;
		if(!execve(exec_path,ex->argv,_ENVIRON)) return 0;
		else state = 1;
		exec_path = tmp;
	}
	return state;
}

void destroy_task(task *tsk)
{
	if(tsk->name != NULL) free(tsk->name);
	if(tsk->first != NULL) free_exec(tsk->first);
}

/* Проверка статуса с последующим выполнением (возвращает 1 если next = NULL) */
void exec_next(sing_exec *ex, int stat)
{
	int spec;

	if (WIFSTOPPED (stat)) return;

	if (ex == NULL) {
		fprintf(stderr, "Ошибка exec_next: передан пустой аргумент ex!\n");
		return;
	}

	if (ex->next != NULL) {
		if((spec = get_from_queue(&(ex->tsk->sp_queue))) != EMPTY_Q) {
			if (((WEXITSTATUS(stat) == 0) && (spec == SPEC_AND)) ||
				((WEXITSTATUS(stat) != 0) && (spec == SPEC_OR))) {
				exec_cmd(ex->next);
			} else {
				ex->tsk->status =  TSK_EXITED;
				return;
			}
		}
	} else ex->tsk->status =  TSK_EXITED;
}

void switch_io(sing_exec *ex)
{
	if (ex->file != NULL) {
		if(bit_seted(ex->ios,IO_OUT))
			if((freopen(ex->file,"w+",stdout)) == NULL)
				perror("open file error!:");
		if(bit_seted(ex->ios,IO_IN))
			if((freopen(ex->file,"r",stdin)) == NULL)
				perror("open file error!:");
	}
}

/* Исполнение команды */
int exec_cmd (sing_exec *ex)
{
	int stat;

	if(ex == NULL) return EMPTY_EX;	/* Если пустая команда */
	
	task *tsk = ex->tsk;			/* Для краткой записи в дальейшем */

#if 0
	if (ex->tsk->mode = RUN_BACKGR) {
		/* Отвязка текущего tty от процесса */
	}
#endif 
		
	if (ex->handler != NULL) {
		stat = ex->handler(ex);
	} else {
		ex->pid = fork();
		if (ex->pid == 0) { 		/* Дочерний процесс */
			switch_io(ex);			/* Если требуется перенаправление в/в */
			set_int_dfl();			/* Установка обработчиков сигналов */
			if((stat = try_exec(getenv("PATH"),ex)) != 0) {
				if( (*(ex->name) == '.')
				&&	((stat = try_exec(getenv("PWD"),ex)) == 0)) {
					_exit(stat);
				}
				printf("\n%s: %s <- исполняемый файл не найден.\n",shell_name,ex->name);
				_exit(stat);
			}
		}
		else {									/* Родитель (оболочка) */ 
			if(tsk->first == ex) {
				_SETPGID(ex->pid,ex->pid);		/* Создаём новую группу процессов (ВАЖНО) */
				tsk->gpid = ex->pid;
				tsk->status = TSK_RUNNING;		/* Задание выполняется */
			} else {
				_SETPGID(tsk->gpid,ex->pid);/* Присоединяем процесс к уже созданной группе */
			}

			tsk -> current_ex = ex;				/* Установка текущей команды */

/* ??? */	current = *tsk;

			/* Проверяем выполняется ли эта команда в фоновом задании */
			if(tsk->mode == RUN_BACKGR) {
				current.gpid = 0;				/* Фоновый процесс не является текущим */
				return PASS_BACKGR;
			} 
			else if(tsk->mode == RUN_ACTIVE) { 
				/* Ожидаем завершение выполнения текущего процесса */
				stat = wait_child(ex);
			}
			else {
				fprintf(stderr, "Ошибка, режим запуска команды не установлен! \n");
				stat = -1;
			}
		}
	}

	exec_next(ex,stat);
	update_jobs();								/* Под вопросом */
	return (WIFEXITED(stat)) ? WEXITSTATUS(stat) : -1;
}


int wait_child(sing_exec *ex)
{
	int child_stat;

	waitpid(ex->pid,&child_stat,WUNTRACED);
	
	if (WIFSTOPPED (child_stat)) {			/* Процесс был остановлен */
		ex->tsk->current_ex = ex;			/* Был остановлен на этой команде */
		add_bg_task(ex->tsk,TSK_STOPPED);
		printf ("%s: process %d \t %s \tstoped by signal :> %s\n", shell_name, ex->pid,
		ex->name, sys_siglist[WSTOPSIG (child_stat)]);
	}

	return child_stat;
}

void update_jobs()
{
	list *tmp, *next;
	task *tsk;

	for (tmp = get_head(bg_jobs)->mnext; 
	tmp != get_head(bg_jobs);
	tmp = next) { 
		tsk = (task*) list_entry(tmp); 
		waitpid(-(tsk->gpid),NULL,WNOHANG);
		errno = 0;									/* Обязательно обнулить errno от старого значения !!! */
		kill(-(tsk->gpid),0);						/* Необходимо проверить работает ли задание */
		if(errno == ESRCH || tsk->status == TSK_EXITED) { /* удостовериться что группа процессов мертва */
			printf("Killed -> %d 	%s\n",
					tsk->gpid, tsk->name);
			next = tmp->mnext;
			list_del_elem(tmp,bg_jobs);
		} else {									/* Если процесс таки не завершился */
			next = tmp->mnext;
		}	
	}
}
 
int find_spec(int i, list_id lid)
{
	int p = i;
	list *tmp;
	list_for_each(tmp,list_get_header(i,lid)) {
		p++;
		if (list_entry(tmp) == NULL) return 0;
		if (is_same(tmp,"&&"))  return p+1; 
		if (is_same(tmp,"||"))  return p+1; 
	}
	return 0;
}


/* Создание нового задания */
task *create_task()
{
	task *tsk = (task *) malloc(sizeof(task));

	tsk -> name = current_cmd;
	tsk -> gpid = 0;
	tsk -> status = 0;
	init_queue(&(tsk -> sp_queue));					/* Инициализация очереди специальных символов */
	tsk -> first = create_exec_queue(tsk);
	tsk -> current_ex = NULL;

	return (tsk -> first == NULL) ? NULL : tsk;		/* Пустое задание - это ничего, => незачем возвращать что-то кроме NULL */
}

/* Запуск выполнения задания */
int exec_task(task *tsk)
{
	int stat = exec_cmd(tsk->first);
	if (tsk -> mode == RUN_BACKGR) {
		add_bg_task(tsk,bg_jobs); 
		printf("+1 background -> %d\n", tsk->gpid );
	}
	return stat;
}

sing_exec *make_sing_exec(task *tsk,int num,list_id arg_list)
{
	sing_exec *ex = NULL;
	ex = (sing_exec *) malloc(sizeof(sing_exec));
	ex -> ios = 0;
	if (arg_list != UNINIT) {
		ex -> name = _STR_DUP((char *) list_get(num,arg_list));
		ex -> file = (char *)  prepare_args(num, ex , FOR_IO, arg_list);	
		ex -> argv = (char **) prepare_args(num, ex,  FOR_ARGS, arg_list);
	} else {
		fprintf(stderr, "Ошибка make_sing_exec! Не инициализирован список аргументов! \n");
		return NULL;
	}
	ex -> handler = is_shell_cmd(ex->name);	/* Проверяем, встроена ли функция в оболочку */
	ex -> tsk = tsk;							/* Указываем на принадлежность к заданию */
	ex -> next = NULL;

	return ex;
}

/* Создание очереди на исполнение */
sing_exec *create_exec_queue(task *tsk)
{
	sing_exec *ex, *past, *next;
	list *tmp;

	int i = 0;

	if (arg_list == UNINIT) return NULL;
	if (list_empty(get_head(arg_list))) return NULL;

	tsk->mode = RUN_ACTIVE;							/* По умолчанию */
	int list_size = list_count(arg_list);
	
	/* Первое прохождение */
	list_for_each(tmp, get_head(arg_list)) {
		if (is_same(tmp,"&&")) add_to_queue(SPEC_AND,&(tsk->sp_queue));
		if (is_same(tmp,"||")) add_to_queue(SPEC_OR,&(tsk->sp_queue));
	}

	if(!compare_str((char *)list_get(list_size-1,arg_list),"&")) {
				tsk->mode = RUN_BACKGR;
				list_connect(list_size-2,list_size,arg_list);		/* Избавляемся от этого символа */
			}
	ex = next = NULL;
	/* Образование самого первого процесса в очереди процессов */
	ex = make_sing_exec(tsk,0,arg_list);

	if(!queue_empty(tsk->sp_queue)) {				/* Учавствует более одного процесса */
		past = ex;
		i = 0;
		while((i = find_spec(i,arg_list))) {
			next = make_sing_exec(tsk,i,arg_list);
			past->next = next;
			past = next;
		}
	}
	return ex;
}

/* Поиск встроенной команды оболочки */
int (* is_shell_cmd(char *cmd)) (void *)
{
	list *tmp;
	list_for_each(tmp,get_head(sh_jobs)) {
		if (!compare_str(((job *)list_entry(tmp)) -> name, cmd)) 
			return ((job *)list_entry(tmp)) -> handler;
	}
	return NULL;
}

/* Инициализация встроенных обработчиков */
void init_jobs()
{
	sh_jobs = init_list();
	bg_jobs = init_list();

	add_job("exit",	exit_handl);
	add_job("pwd",	pwd_handl);
	add_job("cd",	cd_handl);
	add_job("version", version_handl);
	add_job("meow",meow_handl);
	add_job("declare",declare_handl)
	add_job("kill",kill_handl);
	add_job("jobs",jobs_handl);
	add_job("fg",fg_handl);
}

void del_jobs()
{
	list_del(&sh_jobs);
	list_del(&bg_jobs);
}