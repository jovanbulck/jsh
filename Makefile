all: jsh-common alias jsh link
	@echo "<<<< All done >>>>"

jsh-common: jsh-common.c jsh-common.h
	gcc -g -c jsh-common.c -o jsh-common.o
alias: alias.c alias.h jsh-common.h
	gcc -g -c alias.c -o alias.o
jsh: jsh-parse.c jsh.c jsh-common.h
	gcc -g -c jsh.c -o jsh.o
link: jsh-common.o jsh.o alias.o
    gcc -g jsh-common.o jsh.o alias.o -o jsh -l readline
