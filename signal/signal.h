#ifndef	SIGNAL_H
#define	SIGNAL_H
	#include <unistd.h>
	
	void sig_handler(int sig);

	int init_signals();

#endif