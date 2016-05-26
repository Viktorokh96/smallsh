#include "parse.h"
#include <unistd.h>
#include <string.h>

#include <stdio.h>
#define look_further(p)	while((*(p)) && (*(p)) != ';' && (*(p)) != '\n' && (*(p)) == ' ') ((p)++)
#define select_word(p)  while(((*(p)) != ' ') && (*(p)) != ';' && ((*(p)) != '\n') && (*(p))) (p)++;
#define select_path(p)  while(((*(p)) != '\0') && ((*(p)) != ':') && ((*(p)) != '\n') && (*(p))) (p)++;

/* Разделяем команду на части, выделяя исполняемую часть и аргументы */
char *parse_cmd(char *cmd)
{
	unsigned char next = 0;
	char *tmp;
	char *p, *q;

	/* Удаляем старую таблицу */
	while (arg_vec.elem_quant != 0)
		table_del(0, &arg_vec);

	if (cmd == NULL)
		return NULL;

	for (p = q = cmd; (*p != '\n') && (*p != '\0');) {
		look_further(p);	/* Пропускаем пробелы если есть */
		if (*p) {
			if (*p == ';')
				if (p - 1 < cmd || *(p - 1) != ESCAPING)
					return p + 1;
			q = p;
			/* Выделяем путь к исполняемому файлу */
			select_word(q);	/* Выделяется лексема, разделённая пробелами */
			if (*q == ';') {
				if (q - 1 < cmd || *(q - 1) != ESCAPING) {
					*q = 0;
					tmp = _STR_DUP(p);
					table_add(tmp, &arg_vec);
					return q + 1;
				} else {
					p += 1;
					q += 1;
				}
			}
			if(*q) next = 1;
			else next = 0;
			*q = 0;
			tmp = _STR_DUP(p);
			table_add(tmp, &arg_vec);
			if (next && *(q + 1))
				p = q + 1;
			else
				p = q;
		}
	}
	return NULL;
}

/* Функция, разбивающая строку на пути поиска исполняемых файлов */
char *find_exec(char **path, char *execf)
{
	char *p = *path;
	char *q = p;

	if (*q == '\0')
		return NULL;

	select_path(q);
	*q = '\0';
	q++;
	*path = _STR_DUP(p);	/* Теперь работает с независимой копией */
	strncat(*path, DIR_SEP, 1);
	strncat(*path, execf, strlen(execf));
	return q;
}

char *short_path(char *path)
{
	char *buf = _STR_DUP(path);
	char *p = buf, *q = home_path;
	for (; (*p == *q) && (*q); p++, q++) ;
	if (!(*q)) {
		*(--p) = CH_HOME;
		return p;
	} else
		return path;
}

/* Трансофрмация входящего пути в полный путь */
char *full_path(char *path)
{
	if (path == NULL)
		return NULL;
	char *p = _STR_DUP(path);
	char *full_path = p;

	for (; (*p) && (*p != CH_HOME); p++) ;
	if (*p) {		/* Если обнаружили значок домашнего */
		p++;		/* каталога то дополняем его полным путем  */
		char *q = _STR_DUP(home_path);
		strncat(q, p, strlen(p));
		full_path = q;
		free(p - 1);
	}

	return full_path;
}
