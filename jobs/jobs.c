#include "jobs.h"
#include <string.h>
#include "handlers.h"
#include "../parses/parse.h"
#include <wait.h>

#define is_spec(l,s)	(!compare_str((char *) list_entry((l)), s))

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

void free_exec(sing_exec *ex)
{
	int i;
	if (ex != NULL) {
		if(ex->name != NULL) free(ex->name);
		if(ex->argv != NULL) {
			for (i = 0; ex->argv[i] != NULL; free(ex->argv[i]), i++); 
			free(ex->argv);		
		}
		if(ex->files != NULL) free(ex->files);
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
char **prepare_args(int num, unsigned mode)
{
	char **argv;
	int i, size, strsize;
	list *tmp;

	if (num < 0 || num > list_count(arg_list)) return NULL;

	switch(mode) {
		case TO_SPEC:
			size = 1;
			for(tmp = list_get_header(num,arg_list); tmp != get_head(arg_list) &&  
				(!is_spec(tmp,"&&")) && (!is_spec(tmp,"||"));
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

inline void exec_next(sing_exec *ex, int stat)
{
	int spec;
	if (ex->next != NULL) {
		spec = get_spec();
		if(spec == NO_SPEC) { ex->next->exec_func(ex->next); return; }
		if (((stat == 0) && (spec == SPEC_AND)) ||
			((stat != 0) && (spec == SPEC_OR)))
			ex->next->exec_func(ex->next);
			else free_exec(ex->next);	/* Освобождаем ненужные элементы */ 
	}
}

/* Исполнение команды */
int exec (sing_exec *ex)
{
	int stat;
	pid_t pid;
	int child_stat;
	
	pid = fork();
	if (pid == 0) {
		if((stat = try_exec(getenv("PATH"),ex)) != 0) 
		if((stat = try_exec(getenv("PWD"),ex)) != 0) {
			printf("%s: %s <- исполняемый файл не найден.\n",shell_name,ex->name);
			_exit(1);
			
		}
	}
	else {
		wait(&child_stat);
	}

	exec_next(ex,stat);

	return 0;
}

/* Запуск встроенных команд */
int exec_shells (sing_exec *ex)
{
	int stat;

	int (*sh_handler)(void *prm);

	sh_handler = is_shell_cmd(ex->name);

	stat = sh_handler(ex->argv);

	exec_next(ex,stat);

	return 0;
}

inline int find_spec(int i)
{
	int p = i;
	list *tmp;
	list_for_each(tmp,list_get_header(i,arg_list)) {
		p++;
		if (list_entry(tmp) == NULL) return 0;
		if (is_spec(tmp,"&&"))  return p+1; 
		if (is_spec(tmp,"||"))  return p+1; 
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
		if (is_spec(tmp,"&&")) add_spec(SPEC_AND);
		if (is_spec(tmp,"||")) add_spec(SPEC_OR);
	}

	ex = (sing_exec *) malloc(sizeof(sing_exec));
	ex->name = strdup((char *) list_get(0,arg_list));
	ex -> argv = prepare_args(0, TO_SPEC);
	ex -> files = NULL;	/* Временно */
	ex -> next = NULL;
	ex -> exec_func = (is_shell_cmd(ex->name)) ? &exec_shells : &exec;

	if(!no_spec()) {	/* Обнаружена очередь */
		past = ex;
		i = 0;
		while((i = find_spec(i))) {
			next = (sing_exec *) malloc(sizeof(sing_exec));
			next ->	name = strdup((char *) list_get(i,arg_list));
			next -> argv = prepare_args(i, TO_SPEC);
			next -> files = NULL;	/* Временно */
			next -> next = NULL;
			next -> exec_func = (is_shell_cmd(next->name)) ? &exec_shells : &exec;		
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
}

void del_jobs()
{
	list_del(sh_jobs);
	list_del(bg_jobs);
}