#include <signal.h>
#include <stdio.h>
#include "signal.h"
#include "../jobs/jobs.h"
#include "../general.h"

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
		if (getpid() != 0 && current.pid) {
			kill (current.pid,SIGTSTP);
		}
	}

	if ( sig == SIGCHLD) {

	}

	return;
}

/* 	Игнорирование и установка по умолчанию
	сигналов наследуется между ветвелниями 
	и вызовами exec. Это полезно! */
void set_int_ignore() 
{
	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);
}

void set_int_dfl()
{
	signal (SIGQUIT, SIG_DFL);
	signal (SIGINT, SIG_DFL);
	signal (SIGTSTP, SIG_DFL);
}

int init_signals()
{
	signal(SIGINT,&sig_handler);
	signal(SIGTSTP,&sig_handler);
	signal(SIGCHLD,&sig_handler);

	return 0;
}