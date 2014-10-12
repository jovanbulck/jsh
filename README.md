```
                       _                      __           __ __
                      (_)____          _____ / /_   ___   / // /
                     / // __ \ ______ / ___// __ \ / _ \ / // / 
                    / // /_/ //_____/(__  )/ / / //  __// // /  
                 __/ / \____/       /____//_/ /_/ \___//_//_/   
                /___/                                           
```

A proof-of-concept UNIX shell implementation.

`jsh` is free software and makes use of the `GNU Readline` library for input line editing and history.

Configuration files:
 * `~/.jshrc`: file containing commands to be executed at login
 * `~/.jsh_history`: containing the command history auto loaded and saved at login/logout
 * `~/.jsh_login`: file containing the ASCII welcome message auto printed at login of an interactive session

Supported options:
* -h, --help	display the help message
* -d, --debug	turn printing of debug messages on
* -n, --nodebug	turn printing of debug messages on
* -c, --color	turn coloring of jsh output messages on
* -o, --nocolor	turn coloring of jsh output messages off
* -f, --norc	disable autoloading of the ~/.jshrc file
* -l, --license	display licence information

The following grammar is currently supported.

```
 input  :=    expr &              // input is the unit of explicit backgrounding TODO not possible: eg  sleep 2 && echo done &
              expr

/*
TODO:   sleep 2 & ; echo hi
        built_ins e.g. cd &     --> don't support??
        brackets won't be supported: e.g. (sleep 2 && ls | grep j) & will resolve to T &
*/

 expr   :=    <space>expr         // expr is a logical combination of cmds
              expr<space>
              expr #comment
              "expr"
              (expr)
              expr ; expr
              expr && expr
              expr || expr
              cmd &               // cmd is the unit of suspension: ^Z will be interpreted as EXIT_FAILURE

 cmd    :=    cmd | cmd           // cmd is the unit of truth value evaluation
              cmd >> path         // note: pipe redirection get priority over explicit redirection
              cmd 2> path
              cmd > path
              cmd < path
              comd

 comd   :=    comd option         // comd is the unit of built_in / fork execution
              alias               // note priority: alias > built_in > executable TODO alias cd "echo hi i am the cd alias" --> also allowed in zsh
              built_in
              executable_path     // relative (using the PATH env var) or absolute

 alias  :=    (expr)              // alias is a symbolic link to an expr
```

Currenly supported built_ins:
* `cd`
* `color`
* `debug`
* `exit`
* `history`
* `shcat`
* `alias`       // syntax: alias key "value with spaces"
* `unalias`

To compile `jsh`, clone this respository, `cd` into it and execute `make`. See [the wiki page](https://github.com/jovanbulck/jo-shell/wiki/Compiling-and-running) for more info.
