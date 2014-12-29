# How to contribute

:+1::tada: First off, thanks for taking the time to contribute! :tada::+1:

*Developers, developers, developers*: here are some ways to help the `jsh` project:

## Bug reporting and feature requests

Reporting bugs is a great way to help the `jsh` project too, report your issues 
[here](https://github.com/jovanbulck/jo-shell/issues) 

Having a great idea to improve `jsh`? Shout [here](https://github.com/jovanbulck/jo-shell/issues)! 

## Testing

If you tested jsh on a new platform and have some extra instructions, add them 
[here](https://github.com/jovanbulck/jo-shell/wiki/Compiling-and-running) 

## Uploading binary Builds

When a new [release](https://github.com/jovanbulck/jo-shell/releases) comes out, it's great to add new binaries 
for various platforms. As we've had troubles with Github-releases binary builds becoming unavailable, 
I think it's best to open a pull request for the 
[`gh-pages` branch](https://github.com/jovanbulck/jo-shell/tree/gh-pages/releases) containing a back-up of all 
the releases binaries. I'll then add them to the github release page.

## Coding

Feel free to improve existing or implement new features. If you're working on an existing issue, claim it. 
If you're working on a major new feature, you'll best open and claim a new issue for it. 
One address for all your pull requests: https://github.com/jovanbulck/jo-shell/pulls

### Commit messages

- Write informative short commit messages describing the changes.
- Refer to issue and pull request `#numbers` where needed
- Consider starting the commit message with an applicable emoji:
    * :bug: `:bug:` when fixing a bug
    * :star: `:star:` when implemented a new feauture request
    * :hammer: `:hammer:` when fixing the Makefile
    * :penguin: `:penguin:` when fixing something for Linux
    * :apple: `:apple:` when fixing something for Mac
    * :memo: `:memo:` when writing docs
    * :lipstick: `:lipstick:` when cleaning up code
    * :racehorse: `:racehorse:` when improving performance
    * :lock: `:lock:` when dealing with security
    * :green_heart: `:green_heart:` when fixing the TravisCI build
    * :fire: `:fire:` when removing code or files
    * :ghost: `:ghost:` for non-finished proof-of-concept commits ;-)
    * :books: `:books:` when working on GNU `readline`
    * :bangbang: `:bangbang:` when working on `readline` history support
    * :shell: `:shell:` when working on the shell's core structure
    * :package: `:package:` for package / binary build related commits
    * :checkered_flag: `:checkered_flag:` for version number change commits

### Coding guidelines

*work in progress* This section lists some coding guidelines to keep the `jsh` code readable, uniform and clean. Note not all of `jsh`'s current code base confirms to these guidelines. In future, this should be the case however. Therefore all code in pull requests should confirm to these guidelines.

#### General layout guidelines
* Don't use tabs, use 4 spaces instead (set your texteditor to insert spaces instead of tabs)
* Line length shouldn't exceed column 90 - 95 (e.g. `gedit` can be configured to show a right margin)
* all code files should start with the GPL copyright notice, see e.g. `jsh.c`

#### C coding guidelines
* All code should be ANSI C compatible and compile without warnings with `gcc`
* The use of `libc` functions should be as much as possible POSIX compatible.
* Pay great attention to software security: buffer overflows, overreads, format string vulnerabilies, malloc leaks, ...
* Function defintions are formated as follows: `return_type function_name(arg_type1, arg_type2, ...);`
* 'Public' function defintions go into C header files.
* 'Private' helper function defintions are listed at the start of the C file.
* Put generic helper function definitions that can be re-used accross the code base, in `jsh-common.h`
* All functions should have doc. Write something as:
```c
 /*
  * function_name: short description, potentially mentioning @param(arg_name)
  * @arg arg_name         : info on the first argument
  * @arg longer_arg_name  : info on the second argument
  * @return: info on the returned value, if any
  * @note: additional info, if any
  */
  return_type function_name(arg_type1 arg_name, arg_type2 longer_arg_name)
```
* `if then else` and related syntax is formatted as follows:
```c
    if (condition)
        single_line_cmd;

    if (condition) {
        if_branch_cmds;
        on_multiple_lines;
    } else {
        else_cmds;
        multi_line;
    }
```
* Function calls are formatted as follows: `int a = call_a_function(some_param);`
* Choose wisely between `for` and `while` loops: keep readability, elegance and compactness in mind.
* Whenever possible, choose the `bool` datatype defined in `stdbool.h` over integers
* Don't use magic numbers; use `#define KEY value` instead.
* Multiline macro's should always go into a `do { .. } while(false)` loop: see e.g. [this rationale](http://stackoverflow.com/questions/154136/do-while-and-if-else-statements-in-c-c-macros)
