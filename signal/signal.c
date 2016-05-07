#include <signal.h>
#include <stdio.h>
#include <wait.h>
#include "signal.h"
#include "../jobs/jobs.h"
#include "../general.h"

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
	pid_t pgid = tcgetpgrp(sh_terminal);

	if (signo == SIGINT) {
		fflush(stdout);

		if(pgid != 0 && pgid != getpid()) {	
			kill(-pgid,SIGINT);
		}
		printf("\n");
	}

	if (signo == SIGTSTP) {
		fflush(stdout);
		if(pgid != 0 && pgid != getpid()) {	
			kill(-pgid,SIGTSTP);					
		}
		printf("\n");
	}
}

void sigch_handler(int signo, siginfo_t *si, void *ucontext)
{

	if(signo == SIGCHLD) {
		list *tmp;
		sing_exec *ex;
		list_for_each(tmp,get_head(bg_jobs)) {
			if ((ex = have_ex(((task *) list_entry(tmp)),si->si_pid)) != NULL) { 	/* Такой процесс существует в списке фоновых */
				waitpid(ex->pid,NULL,WNOHANG);
				exec_next(ex,si->si_status);
			}
		}
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
	sigaddset (&sigset, SIGQUIT);

	set_sig_act(SIGINT,&sig_handler,SA_RESTART, &sigset );
	set_sig_act(SIGTSTP,&sig_handler,SA_RESTART, &sigset);
	set_sig_act(SIGCHLD,&sigch_handler, SA_SIGINFO | SA_RESTART | SA_NODEFER, &sigset);
	set_sig_act(SIGQUIT,SIG_IGN,0, NULL);

	return 0;
}