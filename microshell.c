#include <stdio.h>              
#include <stdlib.h>             
#include <string.h>             
#include <unistd.h>             // getcwd, chdir
#include <dirent.h>             // void ls() - struct dirent, readdir, closedir
#include <sys/types.h>
#include <sys/wait.h>           // not used
#include <sys/stat.h>           // stat() - checking if entry is directory or file
#include <errno.h>
#define RED "\033[91m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define BLUE "\033[94m"
#define PINK "\033[95m"
#define CYAN "\033[96m"
#define RESET "\033[0m"
#define COMMAND_COLOR PINK
#define NO_FILE 0
#define DIRECTORY 1
#define REGULAR_FILE 2
#define OTHER_FILE 3

// used for testing
#define PRINT_SOURCE printf("mv() | source: %s\n\n", source);
#define PRINT_DESTINATION printf("mv() | destination: %s\n\n", destination);

char previous_directory[PATH_MAX];


void help()
{
	printf("\n"
            "Autor: Wojciech Kubicki\n"
            "Nr Indeksu: 483780\n"
            "----------------------\n"
            "Obslugiwane polecenia:\n"
            COMMAND_COLOR"help"RESET" - see list of commands and project specification\n"
            COMMAND_COLOR"mv"RESET" - move (rename) files\n"
            COMMAND_COLOR"tree"RESET" - list contents of directories in a tree-like format.\n"
            COMMAND_COLOR"clear"RESET" - clear the terminal screen\n"
            COMMAND_COLOR"exit"RESET" - causes normal process termination\n"
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
    char* absolute_path = malloc(PATH_MAX); // allocate memory for the absolute path
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


void move_directory(char *destination)
{
    DIR *directory = opendir(destination);
    if (directory == NULL) {
        perror("move_directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        // code here: get rwx permissions of file

        if (entry->d_type == DT_DIR) {
            // todo -
            // create new directory in destination
            // run function recursively with new directory path as destination
            // todo - remove directory
        }
        else if (entry->d_type == DT_REG) {
            // todo - create new file in destination
            // todo - remove file
        }
    }
    closedir(directory);
    return;
}


 // todo - if (strstr(source, "~") == 0) strcpy(source, getenv("HOME"));
void mv(char *source_no_malloc, char *destination_no_malloc)  
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
    // checks if path is valid for paths requiring relative navigating using '..'
    if (strstr(source, "..")) strcpy(source, relative_to_absolute(cwd, source));
    if (strstr(destination, "..")) strcpy(destination, relative_to_absolute(cwd, destination));

    int source_type = check_file_type(source), destination_type = check_file_type(destination);

    if ((source_type == REGULAR_FILE) && (destination_type == REGULAR_FILE)) {
        // checking if inodes of source file and destination file are the same
        struct stat file1, file2;

        if (stat(source, &file1) == -1) {
            perror(source);
            return;
        }

        stat(destination, &file2);

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
        char filename[256];  
        if (strchr(source, '/') != NULL) {  // if true then source path conatains directory/-ies
            int i = 0, last;
            for (i; i < strlen(source); i++) {  // gets index of last '/' character, all characters after '/' are the filename
                if (source[i] == '/') last = i+1;
            }
            strncpy(filename, source + last, strlen(source) - last);
        }
        else strcpy(filename, source);

        // needs to insert '/' as last character of destination
        // the variable desitnation used in move_file() is of the format {char destination + "/" + char filename}
        if (destination[strlen(destination) - 1] != '/') {
            strcat(destination, "/");
        }

        strcat(destination, filename);

        move_file(source, destination);
    }
    else if ((source_type == DIRECTORY) && (destination_type == DIRECTORY))  {
        printf("directory -> directory\n");

        // make directory in destination
        move_directory(source);
    }
    else {
        printf("mv: source and destination types are incompatible\n");
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


void test()
{
    // code here
    
    return;
}


int main()
{
    chdir(getenv("HOME"));

    // ASCII text art
    printf(GREEN"           _                    _          _ _ \n"
            " _ __ ___ (_) ___ _ __ ___  ___| |__   ___| | |\n"
            "| '_ ` _ \\| |/ __| '__/ _ \\/ __| '_ \\ / _ \\ | |\n"
            "| | | | | | | (__| | | (_) \\__ \\ | | |  __/ | |\n"
            "|_| |_| |_|_|\\___|_|  \\___/|___/_| |_|\\___|_|_|\n\n"RESET
            "code by: Wojciech Kubicki\n"
            "Use the "COMMAND_COLOR"help"RESET", command to view more information about the project.\n\n");

    char cwd[PATH_MAX];
	char command[PATH_MAX];
    char *paramater[PATH_MAX];
    int paramater_count;

    char *login = getenv("USER");
    char *host = getenv("NAME");  // Ubuntu doesn't have HOST variable in env, uses 'NAME' instead. Grep returns 'NAME={machine_name}' when searching for machine name.

    getcwd(previous_directory, sizeof(previous_directory));

	while(1)
	{
        // Replacing HOME path in cwd variable with '~' symbol.
        if (strstr(getcwd(cwd, sizeof(cwd)), getenv("HOME")) != NULL) {
            // write here explenation for below code {https://www.youtube.com/watch?v=0qSU0nxIZiE}
            // First step: get pointer to begining of searched substring in string
            // Second step: move characters from string to 'left' or 'right' depending on if inserted substring is smaller or larger than original substring
            // Third step: 
            memmove(cwd + strlen("~"), cwd + strlen(getenv("HOME")), strlen(cwd) - strlen(getenv("HOME")) + 1);
            memcpy(cwd, "~", strlen("~"));  // Using strcpy() inserts '/0' after inserted substring.
        }

        // Printing user, host name and current working directory.
        // [user@host:cwd]$ 
        printf("["GREEN"%s@%s"RESET":"BLUE"%s"RESET"]$ ", login, host, cwd);
        
        // Gets user input.
		fgets(command, sizeof(command), stdin);
        if (strcmp(command, "\n") != 0) {  // Skip empty input.
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
            mv(paramater[1], paramater[2]);
        }
        else if (strcmp(paramater[0], "tree") == 0) {
            tree(".", 0);
        }
        else if (strcmp(paramater[0], "clear") == 0) {
            clear();
        }
        else if (strcmp(paramater[0], "test") == 0) {
            test();
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
