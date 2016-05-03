#ifndef DEFINES_H
#define DEFINES_H
//#define USE_DEPRICATED 
/* Размер строки выделенную под команды */
#define CMD_SIZE	256
#define STR_SIZE	128
#define Q_LENG		128			

/* Возможные специальные условия выполнения */
#define NO_SPEC		0x0000
#define SPEC_AND	0x0001		/* && */
#define SPEC_OR		0x0002		/* || */

/* Режимы инициализации для init_shell() */
#define SIGNAL		0x0001
#define JOBS		0x0002
#define GENERAL		0x0004
#define LIST		0x0008
#endif

/* Режимы выполнения каманды */
#define IO_IN		0x0001		/* Каманда содержит перенаправление из файла */
#define IO_OUT		0x0002		/* Команда содержит перенаправление в файл */

/* Флаги функции подготовки аргументов */
#define FOR_ARGS		1			/* Подготовить список аргументов */
#define FOR_IO			2			/* Проверять перенаправление ввода вывода */ 
#define FOR_BACKGR		3			/* Проверять запуск в фоновом режиме */

#define ESCAPING	'\\'

#define DIR_SEP		"/"
#define CH_DIR_SEP	'/'
#define CH_HOME		'~'