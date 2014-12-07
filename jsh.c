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
#include "jsh-colors.h"
#include "alias.h"
#include "jsh-parse.h"
#include "jsh-completion.h"
#include <signal.h>
#include <setjmp.h>
#include <readline/readline.h>      // GNU readline: http://cnswww.cns.cwru.edu/php/chet/readline/rltop.html
#include <readline/history.h>

// ########## macro definitions ##########
#ifndef VERSION
    #define VERSION             "jsh: unknown built" // makefile normally fills in the version info
#endif
#define RCFILE                  ".jshrc"
#define HISTFILE                ".jsh_history"
#define LOGIN_FILE              ".jsh_login"
#define LOGOUT_FILE             ".jsh_logout"
#define DEFAULT_PROMPT          "%B%u%n@%h[%S]::%f{yellow}%d%f{reset}%$ "    // default init prompt string: "user@host[status]:pwd$ "
#define MAX_PROMPT_LENGTH       250                 // maximum length of the displayed prompt string
#define MAX_PROMPT_BUF_LENGTH   50                  // the max number of msd of a status integer in the prompt string
// ########## function declarations ##########
void option(char*);
void things_todo_at_start(void);
void things_todo_at_exit(void);
char *getprompt(int);
char *readcmd(int status);
int is_built_in(comd*);
int parse_built_in(comd*, int);
void sig_int_handler(int);
void touch_config_files(void);
char *resolve_prompt_colors(char*);

// ########## global variables ##########
#ifdef NODEBUG
    bool DEBUG = false;
#else
    bool DEBUG = true;
#endif

#ifdef NOCOLOR
    bool COLOR = false;
#else
    bool COLOR = true;
#endif

#ifdef NORC
    bool LOAD_RC = false;
#else
    bool LOAD_RC = true;
#endif

bool WAITING_FOR_CHILD = false; // whether or not the jsh parent process is currently (blocking) waiting for child termination
bool I_AM_FORK = false;
bool IS_INTERACTIVE;            // initialized in things_todo_at_start; (compiler's 'constant initializer' complaints)
int nb_hist_entries = 0;        // number of saved hist entries in this jsh session
sigjmp_buf ctrlc_buf;           // buf used for setjmp/longjmp when SIGINT received
char *user_prompt_string = ">"; // initialized in things_todo_at_start function
int MAX_DIR_LENGTH = 25;        // the maximum length of an expanded pwd substring in the prompt string

/*
 * built_ins[] = array of built_in cmd names; should be sorted with 'qsort(built_ins, nb_built_ins, sizeof(char*), string_cmp);'
 * built_in enum = value corresponds to index in built_ins[]
 */
