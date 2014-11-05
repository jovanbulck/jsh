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

#include "jsh-completion.h"

// #################### helper generator function definitions ####################
char *jsh_cmd_generator(const char*, int);
char *jsh_built_in_generator(const char*, int);
char *jsh_alias_generator(const char*, int);
char *jsh_external_cmd_generator(const char*, int);
char *git_completion_generator(const char*, int);
char *git_branch_completion_generator(const char*, int);
char *apt_compl_generator(const char*, int);
char *jsh_options_generator(const char*, int);
char *debug_completion_generator(const char*, int);
char *make_options_generator(const char*, int);

/*
 * The function that is called by readline; returns a list of matches or NULL iff no matches
 *  found.
 * @note: All custom autocompletion generators should be called from withing this function.
 */
char** jsh_command_completion(const char *text, int start, int end) {
    char **matches = NULL;
    
    // true iff user entered 'cmd text<TAB>'
    #define USR_ENTERED(cmd) \
        (start >= strlen(cmd)+1 && (strncmp(rl_line_buffer + start - strlen(cmd) - 1, \
        cmd, strlen(cmd)) == 0)) // +1 for space
     
    if (is_valid_cmd(text, rl_line_buffer, start)) {
        // try custom cmd autocompletion iff this is a valid 'comd' context
        matches = rl_completion_matches(text, &jsh_cmd_generator);
    }
    else {
        // else try custom autocompletion for specific commands
        if (USR_ENTERED("git")) {
            matches = rl_completion_matches(text, &git_completion_generator);
        }
        else if (USR_ENTERED("git checkout") || USR_ENTERED("git branch") || 
         USR_ENTERED("git merge")) {
            char *pwd_is_git = strclone("git rev-parse --git-dir > /dev/null 2> /dev/null");
            if (parseexpr(pwd_is_git) == EXIT_SUCCESS)
                matches = rl_completion_matches(text, &git_branch_completion_generator);
            free(pwd_is_git);
        }
        else if (USR_ENTERED("jsh")) {
            matches = rl_completion_matches(text, &jsh_options_generator);
        }
        else if (USR_ENTERED("make")) {
             matches = rl_completion_matches(text, &make_options_generator);
        }
        else if (USR_ENTERED("debug")) {
            matches = rl_completion_matches(text, &debug_completion_generator);
        }
        else if (USR_ENTERED("apt")) {
            matches = rl_completion_matches(text, &apt_compl_generator);
        }
    }
        
    return matches;
}

// #################### helper generator function implementations ####################

/*
 * RL COMPLETION GENERATOR FUNCTION JSH DEVELOPER INFO :
 * 
 * 1. a GNU readline generator function is a function that is passed a partially entered
 *      command several times and returns a single possible match each time. The first time
 *      it is called for a new partially entered command, @param(int state) is zero. The 
 *      generator should then initialize static state and return the matches one by one.
 *      When no more matches, return NULL.
 * 2. a jsh generator function is called from the "jsh_command_completion()" function above,
 *      using the "rl_completion_matches()" readline helper function
 * 3. For a straightforward (static string array) completion generator, one should use the
 *      COMPLETION_SKELETON macro, see examples below.
 *
 * In other words (from the readline doc):
 *   "text is the partial word to be completed. state is zero the first time the function 
 *   is called. The generator function returns (char *) NULL to inform rl_completion_matches() 
 *   that there are no more possibilities left. Usually the generator function computes the
 *   list of possible completions when state is zero, and returns them one at a time on 
 *   subsequent calls. Each string the generator function returns as a match must be
 *   allocated with malloc();  Readline frees the strings when it has finished with them."
 */

/*
 * COMPLETION_SKELETON: a sketleton to facilitate the implementation of a custom completion
 *  generator for GNU readline.
 * @arg array       : a char* array with possible commands
 * @arg nb_elements : the length of @param(array)
 */
#define COMPLETION_SKELETON(array, nb_elements) \
    do { \
        static int len; \
        static int index; \
        \
        if (!state) { \
            index = 0; \
            len = strlen(text); \
        } \
        \
        while (index < nb_elements) \
            if (strncmp(array[index], text, len) == 0) \
                return strclone(array[index++]); \
            else \
                index++; \
        \
        return NULL; \
        } \
    while (0)

/*
 * jsh_cmd_generator: a readline generator that combines several helper generators so that 
 *  results from these generators are combined in a single generator.
 */
char *jsh_cmd_generator(const char *text, int state) {
    // a separate state for each helper generator, so that it starts from zero
    static int alias_state = 0;
    static int built_in_state = 0;
    static int unix_state = 0;
    
    if (!state) {
        alias_state = 0;
        built_in_state = 0;
        unix_state = 0;
    }
    
    char *rv;
    // jsh grammar priority: alias > built_in > external_cmd
    if (!(rv = jsh_alias_generator(text, alias_state++)))
        if (!(rv = jsh_built_in_generator(text, built_in_state++)))
            rv = jsh_external_cmd_generator(text, unix_state++);
    return rv;
}

/*
 * jsh_alias_generator: a readline generator that returns matches with jsh aliases.
 */
char *jsh_alias_generator(const char *text, int state) {
    static char **alias_keys = NULL;
    static unsigned int nb_alias_keys;
    if (!state) {
        char **ret;
        if ((ret = get_all_alias_keys(&nb_alias_keys, true))) {
            if (alias_keys) {
                int i;
                for (i = 0; i < nb_alias_keys; i++)
                    free(alias_keys[i]);
                free(alias_keys);
            }
            alias_keys = ret;
        }
    }
    COMPLETION_SKELETON(alias_keys, nb_alias_keys); 
}

