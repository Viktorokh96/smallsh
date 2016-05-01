#ifndef PARSE_H
#define PARSE_H

#include "../defines.h"
#include "../services/bits.h"
#include "../services/list.h"
#include "../jobs/jobs.h"
#include "../general.h"

#if defined  _SVID_SOURCE || _BSD_SOURCE || _XOPEN_SOURCE >= 500 \
|| _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED ||  _POSIX_C_SOURCE >= 200809L
	#define _STR_DUP(s)		strdup((s))
#elif defined  _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700
	#define _STR_DUP(s)		strndup((s),strlen(s)+1)
#elif _GNU_SOURCE
	#define _STR_DUP(s)		strndupa((s))
#endif


/* Разделяем команду на части, выделяя исполняемую часть и аргументы */
char *parse_cmd(char *cmd);

/* Функция, разбивающая строку на пути поиска исполняемых файлов */
char *find_exec(char **path, char *execf);

/* Она медленнее, но должна быть стабильнее */
int compare_str(char *str1, char *str2);

/* Обрезка полного пути для удобства */
char *short_path(char *path);

char *full_path(char *path);

#endif