# Имя компилятора
CC = cc 
# Указание компилятору на поиск заголовочных файлов
INCLUDE = -I/usr/include  

LIBS = 

# Направление поиска для make
vpath %.c ./services ./parses ./jobs ./signal
# Установка флагов для компиляции объектных файлов
FLAGS = -Wall -Wno-deprecated-declarations -c -g -std=gnu99 $(INCLUDE)
# Установка флагов для линковщика
LDFLAGS = $(LIBS)
# Исходные файлы проекта
GENERALSRC = general.c
MAINSOURCE = shell.c 
PARSESRC = parse.c
LISTSRC = list.c
JOBSSRC = jobs.c
SIGNALSRC = signal.c
# Объектные файлы 
MAINOBJ = $(patsubst %.c,%.o,$(MAINSOURCE))
PARSEOBJ = $(patsubst %.c,%.o,$(PARSESRC))
LISTOBJ = $(patsubst %.c,%.o,$(LISTSRC))
JOBSOBJ = $(patsubst %.c,%.o,$(JOBSSRC))
GENERALOBJ = $(patsubst %.c,%.o,$(GENERALSRC))
SIGNALOBJ = $(patsubst %.c,%.o,$(SIGNALSRC))

ALLOBJ	= $(SIGNALOBJ) $(GENERALOBJ) $(JOBSOBJ) $(LISTOBJ) $(PARSEOBJ) $(MAINOBJ)

HOMEPATH = $(HOME)/bin/

EXE := smallsh

all: $(EXE)

$(EXE) : $(ALLOBJ)
		$(CC) $^ $(LDFLAGS) -o $@
		cp $@ $(HOMEPATH) || cp $@ /usr/bin || cp $@ /usr/local/bin

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

$(SIGNALOBJ) : $(SIGNALSRC)
		$(CC) $(FLAGS) $^ $(CFLAGS) -o $@				
		
clean : 
		rm -f $(ALLOBJ) | rm $(EXE)
