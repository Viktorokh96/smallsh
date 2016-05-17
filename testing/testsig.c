#include <signal.h>
#include <stdio.h>

void sig_handler(int signo)
{
	printf("RESSIVED -> %s\t\tCODE ->%d\n", sys_siglist[signo], signo);
}

int main(int argc, char const *argv[])
{
	signal(SIGINT, &sig_handler);
	signal(SIGTSTP, &sig_handler);
	signal(SIGHUP, &sig_handler);
	signal(SIGUSR1, &sig_handler);
	signal(SIGUSR2, &sig_handler);
	signal(SIGABRT, &sig_handler);
	signal(SIGALRM, &sig_handler);
	signal(SIGFPE, &sig_handler);
	signal(SIGILL, &sig_handler);
	signal(SIGTERM, &sig_handler);
	signal(SIGCONT, &sig_handler);
	signal(SIGCHLD, &sig_handler);
	signal(SIGBUS, &sig_handler);
	signal(SIGQUIT, &sig_handler);
	signal(SIGTTIN, &sig_handler);
	signal(SIGTTOU, &sig_handler);

	for (;;) ;

	return 0;
}
