# 🐚 C Shell

A simple shell program written in C. The shell provides a command-line interface for executing various commands and interacting with the operating system. Written for Operating systems classes at the University of Adam Mickiewicz in Poznań.

## Commands

### `cd`

Change the current working directory.

Syntax: `cd [directory]`

- If no `directory` is specified, the home directory is set as the destination.
- Use `~` as a shortcut for the home directory.
- Use `-` as a shortcut to switch to the previous directory.

### `clear`

Clear the terminal screen.

### `exit`

Terminate the shell.

### `help`

Display implemented commands and project specifications.

### `history`

Display previously entered commands.

### `ls`

List directory contents.

Syntax: `ls [options]`

Available options:

- `-a`: Do not ignore entries starting with `.`.
- `-g`: Like `-l`, but do not list owner.
- `-G`: In a long listing, don't print group names.
- `-l`: Use a long listing format.
- `-p`: Append `/` indicator to directories.
- `-Q`: Enclose entry names in double quotes.

### `mv`

Move (rename) files or directories.

Syntax: `mv [source] [destination]`

- Rename `source` to `destination`, or move `source` to `directory`.
- Directories are moved (renamed) recursively.

## Bonus Features

- Extensive control of key input.
- History accessible through up/down arrow keys.
- Shortened home directory path.
- Color-coded text.
- Custom startup animation.
- Sound alert for errors.
- User login and host name.
