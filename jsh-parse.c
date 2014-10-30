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
 *
 * ----------------------------------------------------------------------
 * jsh-parse.c: a file containing functions to parse input according to the following grammar:
 *
 * input  :=    expr
 *
 * expr   :=    <space>expr         // expr is a logical combination
 *              expr<space>
 *              expr #comment
 *              "expr"
 *              (expr)
 *              expr ; expr
 *              expr && expr
 *              expr || expr
 *              cmd
 *
 * cmd    :=    cmd | cmd           // cmd is the unit of truth value evaluation
 *              cmd >> path         // note: pipe redirection get priority over explicit redirection
 *              cmd 2> path
 *              cmd > path
 *              cmd < path
 *              cmd &               *TODO
 *              comd
 *
 * comd   :=    comd option         // comd is the unit of fork / built_in
 *              comd "option with spaces"   // TODO? e.g. echo "ik ben jo" en git commit -m "dit is een message"
 *              alias               // note priority: alias > built_in > executable
 *              built_in
 *              executable_path     // relative (using the PATH env var) or absolute
 * ----------------------------------------------------------------------
 *
 * e.g.  :  ls / -l  >> out.txt && cat < out.txt | grep --color=auto -B 2 usr ; pwd
 *          (ls -al > outfile && cat outfile | grep jsh | wc -w ) &&(pwd || echo i am not executing);echo final
 *          (../exit -1&&echo i am not executing ||echo me neither); pwd
 *          ((../exit -1&& echo i am not executing)|| echo i am);pwd
 *          ( ( (a && b) || (c) ) && (d) )
 *          ls | grep out | print | print | print | cat | print | print | cat | print
 *          (debug off && (echo cur dir is && pwd) && (history | grep ls | shcat | cat | shcat | ../mini-grep pwd | 
 *              cat | shcat | shcat | ../mini-grep shcat)) #comment
 *          echo hi  # dit ~ is (commentaar) && pwd ; dit (((ook cd && ### echo jo )
 *
 * TODO     alias replacement between () for truth value?
 */

#include "jsh-parse.h"

#define RESOLVE_TRUTH_VAL(rv) ((rv == EXIT_SUCCESS)? 'T' : 'F') // note: 'T' and 'F' are built-ins

// #################### helper function declarations ####################
comd *createcomd(char**);
void freecomdlist(comd*);
int parsecmd(char**, int);
int execute(comd*, int);
void redirectstreams(comd*, int, int);
int exec_built_in(comd*, int, int);
extern int is_built_in(comd*);
extern int parse_built_in(comd*, int);

/*
 * createcomd: returns a pointer to a newly created comd struct, 
 *  using defaults: {cmd, lengthof(cmd), NULL, NULL, NULL, 0, NULL}
 *  The caller should free() the returned comd after use, e.g. using the freecomdlist() function.
 */
comd *createcomd(char **cmd) {
    comd *ret = malloc(sizeof(comd));   //TODO chkerr
    ret->cmd = cmd;
    
    // calculate cmd length
    int i, length = 0;
    for (i = 0; cmd[i] != NULL; i++)
        length++;
    
    ret->length = length;
    ret->inf = NULL;
    ret->outf = NULL;
    ret->errf = NULL;
    ret->append_out = 0;
    ret->next = NULL;
    return ret;
}

/*
 * free()s the provided comd chain
 */
void freecomdlist(comd *list) {
    comd *cur = list;
    while (cur != NULL) {
        printdebug("freeing comd struct for '%s'", *cur->cmd);
        comd *temp = cur;
        cur = cur->next;
        free(temp);
   }
}

/*
 * parseexpr: parses the '\0' terminated expr string recursivly, according to the 'expr' grammar.
 * returns exit status (EXIT_SUCCESS || !EXIT_SUCCESS) of executed expression
 */
