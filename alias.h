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

#ifndef ALIAS_H_INCLUDED
#define ALIAS_H_INCLUDED
/* ^^ these are the include guards */

#include "jsh-common.h"

int alias(char*, char*);
int unalias(char* key);
int printaliases();
char *resolvealiases(char*);
bool alias_exists(char*);
bool existing_alias_update(char*,char*);
#endif //ALIAS_H_INCLUDED
