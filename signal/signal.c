#include <signal.h>
#include <stdio.h>
#include <wait.h>
#include "signal.h"
#include "../jobs/jobs.h"
#include "../general.h"


#define _SIGSET_T	__sigset_t

void set_sig_act(int signo, __sighandler_t hand, int flags, _SIGSET_T *sigset)
{									
	struct sigaction sig_act;			
	sig_act.sa_handler = hand;		
	sig_act.sa_flags = flags;			
	if(sigset != NULL) sig_act.sa_mask = *sigset;
	else sigemptyset(&sig_act.sa_mask);
	sigaction (signo,&sig_act,NULL);	
}			

void sig_handler(int signo)
{

	if (signo == SIGINT) {
		fflush(stdout);
		if(current.pid != 0 && current.pid != getpid()) {	
			kill(current.pid,SIGINT);
			current.status = TSK_KILLED;
		}
		printf("\n");
	}

	if (signo == SIGTSTP) {
		fflush(stdout);
		if(current.pid != 0 && current.pid != getpid()) {	
			kill(current.pid,SIGTSTP);					
			current.status = TSK_STOPPED;
		}
		printf("\n");
	}
}					

/* 	Игнорирование и установка по умолчанию
	сигналов наследуется между ветвелниями 
	и вызовами exec. Это полезно! */

void set_int_dfl()
{
	set_sig_act(SIGQUIT,SIG_DFL,0,NULL);
	set_sig_act(SIGTSTP,SIG_DFL,0,NULL);	
	set_sig_act(SIGINT,SIG_DFL,0,NULL);
	set_sig_act(SIGINT,SIG_DFL,0,NULL);
	set_sig_act(SIGTTIN,SIG_DFL,0,NULL);		/* Требуется "временное разрешение" на ипользование */
	set_sig_act(SIGTTOU,SIG_DFL,0,NULL);		/* терминала для вывода своих данных */
	set_sig_act(SIGHUP,SIG_DFL,0,NULL);
	set_sig_act(SIGTERM,SIG_DFL,0,NULL);
}


int init_signals()
{
	_SIGSET_T	sigset;
	sigemptyset(&sigset);
	sigaddset (&sigset, SIGINT);
	sigaddset (&sigset, SIGTSTP);

	set_sig_act(SIGINT,&sig_handler,SA_RESTART, &sigset );
	set_sig_act(SIGTSTP,&sig_handler,SA_RESTART, &sigset);
	set_sig_act(SIGCHLD,SIG_DFL,SA_RESTART | SA_NOCLDSTOP, NULL);
	set_sig_act(SIGQUIT,SIG_IGN,0, NULL);

	return 0;
}