//int parseexpr(char **expr, int length) { //TODO length arg?? ipv null term string
int parseexpr(char *expr) {
    int resolvebrackets(char*, int);          //helper functions declarations
    int splitexpr(char*, char***);
    int length = strlen(expr);
    int i, rv;
    bool inquotes = false;
    
    /**** 0. skip all leading spaces tabs and newlines ****/
    int k =  strspn(expr, " \t\n");
    expr = expr + k;
    if (k)
        printdebug("parseexpr: skipped %d leading spaces/tabs", k);
    
    /**** 1. BRACKETS: resolve, if any ****/
    if ((rv = resolvebrackets(expr, length)) != -1)
        return rv;
    
    /**** 2. OPERATORS: first subexpression (till first logic operator) has no more brackets ****/
    for (i = 0; i < length; i++) {
        if (inquotes && (expr[i] != '"' || expr[i-1] == '\\'))
            continue;   //(unescaped) quotes protect their content from being interpreted
        else if (expr[i] == '#') {
            expr[i] = '\0';
            return parseexpr(expr);    
        }
        else if (expr[i] == '"' && (i == 0 || expr[i-1] != '\\')) {
            inquotes = inquotes?false:true;
        }
        else if (expr[i] == ';') {
            expr[i] = '\0';
            parseexpr(expr);
            return parseexpr(expr+i+1);
        }
        else if (strncmp(expr+i, "&&", 2) == 0) {
            expr[i] = '\0';
            if (parseexpr(expr) == EXIT_SUCCESS)
                return parseexpr(expr+i+2);
            else
                return EXIT_FAILURE;
        }
        else if (strncmp(expr+i, "||", 2) == 0) {
            expr[i] = '\0';
            if (parseexpr(expr) != EXIT_SUCCESS)
                return parseexpr(expr+i+2);
            else
                return EXIT_SUCCESS;
        }
    }
    
    /**** 3. BASE: expr is a cmd ****/
    char **curcmd;
    length = splitexpr(expr, &curcmd);      // split the expression, using space as a delimiter
    rv = parsecmd(curcmd, length);
    printdebug("parseexpr: expr evaluated with return value %d", rv);
    return rv;
}

/*
 * resolvebrackets helper function: recursively resolve any subexpression *directly* following an opening '(' left bracket
 *  returns exit status (EXIT_SUCCESS || !EXIT_SUCCESS) of executed expression or -1 if no brackets where evaluated.
 */
int resolvebrackets(char *expr, int length) {
    char *l = strchr(expr,'('); // pointer to first '('
    if (l == NULL || l > expr) // '(' must be at the start of expr
        return -1;
    
    int i, rv, count = 0;
    char *r = NULL;
    // 1. find pointer *r to matching ')'
    for (i = 1; i < length; i++)
        if (expr[i] == '(')
            count++;
        else if (expr[i] == ')') {
            if (count == 0) {
                r = expr + i;
                break;
            }
            else
                count--;
        }
    
    if (r == NULL) {
        printerr("parse errror: unbalanced parenthesis when evaluating '%s'", expr);
        return EXIT_FAILURE;
    }
        
    // 2. (recursively) parse the expression between brackets
    *r = '\0';
    printdebug("resolvebrackets: now evaluating '%s'", l+1);
    rv = parseexpr(l+1);
        
    /* 3. parse the remainder of the expression, replacing the evaluated subexpression with 
    its built-in truth value (T | F), using memmove for overlapping memory
        [--> no buf overflow, since at least 2 chars '(' and ')' are replaced with a single 'T' or 'F' char]
    */
    *l = RESOLVE_TRUTH_VAL(rv);
    memmove(l + 1, r + 1, strlen(r+1)+1); // len+1 : also copy the '\0'
    printdebug("resolvebrackets: input resolved to '%s'", l);
    return parseexpr(l);
}

/*
 * splitexpr helper function: splits a '\0' terminated input string *expr, using space as a delimiter. Note spaces can be '\ ' escaped.
 *  initializes the provided pointer ***ret to point to a NULL terminated array of pointers to *expr space-delimited-substrings
 *  returns curcmd array length
 *  TODO DONE NOTE: this funtion expects an expr without redundant spaces (as is converted by resolvealiases() for example)
 */
