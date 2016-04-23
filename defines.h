#ifndef DEFINES_H
#define DEFINES_H
//#define USE_DEPRICATED 
/* Размер строки выделенную под команды */
#define CMD_SIZE	256
#define STR_SIZE	128
#define SPEC_LENG	128			

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

/* Флаги командного парсера */
#define INCL_EXEC	0x0001 		/* Каманда содержит исполняемый файл */
#define INCL_ARGS	0x0002		/* Каманда содержит аргументы */
#define INCL_PIPE	0x0004		/* Команда содержит перенаправление в/в */
#define INCL_BACKGR	0x0008		/* Команда требует запуска в фоновом режиме */

/* Флаги функции подготовки аргументов */
#define TO_SPEC		0x0001		/* До специального символа */
#define TO_END		0x0002

#define ESCAPING	'\\'

#define DIR_SEP		"/"
#define CH_DIR_SEP	'/'
