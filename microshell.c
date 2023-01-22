#include <stdio.h>              
#include <stdlib.h>             
#include <string.h>             
#include <unistd.h>             // getcwd, chdir
#include <dirent.h>             // void ls() - struct dirent, readdir, closedir
#include <sys/types.h>
#include <sys/wait.h>           // not used
#include <sys/stat.h>           // stat() - checking if entry is directory or file
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>          // terminal window size

char previous_directory[PATH_MAX];

// colors
#define RED "\033[91m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define BLUE "\033[94m"
#define PINK "\033[95m"
#define CYAN "\033[96m"
#define RESET "\033[0m"
#define COMMAND_COLOR PINK

// file type codes
#define NO_FILE 0
#define DIRECTORY 1
#define REGULAR_FILE 2
#define OTHER_FILE 3

// animation duration
#define ANIMATION 100000

// history function variables
#define HISTORY_LIMIT 1000
char command_history[HISTORY_LIMIT][1024];
int history_index = 0;


void ring() {
    putchar('\a'); 
    fflush(stdout);
    return;
}

void startup()
{
    chdir(getenv("HOME"));

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width = w.ws_col;

    printf("Loading |");
    for (int i = 0; i < (width-16); i++) {
        printf("█");
        fflush(stdout);
        usleep(10000);
    }
    printf("| Done!\n");
    printf("Use the "COMMAND_COLOR"help"RESET", command to view more information about the project.\n\n");
    sleep(2);

    return;
}


void help()
{
	printf("\n"
            "----------------------\n"
            "Author: Wojciech Kubicki\n"
            "----------------------\n"
            "Implemented commands:\n"
            COMMAND_COLOR"cd"RESET" - change current working directory\n"
            COMMAND_COLOR"clear"RESET" - clear the terminal screen\n"
            COMMAND_COLOR"exit"RESET" - terminate program\n"
            COMMAND_COLOR"help"RESET" - display commands and project specification\n"
            COMMAND_COLOR"history"RESET" - list previously entered commands\n"
            COMMAND_COLOR"mv"RESET" - move (rename) files\n"
            COMMAND_COLOR"tree"RESET" - list contents of directories in a tree-like format.\n"
            "----------------------\n"
            "Other features:\n"
            "- custom startup animation\n"
            "- display user login and host name\n"
            "- color coded text\n"
            "- sound alert for errors\n"
            "----------------------\n"
            "\n");

    return;
}


void cd(char *destination)
{
    // Saves current working directory for updating previous_directory later.
    char temp_directory[PATH_MAX];
    getcwd(temp_directory, sizeof(temp_directory));

    // replace below if statement with code that inserts getenv("HOME") in place of '~' character
    if (strcmp(destination, "~") == 0) {
        chdir(getenv("HOME"));
    }
    else if (strcmp(destination, "-") == 0){
        printf("%s\n", previous_directory);
        chdir(previous_directory);  // previous_directory is initialy set to the HOME enviroment variable, updated after first cd function call
    }
    else {
        if(chdir(destination) != 0){
            printf("-bash: cd: %s: No such file or directory\n", destination);
        }
    }

    // Updating previous_directory location.
    strcpy(previous_directory, temp_directory);

    return;
}


// below functions are used for mv()

char* replace_symbol(char *path, char *symbol, char *replacement)
{
    while (strstr(path, symbol) != NULL) {
        memmove(path + strlen(replacement), path + strlen(symbol), strlen(path) - strlen(symbol) + 1);
        memcpy(path, replacement, strlen(replacement));  // Using strcpy() inserts '/0' after inserted substring.
    }

    return path;
}


char *relative_to_absolute(char *cwd, char *relative_path)
{
    char *absolute_path = malloc(PATH_MAX); // allocate memory for the absolute path
    strcpy(absolute_path, cwd); // copy current working directory to the absolute path

    char* token = strtok(relative_path, "/");
    while (token != NULL) {
        if (strcmp(token, "..") == 0) {
            
        }
        else {

        }
    }

    return absolute_path;
}



int check_file_type(char *path)
{
    DIR *directory = opendir(path);
    if (directory != NULL) {
        closedir(directory);
        return DIRECTORY;
    }
    
    FILE *file = fopen(path, "r");
    if (file != NULL) {
        fclose(file);
        return REGULAR_FILE;
    }
    
    if (path[strlen(path)-1] == '/') {
        return NO_FILE;
    }
    else return REGULAR_FILE; /* this could also be OTHER_FILE, but should I try and implement other files as well? 
    I tried using 'struct stat entry;', but that was a mess. 
    This if-else-tree for directories and regular files is a headache free solution. :) */
}


void extract_filename(char *path, char *filename)
{ 
        if (strchr(path, '/') != NULL) {  // if true then source path conatains directory/-ies
            int i = 0, last;
            for (i; i < strlen(path); i++) {  // gets index of last '/' character, all characters after '/' are the filename
                if (path[i] == '/') last = i+1;
            }
            strncpy(filename, path + last, strlen(path) - last);
        }
        else strcpy(filename, path);

    return;
}


