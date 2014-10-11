UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin) # Add library folder for Mac OS X readline (installed with homebrew)
	LINK := gcc -g jsh-common.o jsh.o alias.o -o jsh -L/usr/local/lib/ -lreadline
else
	LINK := gcc -g jsh-common.o jsh.o alias.o -o jsh -lreadline -lncurses
endif

all: jsh-common alias jsh link
	@echo "<<<< All done >>>>"

jsh-common: jsh-common.c jsh-common.h
	gcc -g -c jsh-common.c -o jsh-common.o
alias: alias.c alias.h jsh-common.h
	gcc -g -c alias.c -o alias.o
jsh: jsh-parse.c jsh.c jsh-common.h
	gcc -g -c jsh.c -o jsh.o
link: jsh-common.o jsh.o alias.o
	$(LINK)