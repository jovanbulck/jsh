#include <stdio.h>
#include <string.h>

#define PROGRAM_NAME "mini-grep"	// the name of this program
#define MAXLINE 1000 			// max number of characters per line
#define COLOR   "\033[1;31m"		// 0 = normal, 1 = bold (increased intensity) ; 31 red
#define NONE   "\033[0m"		// to flush the previous color property

// option global variables
int EXCEPT = 0;
int NUMBER = 0;

// function definitions
int option(char *s);
int getLine(char s[]);
int strindex(char *s, char *p, char *r[]);
void printmatch(char *s, long no, char *p, char *i[], int n);

/*
 * Print all lines of stdin, matching a specified pattern.
 * Return number of matching lines.
 */
int main(int argc, char *argv[]) {
	char line[MAXLINE];
	char *index[MAXLINE];	//array of pointers to pattern substrings
	char *pattern;		//pattern to search for
	int i, n, nb_matches = 0;
	long lineno = 1;

	// process options
	for (i=1; i < argc && *argv[i] == '-'; i++)
		if (option(argv[i]+1) < 0)
			return 0;

	if (argc-i != 1)
		printf("Usage: %s [OPTION]... PATTERN\nTry '%s --help' for more information.\n", PROGRAM_NAME, PROGRAM_NAME);
	else {
		pattern = argv[i];
		// read all lines on stdin, printing those who match
		for(;getLine(line) > 0; lineno++)
			if ( ((n = strindex(line, pattern, index)) > 0) != EXCEPT) {
				printmatch(line, lineno, pattern, index, n);
				nb_matches += n;
			}
	}
	return nb_matches;
}

/*
 * option: process an option string
 * Negative return value = terminate program (i.e. don't start pattern search)
 */
int option(char *str) {
	int optionfull(char *s); // option helper function

	char *begin = str;
	while (*str != '\0')
		switch(*str++) {
			case '-':
				if (str-1 == begin)
					return optionfull(str);
				break; // else: ignore
			case 'h':
				printf("Usage: %s [OPTION]... PATTERN\n", PROGRAM_NAME);
				printf("Print all standard input lines containing PATTERN\n\n");
				printf("Recognized options:\n");
				printf("-h, --help\t\tdisplay this help message\n");
				printf("-n, --line-number\tprint line number with output lines\n");
				printf("-x, --except\t\tprint all lines except those matching PATTERN\n");
				return -1;
			case 'n':
				NUMBER = 1;
				break;
			case 'x':
				EXCEPT = 1;
				break;
			default:
				printf("Unrecognized option '-%c'\n", *(str-1));
				printf("Try '%s --help' for a list of regognized options\n", PROGRAM_NAME);
				return -1;
		}
	return 0;
}

/*
 * optionfull: process an option string, in full (--OPTION) notation
 * Negative retun value = terminate program (i.e. don't start pattern search)
 */
int optionfull(char *str) {
	if (strcmp(str,"help") == 0)
		return option("h");
	else if (strcmp(str, "line-number") ==0)
		return option("n");
	else if (strcmp(str, "except") == 0)
		return option("x");
	else {
		printf("Unrecoginized option '--%s'\n", str);
		printf("Try '%s --help' for a list of regognized options\n", PROGRAM_NAME);
		return -1;
	}
}

/*
 * getLine: read line on stdin into line array, return length.
 * Note: (in case inputline length > MAXLINE - 1, only the first (MAXLINE-1) characters
 * are read into the line array. The following characters of that line are read on the next function invocation.)
 */
int getLine(char line[]) {
	int c, i = 0;

	// read stdin into line array, until max or newline or EOF
	while( i < MAXLINE && (c=getchar()) != '\n'  && c != EOF )
		line[i++] = c;
	if ( c == '\n')
		line[i++] = c;
	line[i] = '\0'; // 0 terminate the string
	return i;
}

/*
 * strindex: store in r pointers to all occurences of p in s
 * return number of matches
 */
int strindex(char *s, char *p, char *r[]) {
	int j, n = 0;
	for (; *s != '\0'; s++) {
		for(j = 0; *(p+j) != '\0' && *(s+j) == *(p+j); j++)
			;
		if (j > 0 && *(p+j) == '\0') { // we have a full match
			r[n++] = s;
			s += j-1;
		}
	}
	return n;
}

/*
 * printmatch: print (in color) a null-terminated string s with line number no,
 * containing n pattern matches, specified by the index array.
 */
void printmatch(char *s, long no, char *pattern, char *index[], int n) {
	if (NUMBER)
		printf("%ld: ", no);

	for(; *s != '\0'; s++)
		if( s == *index && n > 0) { // we have a pattern match
			if (isatty(fileno(stdout)))
				printf("%s%s%s", COLOR, pattern , NONE);
			else
				printf("%s", pattern);
			index++;
			s += strlen(pattern) - 1;
			n--;
		}
		else
			printf("%c", *s);
}
