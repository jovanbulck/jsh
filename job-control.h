/* This file is part of jsh.
 * 
 * jsh (jo-shell): A proof-of-concept shell implementation
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

#ifndef JOB_CTRL_H_INCLUDED
#define JOB_CTRL_H_INCLUDED
/* ^^ these are the include guards */

#include "jsh-common.h"

void addbgjob(pid_t);
void removebgjob(pid_t); // called when SIGCHLD signals ending of child process
void continuebgjob(pid_t); // called by bg cmd             send SIGCONT
void killbgjob(pid_t); // called by bg cmd
void printbgjobs();


#endif //JOB_CTRL_INCLUDED