/*
 * jsh_built_in_generator: a readline generator that returns matches with jsh built_ins.
 */
char *jsh_built_in_generator(const char *text, int state) {
    COMPLETION_SKELETON(built_ins, nb_built_ins);
}

/*
 * jsh_external_cmd_generator: a readline generator that returns matches for non-jsh commands.
 *  TODO search $PATH at boot-time and save the directory content in an array...
 */
char *jsh_external_cmd_generator(const char *text, int state) {
    // hackhackhack: an array with some usefull commands
    const char *widely_used_cmds[] = {"git", "cat", "grep", "ls", "exit", "sudo", "kill", \
    "killall", "links", "find", "clear", "chmod", "echo", "make", "poweroff", "reboot", \
    "pacman", "aptitude", "apt-cache", "apt-get", "man", "nano", "vi", "gcc", "jsh", "zsh", \
    "bash"};
    #define nb_widely_used_cmds (sizeof(widely_used_cmds)/sizeof(widely_used_cmds[0]))
    
    COMPLETION_SKELETON(widely_used_cmds, nb_widely_used_cmds);
}

/*
 * git_completion_generator: a readline completor for git commands
 */
char *git_completion_generator(const char *text, int state) {
    static const char *git_cmds[] = {"add", "bisect", "branch", "checkout", "clone", \
    "commit", "diff", "fetch", "grep", "init", "log", "merge", "mv", "pull", "push", \
    "rebase", "reset", "rm", "show", "status", "tag"};
    static const int nb_elements = (sizeof(git_cmds)/sizeof(git_cmds[0]));
      
    COMPLETION_SKELETON(git_cmds, nb_elements);
}

/*
 * debug_completion_generator: a proof of concept readline completor for "jsh debug on/off"
 */
char *debug_completion_generator(const char *text, int state) {
    static const char *options[] = {"on", "off"};
    static const int nb_options = (sizeof(options)/sizeof(options[0]));
    
    COMPLETION_SKELETON(options, nb_options);
}

/*
 * make_completion_generator: a readline completion generator for GNU make
 */
char *make_options_generator(const char *text, int state) {
    static const char *options[] = { "--always-make", "--environment-overrides", \
    "--ignore-errors", "--keep-going", "install", "--no-keep-going", "--stop", "install", \
    "clean", "help"};
    static const int nb_elements = (sizeof(options)/sizeof(options[0]));
    
    COMPLETION_SKELETON(options, nb_elements);
}

/*
 * jsh_options_generator: a readline completion generator for "jsh --options"
 */
char *jsh_options_generator(const char *text, int state) {
    static const char *options[] = {"--nodebug", "--debug", "--color", "--nocolor", \
    "--norc", "--license", "--version", "--help"}; //TODO dont hardcode here --> put enum in jsh.c?
    static const int nb_options = (sizeof(options)/sizeof(options[0]));
    
    COMPLETION_SKELETON(options, nb_options);
}

/*
 * apt_compl_generator: a readline completion generator for "apt-get"
 */
char *apt_compl_generator(const char *text, int state) {
    static const char *options[] = { "list", "search", "show", "install", "remove", \
    "edit-sources", "update", "upgrade", "full-upgrade"};
    static const int nb_elements = (sizeof(options)/sizeof(options[0]));
    
    COMPLETION_SKELETON(options, nb_elements);
}

/*
 * git_branch_completion_generator : a readline completion generator for git branch names
 */
char *git_branch_completion_generator(const char *text, int state) {
    #define MAX_NB_BRANCHES 100     // hackhackhack TODO cleanup...
    #define MAX_BRANCH_NAME_LEN 100
    static char **branches = NULL;
    static int nb_elements = 0;
    
    if (!state) {
        if (branches) {
            int i;
            for (i = 0; i < nb_elements; i++)
                free(branches[i]);
            free(branches);
        }
        branches =  malloc((sizeof(char*) * MAX_NB_BRANCHES));
        FILE *fp = popen("git branch --no-color", "r");
        
        nb_elements = 0;
        int i;
        bool done = false;
        for (i= 0; i < MAX_NB_BRANCHES && !done; i++) {
            branches[i] =  malloc(MAX_BRANCH_NAME_LEN * sizeof(char));
            //if (!fgets(branches[i], MAX_BRANCH_NAME_LEN, fp)) break;
            char *cur = branches[i];
            int j = 0;
            int c;
            while (true) {
                int c = getc(fp);
                if (c == EOF) {
                    done = true;
                    cur[j] = '\0';
                    break;
                }
                if (c == '\n') {
                    cur[j] = '\0';
                    break;
                }
                else if (c != ' ' && c != '*') {
                    cur[j++] = c;
                }
            }
            
            nb_elements++;
        }
        
        pclose(fp);
        
        // TODO cleanup ; no code duplication + git procelain syntax?? for parsing  now remote branches
        fp = popen("git branch --no-color -r", "r");
        
        done = false;
        for (; i < MAX_NB_BRANCHES && !done; i++) {
            branches[i] =  malloc(MAX_BRANCH_NAME_LEN * sizeof(char));
            //if (!fgets(branches[i], MAX_BRANCH_NAME_LEN, fp)) break;
            char *cur = branches[i];
            int j = 0;
            int c;
            while (true) {
                int c = getc(fp);
                if (c == EOF) {
                    done = true;
                    cur[j] = '\0';
                    break;
                }
                if (c == '\n' || c == '>') {
                    cur[j] = '\0';
                    break;
                }
                else if (c != ' ' && c != '*' && c != '-') {
                    cur[j++] = c;
                }
            }
            
            nb_elements++;
        }
        
        pclose(fp);
    }
    
    COMPLETION_SKELETON(branches, nb_elements);
}
