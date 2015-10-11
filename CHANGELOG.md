# Changelog for `jsh`

This page lists all changes made to the `jsh` source code since [the latest release](https://github.com/jovanbulck/jsh/releases/latest). On commiting changes to the `master` branch, this page should be updated.

## Changes to be included in the next release (now in `master`)

work towards a new release:

#### prompt customizing (issue #3 ):
prompt expansion options are fully documented in the `man` page. Short summary below:

- coloring options for prompt:
   - %B turns on bold; %n restores normal
   - %f{color_name} enables foreground text coloring
   - %F{color_name} enables bold foreground text coloring
   - %b{color_name} enables background coloring
- various new prompt expansion options : 
   - %U colors user name red and bold iff sudo activated
   - %$ includes a '$' char or a '#' char iff sudo access is activated
   - %S includes the return value of the last executed shell command, colored red and bold iff non-zero
- directory expansion %d enhancements:
   - truncate the directory to the first char *after* the first '/' within the cwd (else it looks like the root)
   - replace the current user's home directory with '~' in %d prompt expansion

#### git support in prompt:
 - %g includes the git branch name iff the current working directory is a git repository
 - %c includes  a  bold  and red '*' char iff the current working directory is a git repository and git indicates files have changed since the last commit

#### technical things: 
-  preprocessing of the prompt color options for max efficiency
- fixed a bug to allow alias expansion when 'sourcing' files

## Changes for release 1.2.1

jsh 1.2.1 fixes two minor bugs in the v1.2.0 release below:

- Makefile fix for Mac OS X (defined empty variable)
- `source` built_in bugfix allowing parsing of a file with `jsh` alias expansion

## Changes for release 1.2.0

`jsh` 1.2.0 introduces some awesome new features on the user interface side: ;-)

*  Major new feature: context-sensitive custom GNU **readline completion**:
  - built_ins, aliases and a number of predefined UNIX commands
  - some custom completion for some specific UNIX commands: `git, apt-get, make, jsh, ...`
* wrote a `dialog` based sh **installer shell script**, as discussed in issue #55 
* added a **jsh_logout file**, containing shell commands to be executed at logout of an interactive session
* a new `source` built_in to interpret a given file line per line
* fixed the ugly EOF (^D) exiting by outputting an extra newline iff EOF

This release also includes some work on the more technical side:

* **Makefile** enhancements:
   - now automatically builds a version string, containing info of the machine
   - auto-generate a man page with the filled in version number and data (using `sed`)
   - added a `make release` target for making releases with a release version string
   - various compile flags used by the installer
* `alias` bugfix allowing redefinition of an alias
* partially started the source code re-organizing into cohesive modules, as discussed in issue #46 

## Changes for release 1.1.1

* Format string vulnerability patch for the v1.1.0 release.

Details: applied a format string vulnerability patch to the parsestream functions; passing `printf` may introduce format string vulnerabilities; one should pass the helper function `printf_verbatim` instead

## Changes for release 1.1.0

* A minor improvement to the first stable jsh release: added a `--version` option to query the current version number.

## Changes for release 1.0.0

The first stable `jsh` release! An overview of the major features:

* `bash`-like shell grammar with support for:
  * logical expressions: `&& || ; ( ) # ""`
  * stream redirection: `< > >> 2> |`
  * commands with space delimited options: lookup using the `PATH` environment variable
  * alias substitution
  * a number of special shell `built_in` commands: `T F alias cd color debug exit history prompt shcat unalias`
* inputline editing and history using `GNU readline`
* persistent configuration using `~/.jshrc` configuration file
* `jsh` command line options
* compilation and installation of a `man` page using a Makefile
* tested on Linux (Arch, Ubuntu, Tinycore) and Mac OS X
