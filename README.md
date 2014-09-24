jo-shell
========

A proof-of-concept shell implementation.

`jsh` is free software and makes use of the `GNU Readline` library for input line editing and history.

The following grammar is currently supported. Note that aliases are currenly only a hack...

```
 input  :=    expr

 expr   :=    <space>expr         // expr is a logical combination
              expr<space>
              #expr
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

 comd   :=    alias               // alias is a symbolic link to a comd; no expr or cmd...
              comd option         // comd is a single executable; the unit of built_in / fork
              built_in            // note: built_ins get priority over cmds
              executable path
```
To compile `jsh`, clone this respository, `cd` into it and execute `make`.
