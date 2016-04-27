#include <signal.h>
#include <stdio.h>
#include "signal.h"
#include "../jobs/jobs.h"

void show_stoped_job()
{
	printf("\n");
}

/* Определение функции обработчика */
void sig_handler(int sig)
{
	if (sig == SIGINT) {
		if (getpid() != 0) {
			fflush(stdout);
		}
	}

	if (sig == SIGTSTP) {
		if (getpid() != 0) {

		}
	}

	if ( sig == SIGCHLD) {

	}

	return;
}

/* 	Игнорирование сигналов наследуется между ветвелниями 
	и вызовами exec, это полезно */
void set_int_ignore() 
{
	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);
}

int init_signals()
{
	signal(SIGINT,&sig_handler);
	signal(SIGCHLD,&sig_handler);

	return 0;
}