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
	char *tmp;
	char *p,*q;

	/* Удаляем старый список */
	if (arg_list != UNINIT)	list_del(arg_list);

	arg_list = init_list();

	for (p = q = cmd; (*p != '\n') && (*p != '\0');) {
		look_further(p);	/* Пропускаем пробелы если есть */
		if(*p) {
			if (*p == ';') return p+1;
			q = p;
			/* Выделяем путь к исполняемому файлу */
			select_word(q);
			if (*q == ';') {
				*q = 0;
				tmp = strdup(p);
				list_add_tail(tmp,strlen(tmp)+1,arg_list);	
				return q + 1;
			}
			*q = 0;
			tmp = strdup(p);
			list_add_tail(tmp,strlen(tmp)+1,arg_list);
			if(*(q+1)) p = q+1;
			else p = q;
		} 
	}	
	return NULL;
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
	if((*p == '\0') ^ (*q == '\0')) return 1;

	return 0;
}

char *short_path(char *path)
{
	char *buf = strdup(path);
	char *p = buf, *q = home_path;
	for (;(*p == *q) && (*q); p++, q++);
	if (!(*q)) { 
		*(--p) = '~';
		return p;
	}
	else return path;
}