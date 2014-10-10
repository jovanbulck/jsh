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

#include "jsh-common.h"
#include "alias.h"
#include "jsh-parse.c"
#include <signal.h>
#include <setjmp.h>
#include <readline/readline.h>      // GNU readline: http://cnswww.cns.cwru.edu/php/chet/readline/rltop.html
#include <readline/history.h>

// ########## macro definitions ##########
#define MAX_PROMPT_LENGTH       100     // maximum length of the displayed prompt
#define HISTFILE                ".jsh_history"
#define RCFILE                  ".jshrc"
#define LOGIN_FILE              ".jsh_login"

// ########## function declarations ##########
void option(char*);
void things_todo_at_start(void);
void things_todo_at_exit(void);
char *getprompt(int);
char *readcmd(int status);
int is_built_in(comd*);
int parse_built_in(comd*, int);
void sig_int_handler(int sig);
void touch_config_files(void);

// ########## global variables ##########
int DEBUG = 1;
int COLOR = 1;
int LOAD_RC = 1;
int I_AM_FORK = 0;
int IS_INTERACTIVE;      // initialized in things_todo_at_start; (compiler's 'constant initializer' complaints)
int nb_hist_entries = 0; // number of saved hist entries in this jsh session
sigjmp_buf ctrlc_buf;    // buf used for setjmp/longjmp when SIGINT received

/*
 * built_ins[] = array of built_in cmd names; should be sorted with 'qsort(built_ins, nb_built_ins, sizeof(char*), string_cmp);'
 * built_in enum = value corresponds to index in built_ins[]
 */
const char *built_ins[] = {"", "F", "T", "alias", "cd", "color", "debug",\
"exit", "history", "shcat", "unalias"};
#define nb_built_ins (sizeof(built_ins)/sizeof(built_ins[0]))
enum built_in {EMPTY, F, T, ALIAS, CD, CLR, DBG, EXIT, HIST, SHCAT, UNALIAS};
typedef enum built_in built_in;

/*
 *
 * TODO gracious malloc etc
 * TODO snprintf ipv printf for format string vulnerabilities...
 * TODO gnu readline disable on non interactive input
 * TODO read history MAX_HIST_SIZE ofzo??
 */
int main(int argc, char **argv) {
    int i, status;
	// process options
	for (i = 1; i < argc && *argv[i] == '-'; i++)
		option(argv[i]+1);
    
    things_todo_at_start();
    
    signal(SIGINT, sig_int_handler);
    // after receiving SIGINT, program is continued on the next line
    if (sigsetjmp(ctrlc_buf, 1) == 0)
        status = 0;     // get here on direct call
    else
        status = -1;    // get here by SIGINT signal
    
    char *s;
    while ((s = readcmd(status)) != NULL)
        status = parseexpr(s);
        
    exit(EXIT_SUCCESS);
}

/*
 * option: process an option [-OPTIONCHARS] string
 */