const char *built_ins[] = {"", "F", "T", "alias", "cd", "color", "debug",\
"exit", "history", "prompt", "shcat", "source", "unalias"};
const size_t nb_built_ins = sizeof(built_ins)/sizeof(built_ins[0]);
enum built_in {EMPTY, F, T, ALIAS, CD, CLR, DBG, EXIT, HIST, PROMPT, SHCAT, SRC, UNALIAS};
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
                printf("jo-shell: A basic UNIX shell implementation in C\n");
                printf("\nRecognized options:\n");
                printf("-h, --help\tdisplay this help message\n");
                printf("-d, --debug\tturn printing of debug messages on\n");
                printf("-n, --nodebug\tturn printing of debug messages on\n");
                printf("-c, --color\tturn coloring of jsh output messages on\n");
                printf("-o, --nocolor\tturn coloring of jsh output messages off\n");
                printf("-f, --norc\tdisable autoloading of the ~/%s file\n", RCFILE);
		        printf("-l, --license\tdisplay licence information\n");
    	        printf("-v, --version\tdisplay version information\n");
    	        printf("\nConfiguration files:\n");
    	        printf("~/%s\tfile containing commands to be executed at login\n", RCFILE);
    	        printf("~/%s\tfile containing the welcome message auto printed at login of an interactive session\n", LOGIN_FILE);
       	        printf("~/%s\tfile containing commands to be executed at logout of an interactive session\n", LOGOUT_FILE);
    	        printf("~/%s\tfile containing the command history auto loaded and saved at login/logout\n", HISTFILE);
    	        printf("\nReport bugs to: jo.vanbulck@student.kuleuven.be\n");
                printf("jsh homepage: <https://github.com/jovanbulck/jo-shell>\n");
    	        printf("This program is free software, and you are welcome to redistribute it under\n");
                printf("the condititions of the GNU General Public License. Try 'jsh --license' for more info.\n");
		        exit(EXIT_SUCCESS);
            case 'v': 
                printf("%s\n", VERSION);
                exit(EXIT_SUCCESS);
                break;
			case 'd':
				DEBUG = true;
				break;
			case 'n':
				DEBUG = false;
				break;
			case 'c':
				COLOR = true;
				break;
			case 'o':
				COLOR = false;
				break;
		    case 'f':
		        LOAD_RC = false;
		        break;
		    case 'l':
				printf("jo-shell: A basic UNIX shell implementation in C\n");
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
                printf("along with this program. If not, see <https://www.gnu.org/licenses/>.\n");
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
	else if (strcmp(str,"version") == 0)
	    option("v");
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
    
    // enable custom rl_autocompletion
    rl_attempted_completion_function = jsh_command_completion;
    
    // built-in aliases
    alias("~", gethome());
    
    // default prompt
    user_prompt_string = resolve_prompt_colors(DEFAULT_PROMPT);
    
    // read ~/.jshrc if any
    if (LOAD_RC) {
        path = concat(3, gethome(), "/", RCFILE);
        parsefile(path, (void (*)(char*)) parseexpr, false);
        free(path);
    }
    
    // print welcome message (without debugging output)
    if (IS_INTERACTIVE) {
        bool temp = DEBUG;
        DEBUG = false;        
        char * path = concat(3, gethome(), "/", LOGIN_FILE);
        parsefile(path, (void (*)(char*)) puts_verbatim, false);
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
    CREATE_F(LOGOUT_FILE);
}

/*
 * things_todo_at_exit: do stuff that needs to be done at jsh logout (i.e. iff I_AM_FORK == 0)
 */
void things_todo_at_exit(void) {
    if (I_AM_FORK)
        return; //ignore exiting of child processes (e.g. failed fork execv)
    
    // execute commands in logout file (silently)
    if (IS_INTERACTIVE) {
        printdebug("now executing '%s'", LOGOUT_FILE);
        bool dbg = DEBUG;
        DEBUG = false;
        char *path = concat(3, gethome(), "/", LOGOUT_FILE);
        parsefile(path, (void (*)(char*)) parseexpr, false);
        free(path);
        DEBUG = dbg;
        printdebug("'%s' executed", LOGOUT_FILE);
    }
    
    char * path = concat(3, gethome(), "/", HISTFILE);  //TODO check this uses malloc??? fail return status?
    if (append_history(nb_hist_entries, path) == 0)
        printdebug("appending %d history entries to %s succeeded", nb_hist_entries, path);
    else
        printdebug("appending %d history entries to %s failed", nb_hist_entries, path);
    free(path);
}

/*
 * getprompt: return a string representing the command prompt (as defined by the user_prompt_string)
 *  iff IS_INTERACTIVE, else the empty string is returned. The returned string is guaranteed to be less then 
 *  MAX_PROMPT_LENGTH; when a directory is expanded in the prompt string, it is 'smart' truncated to MAX_DIR_LENGTH.
 *  When a status integer or git branch is included in the prompt string, the strings length is truncated to MAX_PROMPT_BUF_LENGTH.
 */
