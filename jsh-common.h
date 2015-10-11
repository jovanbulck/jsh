/* This file is part of jsh.
 * 
 * jsh: A basic UNIX shell implementation in C
 * Copyright (C) 2014 Jo Van Bulck <jo.vanbulck@student.kuleuven.be>
 *
 * jsh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * jsh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with jsh.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JSH_COMMON_H_INCLUDED
#define JSH_COMMON_H_INCLUDED
/* ^^ these are the include guards */

// ########## common includes ##########
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <assert.h>
#include <stdbool.h>

// ########## common macro definitions #########
#define ASSERT                  true    // whether or not to include the assert statements in the pre-compilation phase
#define MAX_FILE_LINE_LENGTH    200     // the max nb of chars per line in a file to parse

#define REDIRECT_STR(fd1, fd2) \
    if (dup2(fd1, fd2) < 0) { \
        printerrno("Redirecting stream %d to %d failed", fd2, fd1); \
        exit(EXIT_FAILURE); \
    }

#define CHK_ERR(cond, cmd) \
    if (cond == -1) { \
    printerrno("%s", cmd); \
    return EXIT_FAILURE; \
}

// hackhackhack: '\n' seems to be needed in tty
#define RESTORE_COLOR(stream) \
    if (IS_INTERACTIVE && COLOR) \
        fprintf(stream, "%s\n", NONE); \
    else \
        fprintf(stream, "\n"); \
        
/*
 * A function that prints the provided '\0' terminated string verbatim on stdout, without 
 *  appending a newline '\n' character.
 * @note: inline function implementations should be in header
 *  see (http://stackoverflow.com/questions/26503235/c-inline-funtion-and-gcc)
 */
static inline int puts_verbatim(const char *s) {
    return fputs(s, stdout);
}

// ######### linux tty color codes ########
#define RESET		        0
#define BRIGHT 		        1
#define DIM		            2
#define UNDERLINE 	        3
#define BLINK		        4
#define REVERSE		        7
#define HIDDEN		        8

#define BLACK 		        0
#define RED		            1
#define GREEN		        2
#define YELLOW		        3
#define BLUE		        4
#define MAGENTA		        5
#define CYAN		        6
#define	WHITE		        7

#define NONE                "\033[0m"       // to flush the previous property

// common global variables
extern bool DEBUG;
extern bool COLOR;
extern bool I_AM_FORK;               // whether or not the current process is a fork, i.e. child process
extern bool IS_INTERACTIVE;
extern bool WAITING_FOR_CHILD;

// common function definitions
void printerr(const char*, ...);
void printerrno(const char *format, ...);
void printdebug(const char*, ...);
void printinfo(const char*, ...);
void textcolor(FILE*, int, int);

void parsefile(char*, void (*f)(char*), bool);
void parsestream(FILE*, char*, void (*f)(char*));

int string_cmp(const void*, const void*);
bool is_sorted(void*, size_t, size_t, int (*compar)(const void *, const void *));
char *gethome();
char *strclone(const char*);
char* concat(int, ...);
// TODO malloc wrapper -- gracious

#endif //JSH_COMMON_H_INCLUDED