void option(char *str) {
	void optionfull(char *s); // option helper function

	char *begin = str;
	while (*str != '\0')
		switch(*str++) {
			case '-':
				if (str-1 == begin)
					return optionfull(str);
				break; // else: ignore
			case 'h':
				printf("jo-shell: A proof-of-concept shell implementation\n");
				printf("\nRecognized options:\n");
				printf("-h, --help\tdisplay this help message\n");
				printf("-d, --debug\tturn printing of debug messages on\n");
				printf("-n, --nodebug\tturn printing of debug messages on\n");
				printf("-c, --color\tturn coloring of jsh output messages on\n");
		        printf("-o, --nocolor\tturn coloring of jsh output messages off\n");
		        printf("-f, --norc\tdisable autoloading of the ~/%s file\n", RCFILE);
		        printf("-l, --license\tdisplay licence information\n");
		        printf("\nConfiguration files:\n");
		        printf("~/%s\tfile containing commands to be executed at login\n", RCFILE);
		        printf("~/%s\tfile containing the welcome message auto printed at login of an interactive session\n", LOGIN_FILE);
		        printf("~/%s\tfile containing the command history auto loaded and saved at login/logout\n", HISTFILE);
		        printf("\nReport bugs to: jo.vanbulck@student.kuleuven.be\n");
		        printf("jsh homepage: <https://todo.github.com/>\n");
		        printf("This program is free software, and you are welcome to redistribute it under\n");
                printf("the condititions of the GNU General Public License. Try 'jsh --license' for more info.\n");
		        exit(EXIT_SUCCESS);
			case 'd':
				DEBUG = 1;
				break;
			case 'n':
				DEBUG = 0;
				break;
			case 'c':
				COLOR = 1;
				break;
			case 'o':
				COLOR = 0;
				break;
		    case 'f':
		        LOAD_RC = 0;
		        break;
		    case 'l':
				printf("jo-shell: A proof-of-concept shell implementation\n");
                printf("Copyright (C) 2014 Jo Van Bulck <jo.vanbulck@student.kuleuven.be>\n");
                
                printf("\nThis program is free software: you can redistribute it and/or modify\n");
                printf("it under the terms of the GNU General Public License as published by\n");
                printf("the Free Software Foundation, either version 3 of the License, or\n");
                printf("(at your option) any later version.\n");
                
                printf("\nThis program is distributed in the hope that it will be useful,\n");
                printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
                printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
                printf("GNU General Public License for more details.\n");
                
                printf("\nYou should have received a copy of the GNU General Public License\n");
                printf("along with this program.  If not, see <https://www.gnu.org/licenses/>.\n");
                exit(EXIT_SUCCESS);
		        break;
			default:
				printerr("Unrecognized option '-%c'", *(str-1));
				printerr("Try 'jsh --help' for a list of regognized options");
				exit(EXIT_FAILURE);
		}
}

/*
 * optionfull: helper function to process an option string, in full [--OPTION] notation
 */
void optionfull(char *str) {
	if (strcmp(str,"nodebug") == 0)
		option("n");
	else if (strcmp(str,"debug") == 0)
		option("d");
	else if (strcmp(str,"nocolor") == 0)
		option("o");
	else if (strcmp(str,"color") == 0)  //TODO mss color=auto ... enum
	    option("c");
	else if (strcmp(str,"help") == 0)
	    option("h");
    else if (strcmp(str,"norc") == 0)
        option("f");
    else if (strcmp(str,"license") == 0)
        option("l");
	else {
		printerr("Unrecoginized option '--%s'\n", str);
		printerr("Try 'jsh --help' for a list of regognized options\n");
		exit(EXIT_FAILURE);
	}
}

/*
 * things_todo_at_exit: do stuff that needs to be done at jsh login
 */
void things_todo_at_start(void) {
    // assert the built_ins array is properly sorted
    #if ASSERT
        assert(is_sorted(built_ins, nb_built_ins, sizeof(char*), string_cmp));
        printdebug("built_ins array is_sorted() assertion passed :-)");
    #endif
    
    // evaluate once at startup; to maintain for forked children in a pipeline
    IS_INTERACTIVE = (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO));

    touch_config_files();
    
    // load history file if any
    char * path = concat(3, gethome(), "/", HISTFILE);
    if (read_history(path) == 0) 
        printdebug("reading history from %s succeeded", path);
    else 
        printdebug("reading history from %s failed", path);
    free(path);
    
    // register the things_todo_at_exit function atexit
    atexit(things_todo_at_exit);
    
    // built-in aliases
    alias("~", gethome());
    
    // read ~/.jshrc if any
    if (LOAD_RC) {
        path = concat(3, gethome(), "/", RCFILE);
        parsefile(path, (void (*)(char*)) parseexpr, false);
        free(path);
    }
    
    // print welcome message (without debugging output)
    if (IS_INTERACTIVE) {
        int temp = DEBUG;
        DEBUG = 0;        
        char * path = concat(3, gethome(), "/", LOGIN_FILE);
        parsefile(path, (void (*)(char*)) &printf, false);
        free(path);
        DEBUG = temp;
        printdebug("debugging is on. Turn it off with 'debug off'.");
    }
}

