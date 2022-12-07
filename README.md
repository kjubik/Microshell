# Description
Project resembling Linux terminal. Created for Operating Systems class during 1st semester of Computer Science undergraduate programme.

# Project requirements
Program can only be written in C without using any system calls.

- display a prompt of the form [{path}] $, where {path} is the path to the current working directory
- handle 'cd' command, which works similarly to the 'cd' known from the bash shell
- handle 'exit' command, terminating the shell program
- handle 'help' command, displaying information on the screen about the author of the program and the functionality it offers
- handle two other shell commands of your choice (your own stand-alone, intermediate implementation of two commands)
- handle commands referring by name to programs located in directories described by the value of the PATH environment variable, and allow these scripts and programs to be called with arguments (using both: fork() + exec*())
- print an error message when command cannot be interpreted properly
- have so-called "bells and whistles", e.g. displaying the login of the currently logged-in user, color support, support for arguments in quotation marks, meaningful signal support (e.g. Ctrl+Z), command history support via arrows, syntax completion, etc.; points are awarded according to the complexity of the problem