int splitexpr(char *expr, char ***ret) {
    static char **curcmd = NULL;        // array of pointers to current cmd and its arguments
    static int curcmd_realloc = 0;      // total nb of reallocations for curcmd
    int j = 0;                          // index in curcmd[] array
    int length = strlen(expr);
    
    #define CMD_OPT_ALLOC_UNIT  10      // unit of re-allocation for curcmd[]
    #define REALLOC_CURCMD \
        if (!(curcmd = realloc(curcmd, sizeof (char*) * CMD_OPT_ALLOC_UNIT * ++curcmd_realloc))) { \
            printf("FAILED"); \
            printerrno("Running out of memory. Exiting"); \
            exit(EXIT_FAILURE); \
        }
    
    // 0. allocate init space on the heap
    if (!curcmd_realloc)
        REALLOC_CURCMD;
    
    // 1. skip all leading spaces
    int i = strspn(expr, " ");
    char *ch = expr + i;             // ch is pointer to the next token to add
        
    // 2. split using space as a delimiter
    for (; i < length; i++) {
        // allow escaping (i.e. skipping) the next char
        #define CHK_ESCAPING(index) \
            if (expr[index] == '\\' && index < length-1) { \
                printdebug("escaping char '%c' in '%s'", expr[index+1], ch); \
                memmove(expr+index, expr+index+1, strlen(expr+index+1) + 1); \
                length--; \
                index++; \
            }
            
        CHK_ESCAPING(i)
        if (expr[i] == '"') { //TODO TODO doc -> protect content
            expr[i] = '\0';
            if (ch < expr + i)
                curcmd[j++] = ch; //TODO realloc??
            ch = expr + i + 1;
            int k;
            bool found = false;
            for (k=i+1; k < length && !found; k++) {
                CHK_ESCAPING(k)
                if (expr[k] == '"') {
                    expr[k] = '\0';                
                    curcmd[j++] = ch; //TODO realloc??
                    ch = expr + k + 1;
                    found = true;
                }
            }
            if (!found)
                printerr("parse errror: unbalanced quoting -> added end quotes \"%s\"...", ch);
            i = k-1;
        }
        else if (expr[i] == ' ') {
            expr[i] = '\0';
            if (ch < expr + i)    //TODO TODO doc
                curcmd[j++] = ch;
            ch = expr + i + 1;
            // realloc curcmd[] if needed
            if (j >= CMD_OPT_ALLOC_UNIT * curcmd_realloc)
                REALLOC_CURCMD;
        }
    }
    
    if (j == 0 || *ch) // ignore trailing spaces
        curcmd[j++] = ch; // add trailing token        
    curcmd[j] = NULL;
    
    *ret = curcmd;  // set provided pointer
    return j;       // return curcmd array length
}

/*
 * parsecmd: parses the space delimited cmd[lenght] array, according to the 'cmd' grammar.
 *  returns exit status (EXIT_SUCCESS || !EXIT_SUCCESS) of executed cmd
 */
int parsecmd(char **cmd, int length) {
    comd *pipeline_head = createcomd(cmd);
    comd *pipeline_tail = pipeline_head;
    
    #define CHK_FILE(op) \
        if (i >= length - 1) { \
            printerr("parse error: no file specified after redirection operator '%s'", op); \
            return EXIT_FAILURE; \
        }
    
    int i, nbpipes;
    for (i = 0, nbpipes = 0; i < length; i++) {
        if (*cmd[i] == '|') {
            cmd[i] = NULL;
            pipeline_tail->length = i;
            comd *new = createcomd(cmd+i+1);
            pipeline_tail->next = new;
            pipeline_tail = new;
            nbpipes++;
        }
        else if (*cmd[i] == '<') {
            CHK_FILE("<")
            cmd[i++] = NULL;
            pipeline_tail->inf = cmd[i];
        }
        else if (strncmp(cmd[i], ">>", 2) == 0) {
            CHK_FILE(">>")
            cmd[i++] = NULL;
            pipeline_tail->outf = cmd[i];
            pipeline_tail->append_out = 1;
        }
        else if (strncmp(cmd[i], "2>", 2) == 0) {
            CHK_FILE("2>")
            cmd[i++] = NULL;
            pipeline_tail->errf = cmd[i];
        }
        else if (*cmd[i] == '>') {
            CHK_FILE(">")
            cmd[i++] = NULL;
            pipeline_tail->outf = cmd[i];
        } 
    }
    // base: pipeline of comds
    return execute(pipeline_head, nbpipes);
}

/*
 * execute: execute a list of comds as a pipeline, using fork and exec.
 *  specified number of pipes npipes  = (length of pipeline - 1)
 *  returns the exit status (EXIT_SUCCESS || !EXIT_SUCCESS) of the last process in the pipeline
 *
 * Note: - pipe redirecting has priority over explicit redirecting: 
 *          e.g. ls > out | less : ls stdout will *only* be directed to less
 */
