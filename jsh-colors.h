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

#ifndef JSH_COLORS_H_INCLUDED
#define JSH_COLORS_H_INCLUDED

// surround non-printing escape sequences with '\x01' and '\x02' for GNU readline lib
//  see e.g. http://bugs.python.org/issue20359

// ANSI escape foreground color codes (see https://en.wikipedia.org/wiki/ANSI_escape_code)
#define BLACK_FG            "\x01\033[30m\x02"
#define RED_FG              "\x01\033[31m\x02"
#define GREEN_FG            "\x01\033[32m\x02"
#define YELLOW_FG           "\x01\033[33m\x02"
#define BLUE_FG             "\x01\033[34m\x02"
#define MAGENTA_FG          "\x01\033[35m\x02"
#define CYAN_FG             "\x01\033[36m\x02"
#define WHITE_FG            "\x01\033[37m\x02"
#define RESET_FG            "\x01\033[39m\x02"

// ANSI escape background color codes
#define BLACK_BG            "\x01\033[40m\x02"
#define RED_BG              "\x01\033[41m\x02"
#define GREEN_BG            "\x01\033[42m\x02"
#define YELLOW_BG           "\x01\033[43m\x02"
#define BLUE_BG             "\x01\033[44m\x02"
#define MAGENTA_BG          "\x01\033[45m\x02"
#define CYAN_BG             "\x01\033[46m\x02"
#define WHITE_BG            "\x01\033[47m\x02"
#define RESET_BG            "\x01\033[49m\x02"

// ANSI escape style color codes
#define COLOR_RESET_ALL     "\x01\033[0m\x02"   // back to defaults
#define COLOR_BOLD 		    "\x01\033[1m\x02"   // implemented as 'bright' on some terminals
#define COLOR_RESET_BOLD    "\x01\033[22m\x02"

// (the following are not widely supported)
#define COLOR_DIM		    "\x01\033[2m\x02"
#define COLOR_UNDERLINE     "\x01\033[3m\x02"
#define COLOR_BLINK		    "\x01\033[4m\x02"
#define COLOR_REVERSE       "\x01\033[7m\x02"

#endif // JSH_COLORS_H_INCLUDED
