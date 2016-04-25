#include "jobs.h"
#include <string.h>
#include "handlers.h"
#include "../parses/parse.h"
#include <wait.h>
#include <errno.h>

#define is_same(l,s)	(!compare_str((char *) list_entry((l)), s))

static inline 
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

static inline
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
void *prepare_args(int num, sing_exec *ex, unsigned mode)
{
	char **argv;
	char *filename;
	int i, size, strsize;
	list *tmp;

	if (num < 0 || num > list_count(arg_list)) return NULL;

	switch(mode) {
		case FOR_ARGS:
			size = 1;
			for(tmp = list_get_header(num,arg_list); tmp != get_head(arg_list) &&  
				(!is_same(tmp,"&&")) && (!is_same(tmp,"||"));
				tmp = tmp->mnext) size++;	/* Доходим до специального символа */
			argv = malloc(size*sizeof(char *));
			for (i = num; i < num+size-1; i++) {
				strsize = strlen((char *)list_get(i,arg_list))+1;
				argv[i-num] = malloc(strsize*sizeof(char));
				memcpy(argv[i-num],(char *)list_get(i,arg_list),strsize);
			}

			/* Список всегда должен завершать NULL */
			argv[i-num] = NULL;

			return argv;

		case FOR_IO:
			size = 1;
			for(tmp = list_get_header(num,arg_list); tmp != get_head(arg_list) &&  
				(!is_same(tmp,"&&")) && (!is_same(tmp,"||"));
				tmp = tmp->mnext) size++;	/* Доходим до специального символа */
			for (i = num; i < num+size-1; i++) {
				if(!compare_str((char *)list_get(i,arg_list),"<") &&
			 		list_get(i+1,arg_list) != NULL) {
					filename = strdup((char *)list_get(i+1,arg_list));
					list_connect(i-1,i+2,arg_list);		/* Избавляемся от этих аргументов */
					set_bit(ex->mode,IO_IN);
					return filename;
				}
				if(!compare_str((char *)list_get(i,arg_list),">") &&
			 		list_get(i+1,arg_list) != NULL) {
					filename = strdup((char *)list_get(i+1,arg_list));
					list_connect(i-1,i+2,arg_list);		/* Избавляемся от этих аргументов */
					set_bit(ex->mode,IO_OUT);
					return filename;
				}
			}
			return NULL;

		case FOR_BACKGR:
			size = 1;
			for(tmp = list_get_header(num,arg_list); tmp != get_head(arg_list) &&  
				(!is_same(tmp,"&&")) && (!is_same(tmp,"||"));
				tmp = tmp->mnext) size++;	/* Доходим до специального символа */
				for (i = num; i < num+size-1; i++) {
					if(!compare_str((char *)list_get(i,arg_list),"&")) {
						set_bit(ex->mode,RUN_BACKGR);
						list_connect(i-1,i+1,arg_list);		/* Избавляемся от этого символа */
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

static inline 
void exec_next(sing_exec *ex, int stat)
{
	int spec;
	if (ex->next != NULL) {
		spec = get_spec();
		if(spec == NO_SPEC) { ex->next->exec_func(ex->next); return; }
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
	pid_t pid;
	int child_stat;
	int (*sh_handler)(void *prm);

	sh_handler = is_shell_cmd(ex->name);			/* Проверяем, встроена ли функция в оболочку */

		
	if (sh_handler != NULL) {
		stat = sh_handler(ex->argv);
	} else {
		pid = fork();
		if (pid == 0) { 
			switch_io(ex);
			if((stat = try_exec(getenv("PATH"),ex)) != 0) 
			if((stat = try_exec(getenv("PWD"),ex)) != 0) {
				printf("%s: %s <- исполняемый файл не найден.\n",shell_name,ex->name);
				_exit(1);
			}
		}
		else {
			if bit_seted(ex->mode,RUN_BACKGR) {
				add_bg_job(ex->name,pid,shell_pid);
				printf("In backgr PID %d 	PPID %d\n",pid,shell_pid);
			} else { /* Нужно ожидать завершения всех фоновых процессов смотри man 2 waitpid */
				while(wait(&child_stat) > 0);
				/* printf ("PROC PID -> %d\n",wait(&child_stat)); */
			}
		}
	}

	exec_next(ex,stat);

	return 0;
}

static inline 
int find_spec(int i)
{
	int p = i;
	list *tmp;
	list_for_each(tmp,list_get_header(i,arg_list)) {
		p++;
		if (list_entry(tmp) == NULL) return 0;
		if (is_same(tmp,"&&"))  return p+1; 
		if (is_same(tmp,"||"))  return p+1; 
	}
	return 0;
}


/* Созданиее очереди на исполнение */
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

	ex = (sing_exec *) malloc(sizeof(sing_exec));
	ex->name = strdup((char *) list_get(0,arg_list));
	prepare_args(0,ex,FOR_BACKGR);
	ex -> file = (char *) prepare_args(0, ex , FOR_IO);	
	ex -> argv = (char **) prepare_args(0, ex , FOR_ARGS);
	ex -> next = NULL;

	if(!no_spec()) {	/* Обнаружена очередь */
		past = ex;
		i = 0;
		while((i = find_spec(i))) {
			next = (sing_exec *) malloc(sizeof(sing_exec));
			next ->	name = strdup((char *) list_get(i,arg_list));
			prepare_args(i,ex,FOR_BACKGR);
			next -> file = (char *)  prepare_args(i, ex , FOR_IO);	
			next -> argv = (char **) prepare_args(i, ex,  FOR_ARGS);
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
}

void del_jobs()
{
	list_del(sh_jobs);
	list_del(bg_jobs);
}