int execute(comd *pipeline, int npipes) {
    int i, pfds[npipes*2];
    // 0. create n pipes and store the file descriptors
    for (i = 0; i < npipes; i++)
        if (pipe(pfds+i*2) < 0) {
            printerrno("Couldn't create pipe");
            exit(EXIT_FAILURE);
        }
    #define CLOSE_ALL_PIPES \
        for (k = 0; k < npipes*2; k++) \
            if (pfds[k] != -1 && close(pfds[k]) == -1) \
                printerrno("couldn't close pipefd %d", pfds[k]);
    #define CLOSE_PREV_PIPE \
        if (stdoutfd != -1 && close(stdoutfd) == -1) \
            printerrno("couldn't close writing end with pipefd %d", pfds[k]); \
        else \
            pfds[j+1] = -1; // to indicate this side is closed
    
    comd *cur = pipeline;
    int j, k, status = 0, nbchildren = 0;
    /* 1. fork nbchildren = (npipes + 1 - nbuiltins) child processes and connect them to the pipes
        NOTE: each iteration: close the writing end of the prev pipe to indicate the parent process (jsh)
        won't use it anymore; otherwise, the next process (built_in) in the pipeline won't receive the EOF...*/
    for (i = 0, j = 0; i <= npipes; i++, j+=2, cur = cur->next) {
        /**** pipe stdin iff not first cmd; stdout iff not last cmd ****/
        int stdinfd = (i > 0)? pfds[j-2] : -1;
        int stdoutfd = (i < npipes)? pfds[j+1] : -1;
        
        //TODO TODO
        //*cur->cmd = resolvealiases(*cur->cmd);
        
        /**** try to execute cur as a built_in ****/
        if ((status = exec_built_in(cur, stdinfd, stdoutfd)) != -1) {
            printdebug("built-in: executed '%s'", *cur->cmd);
            CLOSE_PREV_PIPE
            continue;
        }

        /**** cur is not a built-in; fork a child process ****/
        nbchildren++;
        pid_t pid = fork();
        if (pid == -1) {
            printerrno("Creation of child process failed. Exiting");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {
            // ######## child process execution: redirect streams, setup pipe and execv ########
            printdebug("fork: now executing '%s'", *cur->cmd);
            I_AM_FORK = 1;
            
            redirectstreams(cur, stdinfd, stdoutfd);
            CLOSE_ALL_PIPES; // no longer needed
            // TODO use exevp to auto search for the cmd, using the PATH env
            if (execvp(*cur->cmd, cur->cmd) < 0) {
                printerrno("couldn't execute command '%s'", *cur->cmd); //TODO here no color since !(is_interactive)...
                exit(EXIT_FAILURE);
            }
        }
        // ######## parent process execution: continue loop ########
        CLOSE_PREV_PIPE
    }
    // ######## continued parent process execution: wait for children completion ########
    CLOSE_ALL_PIPES; // close all remaining open pipe fds; no longer needed

    // wait for children completion
    WAITING_FOR_CHILD = true;
    int statuschild = 0;
    for (k = 0; k < nbchildren; k++) {
        wait(&statuschild);
        printdebug("waiting completed: child %d of %d", k+1, nbchildren);
    }
    WAITING_FOR_CHILD = false;
    
    // free() the comd list
    freecomdlist(pipeline);
    
    // return status of last process in the pipeline
    status = ((status == -1)? statuschild: status);
    return ((WIFEXITED(status)? WEXITSTATUS(status): WTERMSIG(status))); //TODO WIFSTOPPED
    
    /*int rv;
    if ( WIFSIGNALED(status) ) {
        rv = WTERMSIG(status);
        printdebug("child terminated with value: %d", rv);
    }
    else if (WIFEXITED(status)) {
        rv = WEXITSTATUS(status);
        printdebug("child exited with value %d", rv);
    }
    else {
        printdebug("child stopped how???");
        exit(EXIT_FAILURE);
    }
    return rv;
    
    if (WIFSTOPPED(status))
        printf("Child received SIGTSTP");
    */
}

/*
 * exec_built_in: try to execute the provided *comd as a built_in shell command;
 *  wrapper for parse_built_in(), redirecting and restoring std streams if needed
 *  the argument stdinfd and stdoutfd are file descriptors for the pipeline if any; else -1
 *  returns -1 if command not recognized as a built_in; 
 *  else returns exit status (EXIT_FAILURE || EXIT_SUCCESS) of executed built_in
 */
int exec_built_in(comd *comd, int stdinfd, int stdoutfd) {
    int i = is_built_in(comd);
    if (i == -1)
        return -1;
    
    // redirect std streams, parse built_in and restore std streams
    int saved_stdin = dup(STDIN_FILENO);        
    int saved_stdout = dup(STDOUT_FILENO);
    redirectstreams(comd, stdinfd, stdoutfd);
    int rv = parse_built_in(comd, i);
    REDIRECT_STR(saved_stdin, STDIN_FILENO);
    REDIRECT_STR(saved_stdout, STDOUT_FILENO);
    close(saved_stdin);
    close(saved_stdout);
    
    return rv;
}

/*
 * redirectstreams: redirect stdin, stdout, stderr as specified in the specified comd struct and 
 *  stdinfd/stdoutfd arguments: specifying the file descriptors for the pipeline if any; else -1
 *  note: pipe redirection has priority over explicit redirection
 *  on failure, prints error message and exit(EXIT_FAILURE)
 */
void redirectstreams(comd *cmd, int stdinfd, int stdoutfd) {
    if (cmd->inf != NULL) {
        printdebug("redirecting stdin to file '%s'", cmd->inf);
        int fd = open(cmd->inf, O_RDONLY);
        if(fd < 0) {
            printerrno("error opening file '%s'", cmd->inf);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd); // no longer needed
    }
    if (cmd->outf != NULL) {
        printdebug("redirecting stdout to file '%s'", cmd->outf);    
        int fd;
        if (cmd->append_out)
            fd = open(cmd->outf, O_WRONLY | O_CREAT | O_APPEND, 0666); //rw-rw-rw; will be combined with current umask
        else
            fd = open(cmd->outf, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if(fd < 0) {
            printerrno("error opening file '%s'", cmd->outf);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    if (cmd->errf != NULL) {
        printdebug("redirecting stderr to file '%s'", cmd->errf);    
        int fd = open(cmd->errf, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if(fd < 0) {
            printerrno("error opening file '%s'", cmd->errf);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
    // piping has priority: override prev redirections if any
    if (stdinfd != -1) {
        printdebug("redirecting stdin to pipefd %d", stdinfd);  
        REDIRECT_STR(stdinfd, STDIN_FILENO);
    }           
    if (stdoutfd != -1) {
        printdebug("redirecting stdout to pipefd %d", stdoutfd);        
        REDIRECT_STR(stdoutfd, STDOUT_FILENO);
    }
}

/* 
 * is_valid_cmd: returns whether or not an occurence of a cmd string is valid in a given 
 *  context string. An cmd is valid iff it occurs as a 'comd' in the grammar.
 *
 *  @param cmd: the command string to be checked
 *  @param context: the context string where the command occurs
 *  @param i: the index in the context string where the command occurs: context+i equals cmd
 */
bool is_valid_cmd(const char* cmd, const char* context, int i) {
    #if ASSERT
        assert(strncmp(context+i, cmd, strlen(cmd)) == 0);
    #endif
    
    // check the context following the cmd occurence
    const char *after = context + i + strlen(cmd);
    bool after_ok = (*after == ' ' || *after == '\0' || *after == '|' || *after == ';' || *after == ')' ||
        (strncmp(after, "&&", 2) == 0) || (strncmp(after, "||", 2) == 0));
    
    if (!after_ok)
        return false;
    
    // check the context preceding the alias occurence
    const char *before = context + i;
    int index_before = i;
    while (index_before > 0 && (*(before-1) == ' ' || *(before-1) == '(')) {
        before--;
        index_before--;
    }
    
    bool before_ok = ( index_before == 0 || (*(before-1) == '|' || *(before-1) == ';') || 
        (index_before >= 2 && (strncmp(before-2, "&&", 2) == 0 || strncmp(before-2, "||", 2) == 0))
        || (index_before >= 4 && strncmp(before-4, "sudo", 4) == 0));
    
    return before_ok;
}
