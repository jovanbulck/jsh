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

#include "alias.h"

#define MAX_ALIAS_VAL_LENGTH    200 // the maximum allowed number of chars per alias value
#define MAX_ALIAS_KEY_LENGTH    50  // the maximum allowed number of chars per alias key

struct alias {
    char *key;
    char *value;
    struct alias *next;
};
typedef struct alias ALIAS;
//#define CREATE_ALIAS(k,v) (struct alias) {k, v}

ALIAS *head = NULL;
ALIAS *tail = NULL;

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
    ALIAS *new = malloc(sizeof(alias)); //TODO chkerr
    new->next = NULL;
    new->key = malloc(sizeof (char) * keylength+1);
    
    // copy the provided strings into the allocated memory
    strncpy(new->key, k, keylength+1);
    new->value = val;
    
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
TODO doc
*/
int unalias(char *key) {
    ALIAS *cur = head;
    ALIAS *prev = NULL;
    int found = 0;
    while (cur != NULL && !found) {
        if (strncmp(cur->key, key, MAX_ALIAS_KEY_LENGTH) == 0) {
            found = 1;
            if(prev)
                prev->next = cur->next;
            else
                cur = NULL;
        }
        prev = cur;
        cur = cur->next;
    }
    return EXIT_SUCCESS;
}

/*
 * printaliases: print a list of all currently set aliases on stdout
 * returns EXIT_SUCCESS
 */
int printaliases() {
    ALIAS *cur = head;
    while(cur != NULL) {
        printf("alias %s = '%s'\n", cur->key, cur->value);
        cur = cur->next;
    }
    return EXIT_SUCCESS;
}


/*
 * resolvealiases: substitutes all known aliases in the inputstring. Returns a pointer to the substituted string.
 *  NOTE: this function returns a pointer to a newly malloced() string. The caller should free() it afterwards, 
 *        as well as also the inputstring *s, if needed
 *
 *  current limitations for aliases:
 *      - alias resolving is done before expression parsing -> if the name of an alias is used as an argument, the argument will also be expanded
 *      - aliases must be space delimited (e.g. echo jo &&clr won't work)
 *      - any spaces in the value must be escaped in the input for the 'alias' cmd    e.g. alias ls ls\ --color=auto
 * TODO allow escaping eg \~ ??
 */
char *resolvealiases(char *s) {
    int maxsize = strlen(s) + total_alias_val_length;   // the max size of the resolved input string s
    char *ret = malloc(sizeof (char) * maxsize);        // alloc space for the return value
    ret[0] = '\0';
    
    // split the supplied string using space as a delimiter
    char *a[sizeof (char) * maxsize];
    char *c = strtok(s, " ");
    if (c == NULL) // s consists of only spaces
        return strcpy(ret, "");
    int i = 0;
    do {
        a[i++] = c;
    } while ((c = strtok(NULL, " ")));
    
    #define CAT_SPACE \
        if (j < i-1) \
            strcat(ret, " ");
    
    int j, k, found;
    for (j = 0; j < i; j++) {
        found = 0;
        ALIAS *cur;
        for (cur = head; cur != NULL && !found; cur = cur->next) {
             if (strcmp(a[j], cur->key) == 0) {
               found = 1;
               strcat(ret, cur->value);
               CAT_SPACE;
            }
        }
        // no matching alias: copy unaltered
        if (!found) {
            strcat(ret, a[j]);
            CAT_SPACE;
        }
    }
    
    printdebug("alias: input resolved to: '%s'", ret);
    return ret;
}
