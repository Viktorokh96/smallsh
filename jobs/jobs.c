#include "jobs.h"
#include <string.h>
#include "handlers.h"
#include "../parses/parse.h"
#include <wait.h>

inline int queue_next (int p)
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

inline int no_spec()
{
	return (sp_queue.prod == sp_queue.cons);
}

/* Добавление специального исмвола в очередь */
void add_spec(int val)
{
	if(sp_queue.next(sp_queue.prod) == sp_queue.cons) return;	/* Очередь полна */
	sp_queue.queue[sp_queue.prod] = val;
	sp_queue.prod = sp_queue.next(sp_queue.prod);
}

/* Взятие специального символа из очереди */
int  get_spec()
{
	if(sp_queue.prod == sp_queue.cons) return NO_SPEC;
	int tmp = sp_queue.queue[sp_queue.cons];
	sp_queue.cons = sp_queue.next(sp_queue.cons);
	return tmp;
}

/* Подготавливаем аргументы */
char **prepare_args(list *begin, unsigned mode)
{
	char **argv;
	int i, size, strsize;
	list *tmp;

	switch(mode) {
		case TO_SPEC:
			size = 1;
			for(tmp = begin; compare_str((char *) list_entry(tmp),"&&") && 
				 compare_str((char *) list_entry(tmp),"||");
				tmp = tmp->mnext) size++;	/* Доходим до специального символа */
			argv = malloc(size*sizeof(char *));
			for (i = 0; i < size; i++) {
				strsize = strlen((char *)list_get(i,arg_list))+1;
				argv[i] = malloc(strsize*sizeof(char));
				memcpy(argv[i],(char *)list_get(i,arg_list),strsize);
			}

			/* Список всегда должен завершать NULL */
			argv[i] = NULL;

			return argv;

		case TO_END:
			size = 1;
			for(tmp = begin; (tmp = tmp->mnext) != get_head(arg_list);) size++;	/* Доходим до конца списка */
			argv = malloc(size*sizeof(char *));
			for (i = 0; i < size; i++) {
				strsize = strlen((char *)list_get(i,arg_list))+1;
				argv[i] = malloc(strsize*sizeof(char));
				memcpy(argv[i],(char *)list_get(i,arg_list),strsize);
			}

			/* Список всегда должен завершать NULL */
			argv[i] = NULL;

			return argv;

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

/* Исполнение команды */
int exec (sing_exec *ex)
{
	pid_t pid;
	int child_stat;
	
	pid = fork();
	if (pid == 0) {
		if(try_exec(getenv("PATH"),ex) != 0) 
		if(try_exec(getenv("PWD"),ex) != 0) {
			printf("Исполняемый файл не найден.\n");
			_exit(1);
			
		}
	}
	else {
		wait(&child_stat);
	}

	if(ex->next != NULL) ex->next->exec_func(ex->next);

	return 0;
}

int exec_shells (sing_exec *ex)
{
	int (*sh_handler)(void *prm);

	sh_handler = is_shell_cmd(ex->name);

	sh_handler(ex->argv);

	ex->next->exec_func(ex->next);
}


/* Созданиее очереди на исполнение */
sing_exec *create_exec_queue()
{
	sing_exec *ex, *past, *next;
	list *tmp;

	int i = 0, base = 0;

	if (list_empty(get_head(arg_list))) return NULL;

	/* Первое прохождение */
	list_for_each(tmp, get_head(arg_list)) {
		if (!compare_str((char *) list_entry(tmp), "&&")) add_spec(SPEC_AND);
		if (!compare_str((char *) list_entry(tmp), "||")) add_spec(SPEC_OR);
	}

	ex = (sing_exec *) malloc(sizeof(sing_exec));
	strcpy(ex->name,(char *) list_get(0,arg_list));
	if(no_spec())
		ex -> argv = prepare_args(get_head(arg_list), TO_END);
	else
		ex -> argv = prepare_args(get_head(arg_list), TO_SPEC);
	ex -> files = NULL;	/* Временно */
	ex -> next = NULL;

	ex -> exec_func = (is_shell_cmd(ex->name)) ? &exec_shells : &exec;

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
}

void del_jobs()
{
	list_del(sh_jobs);
	list_del(bg_jobs);
}