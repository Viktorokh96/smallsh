#ifndef PARSE_H
#define PARSE_H

#include "../defines.h"
#include "../services/bits.h"
#include "../services/list.h"
#include "../jobs/jobs.h"
#include "../general.h"

/* Разделяем команду на части, выделяя исполняемую часть и аргументы */
unsigned parse_cmd(char *cmd, list_id arg_list);

/* Функция, разбивающая строку на пути поиска исполняемых файлов */
char *make_exec_path(char **path, char *execf);

/* Она медленнее, но должна быть стабильнее */
int compare_str(char *str1, char *str2);

#endif