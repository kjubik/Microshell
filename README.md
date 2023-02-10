## Description
Project aimed at ressembling functioning of shell on Linux. Created for Operating Systems classes during 1st semester of Computer Science undergraduate programme. Graded 30 points out of 30 possible.

## Project requirements

- display a prompt of the form [{path}] $, where {path} is the path to the current working directory
- handle 'cd' command, which works similarly to the 'cd' known from the bash shell
- handle 'exit' command, terminating the shell program
- handle 'help' command, displaying information on the screen about the author of the program and the functionality it offers
- handle commands referring by name to programs located in directories described by the value of the PATH environment variable, and allow these scripts and programs to be called with arguments (using both: fork() + exec*())
- print an error message when command cannot be interpreted properly
- (optional) handle two other shell commands of your choice (your own stand-alone, intermediate implementation of two commands)
- (optional) have bonus features, e.g. displaying the login of the currently logged-in user, color support, support for arguments in quotation marks, meaningful signal support (e.g. Ctrl+Z), command history support via arrows, syntax completion, etc.; points are awarded according to the complexity of the problem

## My features

### Commands:
- 'cd' - change directory
- 'clear' - clear terminal screen
- 'exit' - terminate program
- 'help' - display implemented commands and project specification
- 'history' - display previously entered commands
- 'ls' - list directory contents, usage: 'ls' [OPTIONS]
  - [-a] - do not ignore entries starting with .
  - [-g]  like -l, but do not list owner"
  - [-G]  in a long listing, don't print group names
  - [-l]  use a long listing format
  - [-p]  append / indicator to directories
  - [-Q]  enclose entry names in double quotes
  - [-R]  list subdirectories recursively
- 'mv' - move (rename) files
  - Usage: mv {source} {destination}
  - Rename {source} to {destination}, or move {source} to {directory}.
  - {directory} moved/renamed recursively.
  
### Additional features:
- extensive control of key input
- history accessible through up/down arrow keys
- shortened home directory path
- color coded text
- custom startup animation
- sound alert for errors
- user login and host name
