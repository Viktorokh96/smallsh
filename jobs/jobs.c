#include "jobs.h"
#include <string.h>
#include "handlers.h"
#include "../parses/parse.h"

static job job_sh; /* Структура для описания обработчика */

#define add_job(n,h) \
				job_sh.name = (n); \
				job_sh.handler = &(h); \
				list_add(&job_sh,sizeof(job),sh_jobs)

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

	add_job("exit",	exit_handl);
	add_job("pwd",	pwd_handl);
	add_job("cd",	cd_handl);
}

void del_jobs()
{
	list_del(sh_jobs);
}