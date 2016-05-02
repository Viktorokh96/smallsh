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
					set_bit(ex->mode,IO_IN);
					return filename;
				}
				if(!compare_str((char *)list_get(i,lid),">") &&
			 		list_get(i+1,lid) != NULL) {
					filename = _STR_DUP((char *)list_get(i+1,lid));
					list_connect(i-1,i+2,lid);		/* Избавляемся от этих аргументов */
					set_bit(ex->mode,IO_OUT);
					return filename;
				}
			}
			return NULL;

		case FOR_BACKGR:
			size = 1;
			go_spec_symb(tmp,num,lid,size);
				for (i = num; i < num+size-1; i++) {
					if(!compare_str((char *)list_get(i,lid),"&")) {
						set_bit(ex->mode,RUN_BACKGR);
						list_connect(i-1,i+1,lid);		/* Избавляемся от этого символа */
						return NULL;
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

 
void exec_next(sing_exec *ex, int stat)
{
	int spec;
	if (ex->next != NULL) {
		if((spec = get_from_queue(&(ex->tsk->sp_queue))) != EMPTY_Q) {
			if(spec == NO_SPEC) { 
				exec_cmd(ex->next); 
				return; 
			}
			if (((stat == 0) && (spec == SPEC_AND)) ||
				((stat != 0) && (spec == SPEC_OR)))
				exec_cmd(ex->next);
				else free_exec(ex->next);	/* Освобождаем ненужные элементы */ 
		}
	}
}

void switch_io(sing_exec *ex)
{
	if (ex->file != NULL) {
		if(bit_seted(ex->mode,IO_OUT))
			if((freopen(ex->file,"w+",stdout)) == NULL)
				perror("open file error!:");
		if(bit_seted(ex->mode,IO_IN))
			if((freopen(ex->file,"r",stdin)) == NULL)
				perror("open file error!:");
	}
}

/* Исполнение команды */
int exec_cmd (sing_exec *ex)
{
	int stat;

	if(ex == NULL) return 1;								/* Какой-то процесс не был сформирован */
		
	if (ex->handler != NULL) {
		stat = ex->handler(ex->argv);
	} else {
		ex->pid = fork();
		if (ex->pid == 0) { 		/* Дочерний процесс */
			switch_io(ex);			/* Если требуется перенаправление в/в */
			set_int_dfl();			/* Установка обработчиков сигналов */
			_SETPGID(ex->pid,0);	/* Создаём новую группу процессов (ВАЖНО) */
			ex->tsk->gpid = ex->pid;
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
			ex->tsk->current_ex = ex;			/* Установка текущего процесса */
			current = ex->tsk;
			if (bit_seted(ex->mode,RUN_BACKGR)) {
				add_bg_task(ex,TSK_RUNNING);
				printf("+1 background -> %d\n", ex->pid );
				ex->tsk->gpid = 0;	/* Фоновый процесс не является текущим */
			} else { 
				/* Ожидаем завершение выполнения текущего процесса */
				ex->tsk->gpid = ex->pid;
				stat = wait_child(ex);
			}
		}
	}

	exec_next(ex,stat);
	current->gpid = 0;
	free(ex);						/* Процесс отработал своё и больше не нужен */
	return (WIFEXITED(stat)) ? WEXITSTATUS(stat) : -1;
}


int wait_child(sing_exec *ex)
{
	int child_stat;

	waitpid(ex->pid,&child_stat,WUNTRACED);
	
	if (WIFSTOPPED (child_stat)) {			/* Процесс был остановлен */
		add_bg_task(ex,TSK_STOPPED);
		printf ("\n%s: process %d \t %s \tstoped by signal :> %s\n", shell_name, ex->pid,
		ex->name, sys_siglist[WSTOPSIG (child_stat)]);
	}

	return WEXITSTATUS(child_stat);
}

void update_jobs()
{
	list *tmp, *next;
	sing_exec *tsk;

	for (tmp = get_head(bg_jobs)->mnext; 
	tmp != get_head(bg_jobs);
	tmp = next) { 
		tsk = (sing_exec*) list_entry(tmp); 
		kill(-(tsk->pid),0);						/* Необходимо как минимум ещё 1 раз */
		if(errno == ESRCH) {				 		/* удостовериться что группа процессов мертва */
			printf("Killed -> %d 	%s\n",
					tsk->pid, tsk->name);
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

	return tsk;
}

/* Запуск выполнения задания */
int exec_task(task *tsk)
{
	return exec_cmd(tsk->first);
}

/* Создание очереди на исполнение */
sing_exec *create_exec_queue(task *tsk)
{
	sing_exec *ex, *past, *next;
	list *tmp;

	int i = 0;

	if (arg_list == UNINIT) return NULL;
	if (list_empty(get_head(arg_list))) return NULL;

	/* Первое прохождение */
	list_for_each(tmp, get_head(arg_list)) {
		if (is_same(tmp,"&&")) add_to_queue(SPEC_AND,&(tsk->sp_queue));
		if (is_same(tmp,"||")) add_to_queue(SPEC_OR,&(tsk->sp_queue));
	}

	/* Образование самого первого процесса в очереди процессов */
	ex = (sing_exec *) malloc(sizeof(sing_exec));
	ex->mode = 0;
	ex->name = _STR_DUP((char *) list_get(0,arg_list));
	prepare_args(0,ex,FOR_BACKGR,arg_list);
	ex -> file = (char *) prepare_args(0, ex , FOR_IO ,arg_list);	
	ex -> argv = (char **) prepare_args(0, ex , FOR_ARGS ,arg_list);
	ex -> handler = is_shell_cmd(ex->name);			/* Проверяем, встроена ли функция в оболочку */
	ex -> tsk = tsk;								/* Указываем на принадлежность к заданию */
	ex -> next = NULL;

	if(!queue_empty(tsk->sp_queue)) {				/* Учавствует более одного процесса */
		past = ex;
		i = 0;
		while((i = find_spec(i,arg_list))) {
			next = (sing_exec *) malloc(sizeof(sing_exec));
			next -> mode = 0;
			next ->	name = _STR_DUP((char *) list_get(i,arg_list));
			prepare_args(i,ex,FOR_BACKGR,arg_list);
			next -> file = (char *)  prepare_args(i, ex , FOR_IO, arg_list);	
			next -> argv = (char **) prepare_args(i, ex,  FOR_ARGS, arg_list);
			next->handler = is_shell_cmd(next->name);	/* Проверяем, встроена ли функция в оболочку */
			next -> tsk = tsk;							/* Указываем на принадлежность к заданию */
			next -> next = NULL;
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
	list_del(sh_jobs);
	list_del(bg_jobs);
}