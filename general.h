#ifndef GENERAL_H
#define GENERAL_H
#include <unistd.h>
#include <stdlib.h>
#include "./services/table.h"
#include "./defines.h"
#include <sys/types.h>
#include <linux/limits.h>
#include <termios.h>

#ifdef	__USE_GNU
#define _GETWD(p)	get_current_dir_name (void);
#elif (defined __USE_XOPEN_EXTENDED && !defined __USE_XOPEN2K8) \
    || defined __USE_BSD
#define _GETWD(p)	getwd ((p));
#else
#define _GETWD(p) 	getcwd ((p),PATH_MAX);
#endif

#define PATHSIZE	64

	/* current working directory */
char *curr_path;

	/* full home path */
char *home_path;

	/* Username */
char *user_name;

	/* table of path history */
addr_table past_path;

	/* shell group id */
pid_t shell_pgid;

	/* name of shell */
char *shell_name;

	/* shell terminal mode descriptor */
struct termios shell_tmodes;

    /* fd of shell's terminal */
int sh_terminal;

    /* working mode */
int sh_is_interactive;

	/* To get current directory */
char *get_curr_path(char *path);

	/* Initialization of global data structures */
int init_general();

void del_general();

#endif
