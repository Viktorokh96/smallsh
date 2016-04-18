#ifndef PARSE_H
#define PARSE_H

#include "../defines.h"
#include "../services/bits.h"
#include "../services/list.h"
#include "../jobs/jobs.h"
#include "../general.h"

unsigned parse_cmd(char *cmd, list_id arg_list);
char *make_exec_path(char **path, char *execf);

#endif