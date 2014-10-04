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
 input  :=    expr

 expr   :=    <space>expr         // expr is a logical combination of cmds
              expr<space>
              expr #comment
              "expr"
              (expr)
              expr ; expr
              expr && expr
              expr || expr
              cmd

 cmd    :=    cmd | cmd           // cmd is the unit of truth value evaluation
              cmd >> path         // note: pipe redirection get priority over explicit redirection
              cmd 2> path
              cmd > path
              cmd < path
              cmd &               *TODO not yet implemented
              comd

 comd   :=    comd option         // comd is the unit of fork / built_in
              alias               // note priority: alias > built_in > executable
              built_in
              executable_path     // relative (using the PATH env var) or absolute

 alias  :=    (expr)              // alias is a symbolic linkt to an expr
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