char* getprompt(int status) {    
    // static string to hold the current prompt (return value) between function calls
    static char prompt[MAX_PROMPT_LENGTH] = "";
    static char buf[MAX_PROMPT_BUF_LENGTH] = "";    // used for char / int to string conversion 
    
    if (!IS_INTERACTIVE)
        return "";    

    prompt[0] = '\0';               // clear the prev prompt
    char *next;                     // points to the next substring to add to the prompt
    int i;
    for (i = 0; i < strlen(user_prompt_string); i++) {
        /*** check for '%' prompt expansion options ***/
        if (user_prompt_string[i] == '%') {
            i++; // potentially overread the '\0' char (harmless)
            switch (user_prompt_string[i]) {
                case 'u':
                    next = getenv("USER");
                    break;
                case 'U':
                    {
                    char *username = getenv("USER");
                    // make the username red and bold when sudo access is activated
                    // see e.g. http://stackoverflow.com/questions/122276/quickly-check-whether-sudo-permissions-are-available
                    char *cur_user_has_sudo = strclone("sudo -n true > /dev/null 2> /dev/null");
	                if (parseexpr(cur_user_has_sudo) == EXIT_SUCCESS) {
	                    snprintf(buf, MAX_PROMPT_BUF_LENGTH, "%s%s%s", COLOR_BOLD RED_FG, \
	                        username, COLOR_RESET_BOLD RESET_FG);
	                    next = buf;
	                }
	                else
	                    next = username;
	                
	                free(cur_user_has_sudo);
                    break;
                    }
                case '$':
                    {
                    char *cur_user_has_sudo = strclone("sudo -n true > /dev/null 2> /dev/null");
	                if (parseexpr(cur_user_has_sudo) == EXIT_SUCCESS)
	                    next = "#";
	                else
	                    next = "$";
	                
	                free(cur_user_has_sudo);
                    break;
                    }
                case 'h':
                    {
                    int hostlen = sysconf(_SC_HOST_NAME_MAX)+1; // Plus one for null terminate
                    char hostname[hostlen];
                    gethostname(hostname, hostlen);
                    hostname[hostlen-1] = '\0'; // Always null-terminate                     
                    next = hostname;
                    break;
                    }
                case 's':
                    snprintf(buf, MAX_PROMPT_BUF_LENGTH, "%d", status);
                    next = buf;
                    break;
                case 'S':
                    if (!status)
                        snprintf(buf, MAX_PROMPT_BUF_LENGTH, "%d", status);
                    else
                        snprintf(buf, MAX_PROMPT_BUF_LENGTH, "%s%d%s", COLOR_BOLD RED_FG, \
	                        status, COLOR_RESET_BOLD RESET_FG);
                    next = buf;
                    break;
                case 'd':
                    {
                    // get the directory                
                    char *cwd = getcwd(NULL, 0); //TODO portability: this is GNU libc specific... + errchk
                    
                    // replace the home dir with '~' if any
                    char *home = gethome();
                    if (strstr(cwd, home)) {
                        cwd = cwd + strlen(home)-1;
                        cwd[0] = '~';
                    }
                    
                    int cwdlen = strlen(cwd);
                    char *ptr = NULL;
                    // get a ptr to the first '/' + 1 within the truncated directory string
                    if (cwdlen > MAX_DIR_LENGTH) {
                        ptr = strchr(cwd + cwdlen - MAX_DIR_LENGTH, '/');
                        ptr = (ptr < cwd+cwdlen-1)? ptr+1 : ptr;   
                    }
                    next = ptr? ptr : cwd + ((MAX_DIR_LENGTH < cwdlen) ? cwdlen - MAX_DIR_LENGTH : 0);
                    break;
                    }
                case 'g':
                    {
                    char *pwd_is_git = strclone("git rev-parse --git-dir > /dev/null 2> /dev/null");
	                if (parseexpr(pwd_is_git) == EXIT_SUCCESS) {
	                    FILE *fp = popen("git symbolic-ref --short -q HEAD", "r");
	                    buf[0] = ' ';
	                    buf[1] = '[';
	                    buf[MAX_PROMPT_BUF_LENGTH-2] = ']';
	                    buf[MAX_PROMPT_BUF_LENGTH-1] = '\0';
	                    int j;
	                    for (j = 2; j < MAX_PROMPT_BUF_LENGTH-2; j++) {
	                        int cur = getc(fp);
	                        if (cur == EOF || cur == '\n') {
	                            buf[j++] = ']';
	                            buf[j] = '\0';
	                            break;
	                        }
	                        else
	                            buf[j] = cur;
	                    }
	                    next = buf;
	                    pclose(fp);
	                }
	                else
	                    next = "";
	                free(pwd_is_git);
	                break;
	                }
	            case 'c':
	                {
                    char *pwd_is_git = strclone("git rev-parse --git-dir > /dev/null 2> /dev/null");
	                char *files_have_changed = strclone("git diff --exit-code > /dev/null 2> /dev/null");                    
	                if ( (parseexpr(pwd_is_git) == EXIT_SUCCESS) && (parseexpr(files_have_changed) != EXIT_SUCCESS) ) {
                        snprintf(buf, MAX_PROMPT_BUF_LENGTH, "%s%c%s", COLOR_BOLD RED_FG, \
	                        '*', COLOR_RESET_BOLD RESET_FG);
	                    next = buf;
                    }
	                else
	                    next = "";
	                free(pwd_is_git);
                    free(files_have_changed);	                
	                break;
	                }
                case '%':
                    next = "%";
                    break;
                default:
                    printerr("skipping unrecognized prompt option '%%%c'", user_prompt_string[i]);
                    next = "";
                    break;
            }
        }
        /*** no prompt expansion; copy the char verbatim ***/        
        else {
            sprintf(buf, "%c", user_prompt_string[i]);
            next = buf;
        }
        
        /*** check length of string to concat; abort to avoid an overflow ***/
        if ((strlen(prompt) + strlen(next)) >= MAX_PROMPT_LENGTH) {
            printerr("Prompt expansion too long: not concatting '%s'. Now returning...", next);
            return prompt;
        }
        strcat(prompt, next);
    }
    return prompt;
}

