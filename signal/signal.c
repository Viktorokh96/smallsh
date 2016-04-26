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
		printf("\nGETPID -> %d\n",getpid());
		if (getpid() != 0) {
			printf("\n");
			fflush(stdout);
		}
	}

	if (sig == SIGTSTP) {
		if (getpid() != 0) {

		}
	}

	return;
}


int init_signals()
{
	signal(SIGINT,&sig_handler);
	//signal(SIGTSTP,&sig_handler);

	return 0;
}