#include <signal.h>
#include <stdio.h>
#include <wait.h>
#include "signal.h"
#include "../jobs/jobs.h"
#include "../general.h"


#define _SIGSET_T	__sigset_t

void set_sig_act(int signo, __sighandler_t hand, int flags)
{									
	struct sigaction sig_act;			
	sig_act.sa_handler = hand;		
	sig_act.sa_flags = flags;			
	sigemptyset(&sig_act.sa_mask);
	sigaction (signo,&sig_act,NULL);	
}			

void sig_handler(int signo)
{
	if (signo == SIGINT) {
		fflush(stdout);
		if(current.pid != 0 && current.pid != getpid()) {	
			kill(current.pid,SIGKILL);						/* Не вижу иного выхода... :( */
			current.status = TSK_KILLED;
		}
		printf("\n");
	}

	if (signo == SIGTSTP) {
		fflush(stdout);
		if(current.pid != 0 && current.pid != getpid()) {	
			kill(current.pid,SIGSTOP);						/* И здесь тоже... :( */
			current.status = TSK_STOPPED;
		}
		printf("\n");
	}
}					

/* 	Игнорирование и установка по умолчанию
	сигналов наследуется между ветвелниями 
	и вызовами exec. Это полезно! */
void set_int_ignore() 
{
	set_sig_act(SIGINT,SIG_IGN,0);
	set_sig_act(SIGQUIT,SIG_IGN,0);
	set_sig_act(SIGTSTP,SIG_IGN,0);	
	set_sig_act(SIGQUIT,SIG_IGN,0);
}


void set_int_dfl()
{
	set_sig_act(SIGQUIT,SIG_DFL,0);
	set_sig_act(SIGTSTP,SIG_DFL,0);	
	set_sig_act(SIGINT,SIG_DFL,0);
}


int init_signals()
{
	set_sig_act(SIGINT,&sig_handler,SA_RESTART);
	set_sig_act(SIGTSTP,&sig_handler,SA_RESTART);
	set_sig_act(SIGCHLD,SIG_IGN,0);

	return 0;
}