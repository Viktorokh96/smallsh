#include "general.h"

int init_general() 
{
	
#ifdef	__USE_GNU
	curr_path = get_current_dir_name (void);
#endif
#if (defined __USE_XOPEN_EXTENDED && !defined __USE_XOPEN2K8) \
    || defined __USE_BSD
	getwd (curr_path);
#else
	getcwd (curr_path,PATHSIZE);
#endif
	return 0;
}