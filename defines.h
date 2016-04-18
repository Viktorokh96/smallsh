#ifndef DEFINES_H
#define DEFINES_H
//#define USE_DEPRICATED 
/* Размер строки выделенную под команды */
#define CMD_SIZE	256

/* Режимы инициализации для init_shell() */
#define SIGNAL		0x0001
#define JOBS		0x0002
#define GENERAL		0x0004
#endif

/* Флаги командного парсера */

#define INCL_EXEC	0x0001 		/* Каманда содержит исполняемый файл */
#define INCL_ARGS	0x0002		/* Каманда содержит аргументы */
#define INCL_PIPE	0x0004		/* Команда содержит перенаправление в/в */
#define INCL_BACKGR	0x0008		/* Команда требует запуска в фоновом режиме */