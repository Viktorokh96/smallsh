# Имя компилятора
CC = cc 
# Указание компилятору на поиск заголовочных файлов
INCLUDE = -I/usr/include  

LIBS = 

# Направление поиска для make
vpath %.c ./services ./parses ./jobs
# Установка флагов для компиляции объектных файлов
FLAGS = -Wall -c -g $(INCLUDE)
# Установка флагов для линковщика
LDFLAGS = $(LIBS)
# Исходные файлы проекта
GENERALSRC = general.c
MAINSOURCE = init.c 
PARSESRC = parse.c
LISTSRC = list.c
JOBSSRC = jobs.c
# Объектные файлы 
MAINOBJ = $(patsubst %.c,%.o,$(MAINSOURCE))
PARSEOBJ = $(patsubst %.c,%.o,$(PARSESRC))
LISTOBJ = $(patsubst %.c,%.o,$(LISTSRC))
JOBSOBJ = $(patsubst %.c,%.o,$(JOBSSRC))
GENERALOBJ = $(patsubst %.c,%.o,$(GENERALSRC))

EXE := smallsh

all: $(EXE)

$(EXE) : $(GENERALOBJ) $(JOBSOBJ) $(LISTOBJ) $(PARSEOBJ) $(MAINOBJ)
		$(CC) $^ $(LDFLAGS) -o $@

$(MAINOBJ) : $(MAINSOURCE)
		$(CC) $(FLAGS) $^ $(CFLAGS) -o $@

$(PARSEOBJ) : $(PARSESRC)
		$(CC) $(FLAGS) $^ $(CFLAGS) -o $@

$(LISTOBJ) : $(LISTSRC)
		$(CC) $(FLAGS) $^ $(CFLAGS) -o $@				

$(JOBSOBJ) : $(JOBSSRC)
		$(CC) $(FLAGS) $^ $(CFLAGS) -o $@				

$(GENERALOBJ) : $(GENERALSRC)
		$(CC) $(FLAGS) $^ $(CFLAGS) -o $@				

clean : 
		rm -f $(GENERALOBJ) $(JOBSOBJ) $(MAINOBJ) $(PARSEOBJ)  | rm $(EXE)