void move_file(char *source, char *destination)
{
    char byte;

    FILE *source_file;
    source_file = fopen(source, "r");
    if (source_file == NULL) {
        perror(source);
        return;
    }

    FILE *destination_file;
    destination_file = fopen(destination, "w");
    if (destination_file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    while ((byte = fgetc(source_file)) != EOF) {
        fputc(byte, destination_file);
    }

    fclose(source_file);
    fclose(destination_file);
    remove(source);

    return;
}


void move_directory(char *source, char *filename, char *destination)
{
    DIR *directory = opendir(source);
    if (directory == NULL) {
        perror("move_directory");
        return;
    }

    // creates new directory in destination
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    printf("changing directory to: %s\n\n", destination);
    chdir(destination);
    printf("making the new directory: %s\n\n", filename);
    mkdir(filename, 0777);
    printf("changing directory back to: %s\n\n", cwd);
    chdir(cwd);

    // moves all children regular files and directories
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        // code here: get rwx permissions of file

        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // runs function recursively for child directory found in source directory
            char *new_source = malloc(PATH_MAX);
            strcpy(new_source, source);
            strcat(new_source, "/");
            strcat(new_source, entry->d_name);

            char *new_filename = malloc(PATH_MAX);
            strcpy(new_filename, entry->d_name);
            
            char *new_destination = malloc(PATH_MAX);
            strcpy(new_destination, destination);
            strcat(new_destination, "/");
            strcat(new_destination, filename);

            printf("new_source: %s\nnew_filename: %s\nnew_destination: %s\n", new_source, new_filename, new_destination);

            move_directory(new_source, new_filename, new_destination);
            
        }
        else if (entry->d_type == DT_REG) {
            // moves file to directory
            char *new_source = malloc(PATH_MAX);
            strcpy(new_source, source);
            strcat(new_source, "/");
            strcat(new_source, entry->d_name);

            char *new_destination = malloc(PATH_MAX);
            strcpy(new_destination, destination);
            strcat(new_destination, "/");
            strcat(new_destination, filename);
            strcat(new_destination, "/");
            strcat(new_destination, entry->d_name);

            move_file(new_source, new_destination);

        }
    }
    closedir(directory);

    // removes the directory after copying
    rmdir(source);
    return;
}


void move(char *source_no_malloc, char *destination_no_malloc)  
{
    // copies paramaters passed into function into strings with larger memory allocation  - fixed truncation when replacing '~' with full path in replace_symbol
    char *source = malloc(PATH_MAX);
    strcpy(source, source_no_malloc);

    char *destination = malloc(PATH_MAX);
    strcpy(destination, destination_no_malloc);

    // modifies paths containing "~" and/or "." with full path names <- without this opening files wasn't handled properly
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    strcpy(source, replace_symbol(source, "~", getenv("HOME")));
    strcpy(source, replace_symbol(source, ".", cwd));
    strcpy(destination, replace_symbol(destination, "~", getenv("HOME")));
    strcpy(destination, replace_symbol(destination, ".", cwd));
    // THIS PART DOESN'T WORK RIGHT NOW. And I'll probably leave it as is. Too much work rewriting previous code
    // checks if path is valid for paths requiring relative navigating using '..'
    //if (strstr(source, "..")) strcpy(source, relative_to_absolute(cwd, source));
    //if (strstr(destination, "..")) strcpy(destination, relative_to_absolute(cwd, destination));

    int source_type = check_file_type(source), destination_type = check_file_type(destination);

    if ((source_type == REGULAR_FILE) && (destination_type == REGULAR_FILE)) {
        // checking if inodes of source file and destination file are the same
        struct stat file1, file2;

        if (stat(source, &file1) == -1) {
            perror(source);
            return;
        }

        stat(destination, &file2);  // omited checking for error, because file might not exist

        if (file1.st_ino != file2.st_ino) {
            move_file(source, destination);
        }
        else {
            printf("mv:"RED" '%s' and '%s' are the same file\n"RESET, source, destination);
            return;
        }
    }
    else if ((source_type == REGULAR_FILE) && (destination_type == DIRECTORY)) {
        // appends source filename to destination
        char filename[PATH_MAX];
        extract_filename(source, filename);

        // needs to insert '/' as last character of destination
        // the variable desitnation used in move_file() is of the format {char destination + "/" + char filename}
        if (destination[strlen(destination) - 1] != '/') {
            strcat(destination, "/");
        }
        strcat(destination, filename);

        move_file(source, destination);
        return;
    }
    else if (source_type == DIRECTORY /*&& destination_type == DIRECTORY*/)  {
        printf("directory -> directory\n");

        char filename[PATH_MAX];
        extract_filename(source, filename);

        // checks if 
        /*struct stat directory;
        if (stat(destination, &directory) == -1) {
            if (chdir() == NULL) {
                perror("chdir failed");
                return;
            }
            mkdir();
            chdir();
        }*/

        // make directory in destination
        move_directory(source, filename, destination);
        return;
    }
    else {
        printf("mv: source and destination types are incompatible\n");
        return;
    }

    return;
}


