#ifndef	SIGNAL_H
#define	SIGNAL_H
#include <unistd.h>

#define _SIGSET_T	__sigset_t

void sig_handler(int sig);

void set_int_ignore();

void set_int_dfl();

int init_signals();

#endif
