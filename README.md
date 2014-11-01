```
                       _                      __           __ __
                      (_)____          _____ / /_   ___   / // /
                     / // __ \ ______ / ___// __ \ / _ \ / // / 
                    / // /_/ //_____/(__  )/ / / //  __// // /  
                 __/ / \____/       /____//_/ /_/ \___//_//_/   
                /___/
                              a basic UNIX shell implementation in C
```
## Introducing `jsh`
`jsh` (jo-shell): A basic UNIX shell implementation in C

From the `man`page:
> `jsh` is a UNIX command interpreter (shell) that executes commands read from the standard input or from a file. `jsh` implements a subset of the `sh` language grammar and is intended to be POSIX-conformant.

> `jsh` is written 'just for fun' and is not intented to be a full competitor to advanced UNIX shells such as `bash` and `zsh`. `jsh` is free software and you are welcome to collaborate on the github page or to redistribute jsh under the conditions of the GNU General Public License.

## Find out more
| About | Installation guide | Configuration | Documentation | Contributing |
|-------|-------------------|---------------|--------------|--------------|
| ![about-icon](https://cloud.githubusercontent.com/assets/2464627/4871965/28777edc-61d1-11e4-876c-4f874b75e9ae.png) | ![installation-icon](https://cloud.githubusercontent.com/assets/2464627/4871947/8c2f52fc-61d0-11e4-9f31-5f44aaecfcce.png) | ![config-icon](https://cloud.githubusercontent.com/assets/2464627/4872003/81e09858-61d3-11e4-94e4-920777e271cf.png) | ![doc-icon](https://cloud.githubusercontent.com/assets/2464627/4871956/bb9b4e74-61d0-11e4-978b-9af28da67ad8.png) | ![community-icon](https://cloud.githubusercontent.com/assets/2464627/4871945/53747d02-61d0-11e4-8ae8-10e1bdf3b70b.png) |
| [About `jsh`](About) | [Compiling and running `jsh`](Compiling-and-running) | [Customizing `jsh`](Sample-configuration-files) | [`jsh` man page](Manual) | [Coding guidelines](Coding-guidelines)|
| Introducing the jo-shell | Step-by-step guide to compile `jsh`for your own system | Configuring the shell for your own use | Online text version of the latest `man jsh` | Info for developers |

## Get it!
[This page](https://github.com/jovanbulck/jo-shell/releases/latest) provides pre-built binaries for all official `jsh` releases. You can also build `jsh` for your own system yourself using `make`. See [the wiki page](https://github.com/jovanbulck/jo-shell/wiki/Compiling-and-running) for more info.

## Supported shell grammar

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
