#ifndef	SIGNAL_H
#define	SIGNAL_H
	#include <unistd.h>
	
	void sig_handler(int sig);

	void set_int_ignore();

	int init_signals();

#endif