/*
 * create_config_files: looks for the jsh config files;
 *  if not found creates new empty ones (rw-rw-rw; will be combined with current umask)
 */
void touch_config_files(void) {
    #define CREATE_F(name) \
        path = concat(3, gethome(), "/", name); \
        fd = open(path, O_RDWR | O_CREAT, 0666); \
        if (fd >= 0) { \
            printdebug("opened file %s", name); \
            close(fd); \
        } \
        else \
            printdebug("couldn't open/create file '%s': %s", name, strerror(errno)); \
        free(path);
    
    char* path;
    int fd;
    CREATE_F(HISTFILE);
    CREATE_F(RCFILE);
    CREATE_F(LOGIN_FILE);
}

/*
 * things_todo_at_exit: do stuff that needs to be done at jsh logout (i.e. iff I_AM_FORK == 0)
 */
void things_todo_at_exit(void) {
    if (I_AM_FORK)
        return; //ignore exiting of child processes (e.g. failed fork execv)
    
    char * path = concat(3, gethome(), "/", HISTFILE);
    if (append_history(nb_hist_entries, path) == 0)
        printdebug("appending %d history entries to %s succeeded", nb_hist_entries, path);
    else
        printdebug("appending %d history entries to %s failed", nb_hist_entries, path);
    free(path);
}

/*
 * getprompt: return a string containing the command prompt (including status of last executed expr)
 *            iff IS_INTERACTIVE else the empty string is returned.
 *            the returned string is truncated to MAX_PROMPT_LENGTH TODO smarter truncation...
 */
char* getprompt(int status) {
    static char prompt[MAX_PROMPT_LENGTH] = "";  // static: hold between function calls (because return value)
    if (IS_INTERACTIVE) {
        int hostlen = sysconf(_SC_HOST_NAME_MAX)+1; // Plus one for null terminate
        char hostname[hostlen];
        gethostname(hostname, hostlen);
        hostname[hostlen-1] = '\0'; // Always null-terminate
        char *cwd = getcwd(NULL,0); //TODO portability: this is GNU libc specific... + errchk
        snprintf(prompt, MAX_PROMPT_LENGTH - 2, "%s@%s[%d]:%s", getenv("USER"), hostname, status, cwd);
        strcat(prompt, "$ ");
    }
    return prompt;
}

/*
 * readcmd: read the next inputline from stdin, add it to the history and resolve all aliases.
 *  returns the resolved inputline or NULL if EOF on a blank line
 * TODO remove status arg?
 */
char *readcmd(int status) {
    /** static variables that remain between function calls **/
    static char *buf = (char*) NULL;            // inputbuffer for readline()
    
    // display prompt and read full line in buf
    if (buf) {
        printdebug("readcmd: freeing memory for: '%s'", buf);
        free(buf); // If the buffer has already been allocated, return the memory to the free pool.
        buf = NULL;
    }
    buf = readline(getprompt(status));  //TODO fall back to getline() when non-interactive...
    printdebug("You entered: '%s'", buf);

    // If the line has any text in it: expand history, save it to history and resolve aliases
    //  (readline returns NULL iff EOF on a blank line)
    if (buf != NULL && *buf) {
        // do history expansion
        char *expansion = '\0';
        if (history_expand(buf, &expansion) != -1) {
            printdebug("readcmd: cmd '%s' expanded to '%s'", buf, expansion);
            free(buf); // free unexpanded version
            buf = expansion; // point to expanded cmd
        }
        else {
            printerr("readcmd: history expansion failed for '%s': '%s'", buf, expansion);
            free(expansion);
        }
        add_history(buf);
        nb_hist_entries++;
        char *ret = resolvealiases(buf);
        free(buf); // free unresolved version
        buf = ret; // point to resolved cmd
    }
    return buf;
}

/* 
 * is_built_in: returns -1 iff the provided comd isn't recognized as a built_in shell command,
 *  else returns the (positive) index in the built_in[] array.
 */
