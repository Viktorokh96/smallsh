#include "parse.h"
#include <unistd.h>
#include <string.h>

#include <stdio.h>

#define DIR_SEP		"/"

#define look_further(p)	while((*(p)) && (*(p)) != '\n' && (*(p)) == ' ') ((p)++)
#define select_word(p)  while(((*(p)) != ' ') && ((*(p)) != '\n') && (*(p))) (p)++;	
#define select_path(p)  while(((*(p)) != '\0') && ((*(p)) != ':') && ((*(p)) != '\n') && (*(p))) (p)++;	

/* Разделяем команду на части, выделяя исполняемую часть и аргументы */
unsigned parse_cmd(char *cmd, list_id arg_list)
{
	unsigned mode;
	char *tmp;
	char *p,*q;

	mode = 0;

	for (p = q = cmd; (*p != '\n') && (*p != '\0');) {
		look_further(p);	/* Пропускаем пробелы если есть */
		if(*p) {
			q = p;
			/* Выделяем путь к исполняемому файлу */
			select_word(q);
			*q = 0;
			/* Проверка на & */
			if (!compare_str(p,"&")) {
				if(*(q+1)) p = q+1;
				else p = q; 
				set_bit(mode,INCL_BACKGR);
				continue;
			} 
			tmp = strdup(p);
			list_add_tail(tmp,strlen(tmp)+1,arg_list);
			bit_seted(mode,INCL_EXEC) ? set_bit(mode,INCL_ARGS):set_bit(mode,INCL_EXEC);
			if(*(q+1)) p = q+1;
			else p = q;
		} 
	}	

	return mode;
}

/* Функция, разбивающая строку на пути поиска исполняемых файлов */
char *make_exec_path(char **path, char *execf)
{
	char *p = *path;
	char *q = p;

	if(*q == '\0') return NULL;

	select_path(q);
	*q = '\0'; q++;
	*path = strdup(p);		/* Теперь работает с независимой копией */
	strncat(*path, DIR_SEP, 1);
	strncat(*path, execf, strlen(execf));
	return q;
}

int compare_str(char *str1, char *str2)
{
	char *p = str1;
	char *q = str2;

	if ((p == NULL) || (q == NULL)) { 
		fprintf(stderr, "Передача NULL строки в функцию compare_str! \
			Фатальная ошибка!\n");
		return 1;
	}

	for(;(*p) && (*q);p++,q++) if (*p != *q) return 1;

	return 0;
}