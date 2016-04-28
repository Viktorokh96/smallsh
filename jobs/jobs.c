#include "jobs.h"
#include <string.h>
#include "handlers.h"
#include "../parses/parse.h"
#include "../signal/signal.h"
#include <wait.h>
#include <errno.h>

#define is_same(l,s)	(!compare_str((char *) list_entry((l)), s))

 
int queue_next (int p)
{
	if (p < SPEC_LENG-1) return p+1;
	else return 0;
}

struct special_queue {	/* Очередь специальных символов (в виде колцевого буфера) */
	int queue[SPEC_LENG];
	int prod;			/* Указатель на новую позицию для добавления в очередь */
	int cons;			/* Указатель на позицию для следующего чтения из очереди */
	int (*next)(int p);	/* Метод, возвращающий следущий элемент */
} sp_queue;


int no_spec()
{
	return (sp_queue.prod == sp_queue.cons);
}

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

/* Добавление специального исмвола в очередь */
void add_spec(int val)
{
	if(sp_queue.next(sp_queue.prod) == sp_queue.cons) return;	/* Очередь полна */
	sp_queue.queue[sp_queue.prod] = val;
	sp_queue.prod = sp_queue.next(sp_queue.prod);
}

/* Взятие специального символа из очереди */
int get_spec()
{
	if(sp_queue.prod == sp_queue.cons) return NO_SPEC;
	int tmp = sp_queue.queue[sp_queue.cons];
	sp_queue.cons = sp_queue.next(sp_queue.cons);
	return tmp;
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
			for(tmp = list_get_header(num,lid); tmp != get_head(lid) &&  
				(!is_same(tmp,"&&")) && (!is_same(tmp,"||"));
				tmp = tmp->mnext) size++;	/* Доходим до специального символа */
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
			for(tmp = list_get_header(num,lid); tmp != get_head(lid) &&  
				(!is_same(tmp,"&&")) && (!is_same(tmp,"||"));
				tmp = tmp->mnext) size++;	/* Доходим до специального символа */
			for (i = num; i < num+size-1; i++) {
				if(!compare_str((char *)list_get(i,lid),"<") &&
			 		list_get(i+1,lid) != NULL) {
					filename = strdup((char *)list_get(i+1,lid));
					list_connect(i-1,i+2,lid);		/* Избавляемся от этих аргументов */
					set_bit(ex->mode,IO_IN);
					return filename;
				}
				if(!compare_str((char *)list_get(i,lid),">") &&
			 		list_get(i+1,lid) != NULL) {
					filename = strdup((char *)list_get(i+1,lid));
					list_connect(i-1,i+2,lid);		/* Избавляемся от этих аргументов */
					set_bit(ex->mode,IO_OUT);
					return filename;
				}
			}
			return NULL;

		case FOR_BACKGR:
			size = 1;
			for(tmp = list_get_header(num,lid); tmp != get_head(lid) &&  
				(!is_same(tmp,"&&")) && (!is_same(tmp,"||"));
				tmp = tmp->mnext) size++;	/* Доходим до специального символа */
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

int try_exec(char *path, sing_exec *ex) 
{
	int state = 0;
	char *tmp;
	char *exec_path = strdup(path);
	
	while((tmp = make_exec_path(&exec_path,
		ex->name)) != NULL) {
		if (exec_path == NULL) state = 1;
	#ifdef __USE_GNU
		if(!execve(exec_path,ex->argv,environ)) return 0;
		else state = 1;
	#else
		if(!execve(exec_path,ex->argv,__environ)) return 0;
		else state = 1;
	#endif
		exec_path = tmp;
	}
	return state;
}

 
void exec_next(sing_exec *ex, int stat)
{
	int spec;
	if (ex->next != NULL) {
		spec = get_spec();
		if(spec == NO_SPEC) { exec(ex->next); return; }
		if (((stat == 0) && (spec == SPEC_AND)) ||
			((stat != 0) && (spec == SPEC_OR)))
			exec(ex->next);
			else free_exec(ex->next);	/* Освобождаем ненужные элементы */ 
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
int exec (sing_exec *ex)
{
	int stat;

	if(ex == NULL) return 1;								/* Какой-то процесс не был сформирован */
		
	if (ex->handler != NULL) {
		stat = ex->handler(ex->argv);
	} else {
		ex->pid = fork();
		if (ex->pid == 0) { 		/* Дочерний процесс */
			switch_io(ex);
			/*if (bit_seted(ex->mode,RUN_BACKGR)) 
			else set_int_dfl(); */
			set_int_ignore();
			if((stat = try_exec(getenv("PATH"),ex)) != 0) 
			if((stat = try_exec(getenv("PWD"),ex)) != 0) {
				printf("%s: %s <- исполняемый файл не найден.\n",shell_name,ex->name);
				_exit(stat);
			}
		}
		else {						/* Родитель (оболочка) */ 
			current = *ex;			/* Установка текущего процесса */
			if (bit_seted(ex->mode,RUN_BACKGR)) {
				add_bg_job(ex,TSK_RUNNING);
				printf("+1 background -> %d\n", ex->pid );
				current.pid = 0;	/* Фоновый процесс не является текущим */
			} else { 
				/* Ожидаем завершение выполнения текущего процесса */
				wait_child(ex);
			}
		}
	}

	exec_next(ex,stat);
	current.pid = 0;
	free(ex);						/* Процесс отработал своё и больше не нужен */
	return (WIFEXITED(stat)) ? WEXITSTATUS(stat) : -1;
}


int wait_child(sing_exec *ex)
{
	int child_stat;

	waitpid(ex->pid,&child_stat,WUNTRACED);
	
	/*if(ch_pid == -1) {
		perror("waitpid: ");
	} */
	/* if (WTERMSIG(stat) == SIGHUP) exec(ex); Нужно проверять свободен ли терминал в момент вывода */
	/*if (WIFSIGNALED (child_stat))				
		printf ("%s: process %d \t %s \t killed by signal :> %s%s\n", shell_name, ex->pid,
			ex->name,sys_siglist[WTERMSIG (child_stat)],
			(WCOREDUMP(child_stat)) ? " (dumped core)" : "");
	*/
	if (WIFSTOPPED (child_stat)) {			/* Процесс был остановлен */
		add_bg_job(ex,TSK_STOPPED);
		printf ("\n%s: process %d \t %s \tstoped by signal :> %s\n", shell_name, ex->pid,
		ex->name, sys_siglist[WSTOPSIG (child_stat)]);
	}

	return child_stat;
}

void update_jobs()
{
	list *tmp, *next;
	sing_exec *tsk;

	for (tmp = get_head(bg_jobs)->mnext; 
	tmp != get_head(bg_jobs);
	tmp = next) { 
		tsk = (sing_exec*) list_entry(tmp); 
		if (tsk->status == TSK_KILLED) {
			next = tmp->mnext;
			list_del_elem(tmp,bg_jobs);
		} else {
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


/* Создание очереди на исполнение */
sing_exec *create_exec_queue()
{
	sing_exec *ex, *past, *next;
	list *tmp;

	int i = 0;

	if (list_empty(get_head(arg_list))) return NULL;

	/* Первое прохождение */
	list_for_each(tmp, get_head(arg_list)) {
		if (is_same(tmp,"&&")) add_spec(SPEC_AND);
		if (is_same(tmp,"||")) add_spec(SPEC_OR);
	}

	/* Образование самого первого процесса в очереди процессов */
	ex = (sing_exec *) malloc(sizeof(sing_exec));
	ex->mode = 0;
	ex->name = strdup((char *) list_get(0,arg_list));
	prepare_args(0,ex,FOR_BACKGR,arg_list);
	ex -> file = (char *) prepare_args(0, ex , FOR_IO ,arg_list);	
	ex -> argv = (char **) prepare_args(0, ex , FOR_ARGS ,arg_list);
	ex->handler = is_shell_cmd(ex->name);			/* Проверяем, встроена ли функция в оболочку */
	ex -> next = NULL;

	if(!no_spec()) {	/* Учавствует более одного процесса */
		past = ex;
		i = 0;
		while((i = find_spec(i,arg_list))) {
			next = (sing_exec *) malloc(sizeof(sing_exec));
			next -> mode = 0;
			next ->	name = strdup((char *) list_get(i,arg_list));
			prepare_args(i,ex,FOR_BACKGR,arg_list);
			next -> file = (char *)  prepare_args(i, ex , FOR_IO, arg_list);	
			next -> argv = (char **) prepare_args(i, ex,  FOR_ARGS, arg_list);
			next->handler = is_shell_cmd(next->name);	/* Проверяем, встроена ли функция в оболочку */
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
	sp_queue.prod = sp_queue.cons = 0;
	sp_queue.next = &queue_next;

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