#include <signal.h>
#include <stdio.h>
#include <wait.h>
#include "signal.h"
#include "../jobs/jobs.h"
#include "../general.h"


#define _SIGSET_T	__sigset_t

/* Установка обработчика сигнала 
#define	set_sig_act(signo,hand,mask,flags)	\
	do {									\
		sig_act.sa_handler = (hand);		\
		sig_act.sa_mask = (_SIGSET_T)(mask);\
		sig_act.sa_flags = (flags);			\
		sigaction ((signo),&sig_act,NULL);	\
	} while(0);		*/

void set_sig_act(int signo, __sighandler_t hand, int flags)
{									
	struct sigaction sig_act;			
	sig_act.sa_handler = hand;		
	sig_act.sa_flags = flags;			
	sigaction (signo,&sig_act,NULL);	
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
	set_sig_act(SIGINT,SIG_IGN,0);
	set_sig_act(SIGTSTP,SIG_IGN,0);
	set_sig_act(SIGCHLD,SIG_IGN,0);

	return 0;
}