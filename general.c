#include "general.h"

int init_general() 
{
	path_list = init_list();
	
#ifdef	__USE_GNU
	curr_path = get_current_dir_name (void);
#endif
#if (defined __USE_XOPEN_EXTENDED && !defined __USE_XOPEN2K8) \
    || defined __USE_BSD
	getwd (curr_path);
#else
	getcwd (curr_path,PATHSIZE);
#endif
	list_add(curr_path,strlen(curr_path)+1,path_list);
	shell_pid = getpid();
	home_path = getenv("HOME");

	return 0;
}

void del_general()
{
	list_del(arg_list);
	list_del(path_list);
}