int is_built_in(comd* comd) {
   const char **rv = bsearch(comd->cmd, built_ins, nb_built_ins, sizeof(char*), string_cmp); //TODO strncmp?? buf overflow
   return ((rv == NULL)? -1: rv - built_ins);
}

/* 
 * parse_built_in: parses the provided *comd as a built_in shell command iff is_built_in(comd) != -1
 *  the provided int is an index in the built_in[] array, as provided by is_built_in(comd)
 *  returns exit status (EXIT_FAILURE || EXIT_SUCCESS) of executed built_in
 */
int parse_built_in(comd *comd, int index) {
    #if ASSERT
        assert(strcmp(*comd->cmd, built_ins[index]) == 0);
    #endif

    //######## common macro definitions ###########
    #define CHK_ARGC(cmd, argc) \
        if (comd->length != argc+1) { \
            printerr("%s: wrong number of arguments\t(expected = %d)", cmd, argc); \
            return EXIT_FAILURE; \
        }
    #define TOGGLE_VAR(name, var, arg) \
        CHK_ARGC(name, 1); \
        if (strcmp(arg, "on") == 0) { \
            printinfo("%s mode on", name); \
            var = 1; \
            return EXIT_SUCCESS; \
        } \
        else if (strcmp(arg, "off") == 0) { \
            printinfo("%s mode off", name); \
            var = 0; \
            return EXIT_SUCCESS; \
        } \
        else { \
            printerr("%s: expects argument 'on' || 'off'", name); \
            return EXIT_FAILURE; \
        }
    //######## built-in cmds switch ###########
    built_in b = (built_in) index;
    switch(b) {
        case EMPTY:
            printdebug("built-in: ignoring empty input");
            return EXIT_SUCCESS;
            break;
        case F:
            return EXIT_FAILURE; // FALSE
            break;
        case T:
            return EXIT_SUCCESS; // TRUE
            break;
        case ALIAS:
            if (comd->length == 1) {
                printaliases();
                return EXIT_SUCCESS;
            }
            else {
                CHK_ARGC("alias", 2);
                return alias(comd->cmd[1], comd->cmd[2]);
            }
            break;
        case CD: ; // (hackhackhack to allow declarions inside a switch)
            char *dir;
            if (comd->length == 1)
                dir = getenv("HOME");
            else {
                CHK_ARGC("cd",1);
                dir = comd->cmd[1];
            }
            CHK_ERR(chdir(dir), "cd");
            setenv("PWD", dir, 1);
            return EXIT_SUCCESS;
            break;
        case CLR:
            TOGGLE_VAR("color", COLOR, comd->cmd[1]);   //TODO global var
            break;
        case DBG:
            TOGGLE_VAR("debug", DEBUG, comd->cmd[1]);
            break;
        case EXIT:
            exit(EXIT_SUCCESS);
            break;
        case HIST:
            CHK_ARGC("history", 0);
            HIST_ENTRY **hlist = history_list();
            int i;
            if (hlist)
                for (i = 0; hlist[i]; i++)
                    printf ("%s\n", hlist[i]->line);
            return EXIT_SUCCESS;
            break;
        case SHCAT:
            parsestream(stdin, "stdin", (void (*)(char*)) printf);  // built_in cat; mainly for testing purposes (redirecting stdin)
            return EXIT_SUCCESS;
            break;
        case UNALIAS:
            CHK_ARGC("unalias", 1);
            return unalias(comd->cmd[1]); //TODO dit wordt geresolved ...
            break;
        default:
            printerr("parse_built_in: unrecognized built_in command: '%s' with index %d", *comd->cmd, index);
			exit(EXIT_FAILURE);
    }
}

/*
 * sig_int_handler: this function is called when the user enters ^C and 
 * tells GNU readline to diplay a prompt on a newline
 */
void sig_int_handler(int signo) {
    rl_crlf();                  // set cursor to newline
    siglongjmp(ctrlc_buf, 1);   // jump back to main loop
}