void ls()
{
    /* code here */
    printf("ls\n");

    return;
}


void tree(char *path, int indentlevel)
{
    /* code here */
    printf("tree\n");

    return;
}


void clear()
{
    printf("\033c");
    return;
}


/* history_index = number of last entered command
command_history = list containing entered commands
command_count = number of lines to print out */
void save_history(char *command) 
{
    if (history_index == HISTORY_LIMIT) {
        // Overwrite the first command with the last one
        for (int i = 0; i < HISTORY_LIMIT - 1; i++) {
            strcpy(command_history[i], command_history[i + 1]);
        }
        history_index--;
    }
    strcpy(command_history[history_index], command);
    history_index++;

    return;
}


void list_history() 
{
    int command_count;
    if (history_index > HISTORY_LIMIT) command_count = HISTORY_LIMIT;
    else command_count = history_index;

    for (int i = 0; i < command_count; i++) {
        int command_num = i + 1;
        int digits = 0;
        int temp = command_num;
        while (temp > 0) {
            temp /= 10;
            digits++;
        }
        printf(" %*d  %s", 3 - digits, history_index + command_num, command_history[i]);
    }

    return;
}


void test()
{
    // code here
    
    return;
}


int main()
{
    //startup();
    chdir(getenv("HOME"));

    // ASCII text art
    /*printf(GREEN"           _                    _          _ _ \n"
            " _ __ ___ (_) ___ _ __ ___  ___| |__   ___| | |\n"
            "| '_ ` _ \\| |/ __| '__/ _ \\/ __| '_ \\ / _ \\ | |\n"
            "| | | | | | | (__| | | (_) \\__ \\ | | |  __/ | |\n"
            "|_| |_| |_|_|\\___|_|  \\___/|___/_| |_|\\___|_|_|\n\n"RESET
            "code by: Wojciech Kubicki\n"
            "Use the "COMMAND_COLOR"help"RESET", command to view more information about the project.\n\n");*/

    char cwd[PATH_MAX];
	char command[PATH_MAX];
    char *paramater[PATH_MAX];
    int paramater_count;

    char *login = getenv("USER");
    char *host = getenv("NAME");  // Ubuntu doesn't have HOST variable in env, uses 'NAME' instead. Grep returns 'NAME={machine_name}' when searching for machine name.
    char *home = getenv("HOME");

    getcwd(previous_directory, sizeof(previous_directory));

	while(1)
	{
        // Replacing HOME path in cwd variable with '~' symbol.
        if (strstr(getcwd(cwd, sizeof(cwd)), home) != NULL) {
            // write here explenation for below code {https://www.youtube.com/watch?v=0qSU0nxIZiE}
            // First step: get pointer to begining of searched substring in string
            // Second step: move characters from string to 'left' or 'right' depending on if inserted substring is smaller or larger than original substring
            // Third step: 
            memmove(cwd + strlen("~"), cwd + strlen(home), strlen(cwd) - strlen(home) + 1);
            memcpy(cwd, "~", strlen("~"));  // Using strcpy() inserts '/0' after inserted substring.
        }

        // Printing user, host name and current working directory.
        // [user@host:cwd]$ 
        printf("["GREEN"%s@%s"RESET":"BLUE"%s"RESET"]$ ", login, host, cwd);
        
        // Gets user input.
		fgets(command, sizeof(command), stdin);
        if (strcmp(command, "\n") != 0) {  // Skip empty input.
            save_history(command);
            // Tokenizing string into paramaters.
            paramater_count = 0;
            char *token = strtok(command, " \n");  // TO ADD: Read all spaces in between quotes.
            while(token != NULL){
                paramater[paramater_count] = token;
                token = strtok(NULL, " \n");
                paramater_count++;
            }
            paramater[paramater_count] = NULL;  // This fixed segmentation fold.
        }
        else continue;
        
        // Checking for command in my microshell function implementations.
		if (strcmp(paramater[0],"help") == 0) {
            help();
		} 
        else if ((strcmp(paramater[0],"exit") == 0) || (strcmp(paramater[0],"q") == 0)) {
			printf("\nProgram zakonczony\n\n");
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(paramater[0],"cd") == 0) {
            if (paramater_count == 1) {
                paramater[1] = "~";
            }
            cd(paramater[1]);
        }
        else if (strcmp(paramater[0], "mv") == 0) {
            move(paramater[1], paramater[2]);
        }
        else if (strcmp(paramater[0], "tree!") == 0) {
            tree(".", 0);
        }
        else if (strcmp(paramater[0], "clear") == 0) {
            clear();
        }
        else if (strcmp(paramater[0], "history") == 0) {
            list_history();
        }
		else {  // If entered command isn't implemented in program, run the bash command.
            if (fork() == 0) {
				exit(execvp(paramater[0],paramater));
			}
			else {
				int status = 0;
				wait(&status);
                if(status == 65280) printf("%s: command not found\n", paramater[0]);
			}
        }
    }
}
