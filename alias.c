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

#include "alias.h"

#define MAX_ALIAS_VAL_LENGTH    200 // the maximum allowed number of chars per alias value
#define MAX_ALIAS_KEY_LENGTH    50  // the maximum allowed number of chars per alias key

struct alias {
    char *key;
    char *value;
    struct alias *next;
};
//typedef struct alias ALIAS;
//#define CREATE_ALIAS(k,v) (struct alias) {k, v}

struct alias *head = NULL;
struct alias *tail = NULL;

int total_alias_val_length = 0;

/*
 * alias: create a mapping between a key and value pair that can be resolved with resolvealiases().
 *  returns EXIT_SUCCESS or EXIT_FAILURE if something went wrong (e.g. malloc)
 *  (note that provided strings that are too long are silently truncated)
 */
int alias(char *k, char *v) {
    // allow recursive alias definitions
    char *val = resolvealiases(v);

    int vallength = strnlen(val, MAX_ALIAS_VAL_LENGTH);
    int keylength = strnlen(k, MAX_ALIAS_KEY_LENGTH);
    
    // alloc memory for the new alias struct and its key
    // note: *val has already been alloced by the resolvealiases() call
    struct alias *new = malloc(sizeof(struct alias)); //TODO chkerr
    new->next = NULL;
    new->key = malloc(sizeof (char) * keylength+1);
    
    // copy the provided strings into the allocated memory
    strncpy(new->key, k, keylength+1);
    new->value = val;
    
    if(alias_exists(k)) {
        unalias(k);
    }
	    total_alias_val_length += vallength;
    
	    if (head == NULL) {
	        head = new;
	        tail = head;
	    }
	    else {
	        tail->next = new;
	        tail = new;
	    }
    return EXIT_SUCCESS;
}

/*
 * unalias: unaliases a specified key
 *  returns EXIT_SUCCESS if the specified key was found; else prints an error message and returns EXIT_FAILURE 
 */
int unalias(char *key) {
    struct alias *cur = head;
    struct alias *prev = NULL;
    while (cur != NULL) {
        if (strncmp(cur->key, key, MAX_ALIAS_KEY_LENGTH) == 0) {
            // re-organize the linked list
            if (prev)
                prev->next = cur->next;
            else
                head = cur->next;
            if (cur == tail)
                tail = prev;
            // free the unalised alias and return
            total_alias_val_length -= strnlen(cur->value, MAX_ALIAS_VAL_LENGTH);
            free(cur);
            return EXIT_SUCCESS;;
        }
        prev = cur;
        cur = cur->next;
    }
    printerr("unalias: no such alias key: %s", key);
    return EXIT_FAILURE;
}

/*
 * printaliases: print a list of all currently set aliases on stdout
 * returns EXIT_SUCCESS
 */
int printaliases() {
    struct alias *cur = head;
    while(cur != NULL) {
        printf("alias %s = '%s'\n", cur->key, cur->value);
        cur = cur->next;
    }
    return EXIT_SUCCESS;
}


/*
 * resolvealiases: substitutes all known aliases in the inputstring. Returns a pointer to the alias-expanded string.
 *  NOTE: this function returns a pointer to a newly malloced() string. The caller should free() it afterwards, 
 *        as well as also the inputstring *s, if needed
 *
 *  current limitations for aliases:
 * TODO - any spaces in the value must be escaped in the input for the 'alias' cmd    e.g. alias ls ls\ --color=auto
 *                                                                                    alt syntax: alias ls "ls --color=auto"
 */
char *resolvealiases(char *s) {
    bool is_valid_alias(struct alias*, char*, int); // helper function def

    // alloc enough space for the return value
    int maxsize = strlen(s) + total_alias_val_length; 
    char *ret = malloc(sizeof (char) * maxsize);
    strcpy(ret, s);
    
    // find all alias key substrings, replacing them if valid in context
    struct alias *cur;
    char *p, *str; // p is pointer to matched substring; str is pointer to not-yet-checked string
    for (cur = head; cur != NULL; cur = cur->next)
        for (str = ret; (p = strstr(str, cur->key)) != NULL;) {
            if (is_valid_alias(cur, ret, p-ret)) {
                printdebug("alias: '%s' VALID in context '%s'", cur->key, p);
                
                char *after = p + strlen(cur->key);
                memmove(p + strlen(cur->value), after, strlen(after)+1); // overlapping mem; len+1 : also copy the '\0'
                memcpy(p, cur->value, strlen(cur->value)); // non-overlapping mem
                str = p+strlen(cur->value);
                }
            else {
                printdebug("alias: '%s' INVALID in context '%s'", cur->key, p);
                str = p+strlen(cur->key);
            }
        }
    
    printdebug("alias: input resolved to: '%s'", ret);
    return ret;
}

