/* This file is part of jsh.
 * 
 * jsh (jo-shell): A basic shell implementation
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

#ifndef COMPL_H_INCLUDED
#define COMPL_H_INCLUDED
/* ^^ these are the include guards */

#include "jsh-common.h"
#include "jsh-parse.h"
#include "alias.h"
#include <readline/readline.h>      // GNU readline: http://cnswww.cns.cwru.edu/php/chet/readline/rltop.html

extern const char *built_ins[];
extern const size_t nb_built_ins;

/*
 * jsh_completion: a custom GNU readline completion function for jsh command completion.
 *  This function is called by readline; if the result is non-NULL, readline wont perform 
 *  the default file completion.
 * @arg: from the readline manual: "start and end are indices in rl_line_buffer defining 
 *  the boundaries of text, which is a character string."
 * @return: an array of strings with the possible completions or NULL of no completions.
 */
char** jsh_command_completion(const char*, int, int);

#endif //jsh-completion.h