/**
 * resolve_prompt_colors: return a newly malloced prompt string with the
 *  symbolic color expansion codes replaced with the corresponding ANSI escape codes.
 *  This function can be called once on new promp initialisation to save time on
 *  subsequent prompt re-evaluations.
 * NOTE: the return value is a malloced str with length MAX_PROMPT_LENGTH; when the 
 *  expansion is longer, it is truncated and an error message is written to stdout
 */
char *resolve_prompt_colors(char *prompt) {
    char *rv = malloc(MAX_PROMPT_LENGTH);
    rv[0] = '\0';
    char buf[3]; // to hold the next char and '%' temporarily to append to rv

    // macro to insert a given ANSI escape color value str iff a given name str matches
    #define CHK_COLOR(cname, cvalue) \
        if (!strncmp(prompt+i+1, "{" cname "}", strlen("{" cname "}"))) { \
            next = cvalue; \
            i += strlen("{" cname "}"); \
            break; \
        } 
    
    char *next;
    int i;
    for (i = 0; i < strlen(prompt); i++) {
        if (prompt[i] == '%') {
            i++; // potentially overread the '\0' char (harmless)
            switch (prompt[i]) {
                case 'f':
                    // fg color, normal
                    CHK_COLOR("black", BLACK_FG)
                    CHK_COLOR("red", RED_FG)
                    CHK_COLOR("green", GREEN_FG)
                    CHK_COLOR("yellow", YELLOW_FG)
                    CHK_COLOR("blue", BLUE_FG)
                    CHK_COLOR("magenta", MAGENTA_FG)
                    CHK_COLOR("cyan", CYAN_FG)
                    CHK_COLOR("white", WHITE_FG)
                    CHK_COLOR("reset", RESET_FG)
                    CHK_COLOR("resetall", COLOR_RESET_ALL)
                    
                    printerr("resolve_prompt_colors: skipping empty color prompt option: specify non-bold fg color with '%%f{color_name}'");
                    next = "";
                    break;
                 case 'F':
                    // fg color, bold
                    CHK_COLOR("black", COLOR_BOLD BLACK_FG)
                    CHK_COLOR("red", COLOR_BOLD RED_FG)
                    CHK_COLOR("green", COLOR_BOLD GREEN_FG)
                    CHK_COLOR("yellow", COLOR_BOLD YELLOW_FG)
                    CHK_COLOR("blue", COLOR_BOLD BLUE_FG)
                    CHK_COLOR("magenta", COLOR_BOLD MAGENTA_FG)
                    CHK_COLOR("cyan", COLOR_BOLD CYAN_FG)
                    CHK_COLOR("white", COLOR_BOLD WHITE_FG)
                    CHK_COLOR("reset", COLOR_RESET_BOLD RESET_FG)
                    CHK_COLOR("resetall", COLOR_RESET_ALL)
                    
                    printerr("resolve_prompt_colors: skipping empty color prompt option: specify bold fg color with '%%f{color_name}'");
                    next = "";
                    break;
                case 'B':
                    next = COLOR_BOLD;
                    break;
                case 'n':
                    next = COLOR_RESET_BOLD;
                    break;
                case 'b':
                    // bg color, normal
                    CHK_COLOR("black", BLACK_BG)
                    CHK_COLOR("red", RED_BG)
                    CHK_COLOR("green", GREEN_BG)
                    CHK_COLOR("yellow", YELLOW_BG)
                    CHK_COLOR("blue", BLUE_BG)
                    CHK_COLOR("magenta", MAGENTA_BG)
                    CHK_COLOR("cyan", CYAN_BG)
                    CHK_COLOR("white", WHITE_BG)
                    CHK_COLOR("reset", RESET_BG)
                    CHK_COLOR("resetall", COLOR_RESET_ALL)
                    
                    printerr("resolve_prompt_colors: skipping empty color prompt option: specify bg color with '%%b{color_name}'");
                    next = "";
                    break;
                default:
                    sprintf(buf, "%%%c", prompt[i]);
                    next = buf;
                    break;
            }
        }
        /*** no prompt expansion; copy the char verbatim ***/        
        else {
            sprintf(buf, "%c", prompt[i]);
            next = buf;
        }
        
        /*** check length of string to concat; abort to avoid an overflow ***/
        if ((strlen(rv) + strlen(next)) >= MAX_PROMPT_LENGTH) {
            printerr("Prompt expansion too long: not concatting '%s'. Now returning...", next);
            return rv;
        }
        strcat(rv, next);
    }
    return rv;   
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
    
    // If the line has any text in it: expand history, save it to history and resolve aliases
    //  (readline returns NULL iff EOF on a blank line)
    if (buf && *buf) {
        printdebug("You entered: '%s'", buf);
        // do history expansion
        char *expansion = "";
        int hist_rv;
        if ((hist_rv = history_expand(buf, &expansion)) != -1) {
            if (hist_rv == 1)
                printf("%s\n", expansion);  // bash-style print the expanded command string iff changed
            free(buf);                      // free unexpanded version
            buf = expansion;                // point to expanded cmd
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
    else if (!buf) {
        printf("\n");
        printdebug("You entered EOF");
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
        case CD:
            { // to allow declarions inside a switch)
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
            }
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
            // check for the optional argument
            // nb-entries: print the number of hist entries in the current session
            if (comd->length == 2 && strcmp(comd->cmd[1], "--nb-entries") == 0) {
                printf("%d\n", nb_hist_entries);
                return EXIT_SUCCESS;
                break;
            }
            else
                CHK_ARGC("history", 0);
            
            HIST_ENTRY **hlist = history_list();
            int i;
            if (hlist)
                for (i = 0; hlist[i]; i++)
                    printf ("%s\n", hlist[i]->line);
            return EXIT_SUCCESS;
            break;
        case PROMPT:
            {
            // check for the optional dir length argument
            if (comd->length == 3) {
                MAX_DIR_LENGTH = abs(atoi(comd->cmd[2]));    // will return 0 on non-integer
                printdebug("setting MAX_DIR_LENGTH to %d", MAX_DIR_LENGTH);
            }
            else
                CHK_ARGC("prompt", 1);
            
            free(user_prompt_string);
            char *newprompt = resolve_prompt_colors(comd->cmd[1]);
            printdebug("setting user_prompt_string to '%s'", newprompt);
            user_prompt_string = newprompt;
            return EXIT_SUCCESS;
            break;
            }
        case SHCAT:
            parsestream(stdin, "stdin", (void (*)(char*)) puts_verbatim);  // built_in cat; mainly for testing purposes (redirecting stdin)
            return EXIT_SUCCESS;
            break;
        case UNALIAS:
            CHK_ARGC("unalias", 1);
            return unalias(comd->cmd[1]);
            break;
		case SRC:
			CHK_ARGC("source", 1);
			parsefile(comd->cmd[1], (void (*)(char*)) parseexpr, true); // errormsg if file not found
			return EXIT_SUCCESS;
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
    // if ^C entered in child process --> also sent to parent (jsh) process
    // --> only clear the prompt when not waiting for an executing child (allow the waitpid to return)
    if (!WAITING_FOR_CHILD) {
        rl_crlf();                  // set cursor to newline
        siglongjmp(ctrlc_buf, 1);   // jump back to main loop
    }
}
