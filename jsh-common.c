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

#include "jsh-common.h"

#define SET_ERR_COLOR \
    if (IS_INTERACTIVE && COLOR) \
        textcolor(stderr, BRIGHT, RED);

void printerr(const char *format, ...) {
    SET_ERR_COLOR;
    va_list args;
    fprintf(stderr, "jsh: ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    RESTORE_COLOR(stderr);
}

void printerrno(const char *format, ...) {
    SET_ERR_COLOR;
    va_list args;
    fprintf(stderr, "jsh: ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args );
    fprintf(stderr, ": %s", strerror(errno));
    RESTORE_COLOR(stderr);
}

// TODO mss int debuglevel / priority
void printdebug(const char *format, ...) {
    if (DEBUG) {
        if (IS_INTERACTIVE && COLOR)
            textcolor(stdout, RESET, YELLOW);	
        
        va_list args;
        fprintf(stdout, "[DEBUG: ");
        va_start(args, format);
        vfprintf(stdout, format, args);
        va_end(args);
        fprintf(stdout, "]");
        RESTORE_COLOR(stdout);
        fflush(stdout); // force the debug info to be instantly written
    }
}

void printinfo(const char *format, ...) {
    va_list args;
    fprintf(stdout, "jsh: ");
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args );
    fprintf(stdout, "\n");
}

/*
 * only set the style and fg (to prevent problems with translucency...)
 * use RESTORE_COLOR macro to reset the color afterwards
 */
void textcolor(FILE *stream, int style, int fg) {
    char colorcode[13];
	sprintf(colorcode, "%c[%d;%dm", 0x1B, style, fg + 30);
	fprintf(stream, "%s", colorcode);
}

/*
 * returns getenv("HOME") if not null; else "."
 */
char *gethome() {
    char *curdir = ".";
    char *rv =  getenv("HOME");
    return (rv != NULL)? rv: curdir;
}

inline int puts_verbatim(const char *s) {
    return fputs(s, stdout);
}

/*
 * parsefile: wrapper for parsestream(), opening and closing the file at the provided path.
 * @arg errmsg: true  = print an error message if opening the file failed
 *               false = exit silently if opening the file failed
 * @NOTE: if you pass a pointer to 'printf()' here, this may introduce format-string-
 *  vulnerabilities as the lines are passed verbatim to the function. If you want to use
 *  this function to print the content of a file line per line, pass a pointer to
 *  'puts_verbatim()' (defined in jsh-common.h) instead.
 */
void parsefile(char *path, void (*fct)(char*), bool errmsg) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        if (errmsg) printerrno("opening of file '%s' failed", path);
        return;
    }
    parsestream(file, path, fct);
    fclose(file);
}

/*
 * parsestream: reads the provided stream strm line per line, passing each line, 
 *  including '\n' to the provided function fct.
 * @NOTE: if you pass a pointer to 'printf()' here, this may introduce format-string-
 *  vulnerabilities as the lines are passed verbatim to the function. If you want to use
 *  this function to print the content of a file line per line, pass a pointer to
 *  'puts_verbatim()' (defined in jsh-common.h) instead.
 */
void parsestream(FILE *strm, char* name, void (*fct)(char*)) {
    printdebug("-------- now parsing stream '%s' --------", name);
    
    char line[MAX_FILE_LINE_LENGTH+1];
    int c, i = 0, j = 1;   // i = index in line; j = line nb
    
    while ((c = fgetc(strm)) != EOF)
         if (c == '\n') {
            line[i] = '\0';
            printdebug("%s: now parsing line %d: '%s'", name, j, line);
            fct(line);
            fct("\n");
            i = 0;
            j++;
         }
         else if (i < MAX_FILE_LINE_LENGTH)
            line[i++] = c;
         else {
            printerr("%s: ignoring line %d exeeding the max line length: %d", name, j, MAX_FILE_LINE_LENGTH);
            while ((c=fgetc(strm)) != '\n');
            i = 0;
            j++;
         }
    printdebug("-------- end of stream '%s' --------", name);    
}

/*
 * string_cmp: wrapper function for strcmp(); to be passed to bsearch() or qsort() in order to compare
 *  two pointers to a string (char**)
 */
int string_cmp(const void *a, const void *b) {
    const char **ia = (const char **) a;
    const char **ib = (const char **) b;
    return strcmp(*ia, *ib);
}


/*
 * is_sorted: returns false iff the provided array a isn't sorted according to the provided comparison function,
 *  else returns true; time complexity O(n)
 */
bool is_sorted(void *a, size_t n, size_t el_size, int (*compar)(const void *, const void *)) {
    int i;
    for (i = 0; i < (n - 1)*el_size; i += el_size) {
        int rv = compar((a + i), (a + i + el_size));
        if (rv >= 0)
            return false;
    }
    return true;
}


/*
 * concat: concatenates count nb of strings and returns a pointer to the malloc()ed result.
 *  the caller should free() the result afterwards
 * credits: http://stackoverflow.com/a/11394336
 */
char* concat(int count, ...) {
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    // Allocate memory to concat strings TODO chkerr
    char *merged = calloc(sizeof(char),len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}

/*
 * remove_char: helper function: deletes all occurences of a specified char in a given '\0' terminated string.
 *  returns the resuling '\0' terminated string
 *  credits: http://stackoverflow.com/a/8733511
 */
void remove_char(char *str, char garbage) {
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}
