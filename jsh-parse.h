/* This file is part of jsh.
 * 
 * jsh (jo-shell): A basic UNIX shell implementation in C
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

#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED
/* ^^ these are the include guards */

#include "jsh-common.h"
#include "alias.h"

struct comd {
    char **cmd;         // NULL-terminated array of pointers to the command's name and its arguments
    int length;         // the length of the **cmd array: cmd[length] = NULL
    char *inf;          // name of the file for redirecting stdin or NULL
    char *outf;         // name of the file for redirecting stdout or NULL
    char *errf;         // name of the file for redirecting stderr or NULL
    int append_out;     // whether or not stdout should append to the file, if redirected
    struct comd *next;  // pointer to the next comd in the pipeline or NULL if no next
};
typedef struct comd comd;

/**
 * TODO also take aliases etc into account
 */
int parse_from_file(char *line);

/*
 * parseexpr: parses the '\0' terminated expr string recursivly, according to the 'expr' grammar.
 * returns exit status (EXIT_SUCCESS || !EXIT_SUCCESS) of executed expression
 */
int parseexpr(char*);

/* 
 * is_valid_cmd: returns whether or not an occurence of a cmd string is valid in a given 
 *  context string. An cmd is valid iff it occurs as a comd in the grammar.
 *
 *  @param cmd: the command string to be checked
 *  @param context: the context string where the command occurs
 *  @param i: the index in the context string where the command occurs: context+i equals cmd
 */
bool is_valid_cmd(const char*, const char*, int);

#endif // jsh-parse.h