/*
 * is_valid_alias: returns whether or not an occurence of an alias key is valid (i.e. must be 
 *  replaced by its value) in a given context string. An alias match is valid iff it occurs 
 *  as a comd in the grammar and it's not '\' escaped.
 *
 *  @param alias: the alias struct corresponding to the alias that is matched
 *  @param context: the context string where the alias is matched
 *  @param i: the index in the context string where the alias is matched: context+i equals alias->key
 */
bool is_valid_alias(struct alias *alias, char *context, int i) {
    #if ASSERT
        assert(strncmp(context+i, alias->key, strlen(alias->key)) == 0);
    #endif
    
    // allow escaping '\' of aliases
    if ( i > 0 && *(context+i-1) == '\\' ) {
        printdebug("alias: escaping '%s'", alias->key);
        memmove(context+i-1, context+i, strlen(context+i)+1);
        return false;
    }
    
    // built_in aliases are valid in any context
    if (*alias->key == '~')
        return true;
    
    // check the context following the alias occurence
    char *after = context + i + strlen(alias->key);
    bool after_ok = (*after == ' ' || *after == '\0' || *after == '|' || *after == ';' || *after == ')' ||
        (strncmp(after, "&&", 2) == 0) || (strncmp(after, "||", 2) == 0));
    
    if (!after_ok)
        return false;
    
    // check the context preceding the alias occurence
    char *before = context + i;
    int nb_before = i;
    while (nb_before > 0 && (*(before-1) == ' ' || *(before-1) == '(')) {
        before--;
        nb_before--;
    }
    
    bool before_ok = ( nb_before == 0 || (*(before-1) == '|' || *(before-1) == ';') || 
        (nb_before >= 2 && (strncmp(before-2, "&&", 2) == 0 || strncmp(before-2, "||", 2) == 0)));
    
    return before_ok;
}

/*
bool is_valid_alias(struct alias *alias, char *context) {
    if (*context == '\0' || *alias->key == '~')
        return true;
    
    char *contextend = context + strlen(context);   
    char prev = *(contextend-2);   // the first non-space character preceding the alias in the context string
    char* prevprev = (contextend-3);
    bool ret = (prev == '|' || prev == ';' || strncmp(prevprev,"&&", 2) == 0 || strncmp(prevprev,"||", 2) == 0);
    printdebug("is_valid_alias: '%s', %d", alias->key, ret);
    return ret;
}*/

/*
 * @return true if the supplied alias already exists.
 **/
bool alias_exists(char* key) {
	if(head == 0) return false;

	bool keepLooping = true;
	struct alias *curAlias = head;
	
    while(keepLooping) {
		// If we've finished looping through all the aliases without a problem, return false.
		if(curAlias == 0) return false;
		
		// If the current alias matches one we're trying to define, return true.
		if(strcmp(curAlias->key, key) == 0)	return true;
		
		// update curAlias
		curAlias = curAlias->next;
	}
}

/*
 * Use to update existing aliases. The alias with the supplied key will be reassigned
 * so that it'll have the supplied value instead of it's current value.
 *
 * @return true if reassignment succeeded. False if it failed.
 **/
bool existing_alias_update(char* key,char* value) {
	if(head == 0) return false;

	struct alias *curAlias = head;
    struct alias *prevAlias = head;
    struct alias *newAlias = malloc(sizeof(struct alias));
    newAlias->key = malloc(strlen(key) + 1);
    strncpy(newAlias->key, key, strlen(key) + 1);
    newAlias->value = malloc(strlen(value) + 1);
    strncpy(newAlias->value, value, strlen(value) + 1);
    newAlias->next = 0;

	while(true) {
        // if no matching aliases are found, return false.
		if(curAlias == 0) return false;
		
		// If the current alias matches one we're trying to define, return true.
		if(strcmp(curAlias->key, key) == 0)	{
			prevAlias->next = newAlias;
            newAlias->next = curAlias->next;
            free(curAlias);
			return true;
		}
		
		// update curAlias
        prevAlias = curAlias;
		curAlias = curAlias->next;
	}
}
