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
#include <math.h>
#include <pwd.h>                // getpwuid()

char current_working_directory[PATH_MAX];
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
#define HISTORY_LIMIT 99
char command_history[HISTORY_LIMIT][256];
int history_index = 0;
int total_commands = 0;
int index_shift = 1;


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
	printf("\nAuthor:\n"
            "Wojciech Kubicki\n\n"
            "Implemented commands:\n"
            COMMAND_COLOR"cd"RESET" - change current working directory\n"
            COMMAND_COLOR"clear"RESET" - clear the terminal screen\n"
            COMMAND_COLOR"exit"RESET" - terminate program\n"
            COMMAND_COLOR"help"RESET" - display commands and project specification\n"
            COMMAND_COLOR"ls"RESET" - list directory contents\n"
            "| -a | -g | -G | -i | -l | -p | -Q | -1 |\n"
            COMMAND_COLOR"mv"RESET" - move (rename) files\n"
            "file -> file | file -> directory | directory -> directory (recursively)\n\n"
            "Bonus features:\n"
            "- custom startup animation [COMING SOON]\n"
            "- user login and host name\n"
            "- shortened home directory path\n"
            "- color coded text\n"
            "- sound alert for errors [COMING SOON]\n\n");

    return;
}


void change_directory(char *destination)
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

char *replace_symbol(char *path, char *target, char *replacement)
{
    // write here explenation for below code {https://www.youtube.com/watch?v=0qSU0nxIZiE}
    // First step: get pointer to begining of searched substring in string
    // Second step: move characters from string to 'left' or 'right' depending on if inserted substring is smaller or larger than original substring
    // Third step: 
    while (strstr(path, target) != NULL) {
        memmove(path + strlen(replacement), path + strlen(target), strlen(path) - strlen(target) + 1);
        memcpy(path, replacement, strlen(replacement));  // Using strcpy() inserts '/0' after inserted substring.
    }

    return path;
}


char* replace_symbols(char *path)
{
    if (strcmp(path, ".") == 0) {
        getcwd(current_working_directory, sizeof(current_working_directory));
        return current_working_directory;
    }
    else if (strcmp(path, "~")== 0) {
        return getenv("HOME");
    }
    else if (strcmp(path, "..") == 0) {
        // code here
    }
    else {
        getcwd(current_working_directory, sizeof(current_working_directory));
        strcpy(path, replace_symbol(path, "./", strcat(current_working_directory, "/")));
        char home[256];  // get stack smashing when sizeof(getenv("HOME")) <- why? works in list_files - probably because list_files calls this?
        strcpy(path, replace_symbol(path, "~/", home));
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
    // copies paramaters passed into function into strings with larger memory allocation  - fixed truncation when replacing '~' with full path in replace_symbols
    char *source = malloc(PATH_MAX);
    strcpy(source, source_no_malloc);

    char *destination = malloc(PATH_MAX);
    strcpy(destination, destination_no_malloc);

    // modifies paths containing "~" and/or "." with full path names <- without this opening files wasn't handled properly
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));

    strcpy(source, replace_symbols(source));  // This is retarted. Files with extenstions ({filename}.{ext}) are unusable, because "." gets replaced with CWD
    strcpy(destination, replace_symbols(destination));  // Same as above. This is retarted.

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

        char source_filename[PATH_MAX];
        extract_filename(source, source_filename);

        struct stat phantom_directory;

        if (stat(destination, &phantom_directory) == -1) {
            char penultimate_path[PATH_MAX];
            strcpy(penultimate_path, destination);

            if (strrchr(penultimate_path, '/') != NULL) {
                char *last_slash = strrchr(penultimate_path, '/');
                if (last_slash != NULL) {
                    *last_slash = '\0';
                    
                    if (stat(penultimate_path, &phantom_directory) == -1) {
                        perror("no directory like this boy");
                        return;
                    }
                    else {
                        // move the source directory to the penultimate_path under a new name 
                        // as declared by the last directory in the destination path
                        char destination_filename[PATH_MAX];
                        extract_filename(destination, destination_filename);

                        move_directory(source, destination_filename, penultimate_path);
                    }
                }
            }
            else {
                // if the destination doesn't contain a '/', 
                // then the source is moved to the current directory under a new name
                move_directory(source, destination, cwd);
            }
        }

        // make directory in destination
        move_directory(source, source_filename, destination);
        return;
    }
    else {
        printf("mv: source and destination types are incompatible\n");
        return;
    }

    return;
}


void list_files(char *arguments, char *location)
{
    // Allocates memory for path
    char *path = malloc(PATH_MAX);
    strcpy(path, location);
    strcat(path, "/");
    char home[sizeof(getenv("HOME"))];
    strcpy(home, getenv("HOME"));
    strcat(home, "/");
    strcpy(path, replace_symbol(path, "~/", home));
    
    DIR* directory = opendir(path);
    if (directory == NULL) {
        printf("%s - ", path);
        perror("ls");
        return;
    }

    long int total_size = 0;

    // -R  list subdirectories recursively
    if (strchr(arguments, 'R')) {
        printf("%s:\n", path);
    }

    // reading every file in directory
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        // arguments applied before display
        char name[PATH_MAX];
        strcpy(name, entry->d_name);

        struct stat file_st;
        stat(name, &file_st);

        // -a  do not ignore entries starting with .
        if (!strchr(arguments, 'a')) {
            if (name[0] == '.')
                continue;
        }
        

        // -i  print the index number of each file
        if (strchr(arguments, 'i')) {
            printf("%ld ", entry->d_ino);
        }

        // -l  use a long listing format
        // -g  like -l, but do not list owner
        if (strchr(arguments, 'l') || strchr(arguments, 'g')) {

            if (S_ISDIR(file_st.st_mode)) printf("d");
            else printf("-");
            if (file_st.st_mode & S_IRUSR) printf("r");
            else printf("-");
            if (file_st.st_mode & S_IWUSR) printf("w");
            else printf("-");
            if (file_st.st_mode & S_IXUSR) printf("x");
            else printf("-");
            if (file_st.st_mode & S_IRGRP) printf("r");
            else printf("-");
            if (file_st.st_mode & S_IWGRP) printf("w");
            else printf("-");
            if (file_st.st_mode & S_IXGRP) printf("x");
            else printf("-");
            if (file_st.st_mode & S_IROTH) printf("r");
            else printf("-");
            if (file_st.st_mode & S_IWOTH) printf("w");
            else printf("-");
            if (file_st.st_mode & S_IXOTH) printf("x");
            else printf("-");
            printf(" ");

            printf("%ld ", file_st.st_nlink);

            if (!strchr(arguments, 'g')) {
                struct passwd *userid = getpwuid(file_st.st_uid);
                printf("%s ", userid->pw_name);
            }

            if (!strchr(arguments, 'G')) {
                struct passwd *groupid = getpwuid(file_st.st_gid);
                printf("%s ", groupid->pw_name);
            }

            printf("%ld ", file_st.st_size);
            total_size += file_st.st_size;

            // https://man7.org/linux/man-pages/man3/ctime.3.html
            // https://www.ibm.com/docs/en/i/7.3?topic=functions-ctime-convert-time-character-string
            // st_atime (access time) = when was it opened last as "r"
            // st_mtime (modification time) = when was it opened last as "a" or "w"
            // st_ctime (change mode time) = when was chmod() called on it last <- closest to time of creation
            struct tm *time_st = localtime(&file_st.st_ctime);  // no st_birthtime member, had to use st_ctime - Linux doesn't support it? Seems that only MacOS does :(
            char month[4];
            strftime(month, sizeof(month), "%m", time_st);
            if (strcmp(month, "01") == 0) {
                strcpy(month, "Jan");
            }
            else if (strcmp(month, "02") == 0) {
                strcpy(month, "Feb");
            }
            else if (strcmp(month, "03") == 0) {
                strcpy(month, "Mar");
            }
            else if (strcmp(month, "04") == 0) {
                strcpy(month, "Apr");
            }
            else if (strcmp(month, "05") == 0) {
                strcpy(month, "May");
            }
            else if (strcmp(month, "06") == 0) {
                strcpy(month, "Jun");
            }
            else if (strcmp(month, "07") == 0) {
                strcpy(month, "Jul");
            }
            else if (strcmp(month, "08") == 0) {
                strcpy(month, "Aug");
            }
            else if (strcmp(month, "09") == 0) {
                strcpy(month, "Sep");
            }
            else if (strcmp(month, "10") == 0) {
                strcpy(month, "Oct");
            }
            else if (strcmp(month, "11") == 0) {
                strcpy(month, "Nov");
            }
            else if (strcmp(month, "12") == 0) {
                strcpy(month, "Dec");
            }
            char time_tail[256];
            strftime(time_tail, sizeof(time_tail), " %d %H:%M", time_st);
            strcat(month, time_tail);
            printf("%s ", month);
        }

        // -Q  enclose entry names in double quotes
        if (strchr(arguments, 'Q')) {
            printf("\"");
        }

        // prints entry in directory
        if (entry->d_type == DT_DIR) {  // file is a directory
            printf(BLUE"%s"RESET, entry->d_name);
        }
        else if (entry->d_type == DT_REG) {  // file is a regular file
            // regfile variable created so that path to file can be passed in stat()
            char regfile[PATH_MAX];
            strcpy(regfile, path);
            strcat(regfile, "/");
            strcat(regfile, entry->d_name);

            struct stat filestat;

            if (stat(regfile, &filestat) == 0 && filestat.st_mode & S_IXUSR) {  // regular file is an executable
                printf(GREEN"%s"RESET, entry->d_name);
            }
            else {  // regular file without execute permission
                printf("%s", entry->d_name);
            }
        }

        // arguments applied after file has been display

        // -Q  enclose entry names in double quotes
        if (strchr(arguments, 'Q')) {
            printf("\"");
        }

        // -p  append / indicator to directories
        if (strchr(arguments, 'p') && entry->d_type == DT_DIR) {
            printf("/");
        }

        printf("  ");

        // -1  list one file per line.
        if (strchr(arguments, '1') || strchr(arguments, 'l') || strchr(arguments, 'g')) {
            printf("\n");
        }

    }

    if (!(strchr(arguments, '1') || strchr(arguments, 'l') || strchr(arguments, 'g'))) 
        printf("\n");

    closedir(directory);

    if (strchr(arguments, 'l')) {
        printf("total %ld\n", total_size);
    }

    if (strchr(arguments, 'R')) {
        DIR* directory = opendir(path);
        if (directory == NULL) {
            perror("ls");
            return;
        }

        while ((entry = readdir(directory)) != NULL) {
            if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {  // directories with "." would casue problem on Ubuntu VM on home workstation
                char child_path[PATH_MAX];
                strcpy(child_path, path);
                strcat(child_path, entry->d_name);
                printf("\n");
                list_files(arguments, child_path);
            }
        }
        closedir(directory);
    }

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
    total_commands++;
    if (history_index == HISTORY_LIMIT) {
        // removes the first command saved and moves each next command to the previous index
        for (int i = 0; i < HISTORY_LIMIT - 1; i++) {
            strcpy(command_history[i], command_history[i + 1]);
        }
        history_index--;
        index_shift++;
    }
    strcpy(command_history[history_index], command);
    history_index++;

    return;
}


void list_history() 
{
    for (int i = 1; i < history_index+1; i++) {
        int digits = 0;
        int temp = i;
        while (temp > 0) {
            temp /= 10;
            digits++;
        }
        printf(" %*d  %s", digits, i + index_shift, command_history[i]);
    }

    return;
}


void fix_home_env();


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
        getcwd(current_working_directory, sizeof(current_working_directory));
        strcpy(current_working_directory, replace_symbol(current_working_directory, getenv("HOME"), "~"));  // after using move() function cwd gets printed as envvar HOME. why does this happen?

        // Printing user, host name and current working directory.
        // [user@host:cwd]$ 
        printf("["GREEN"%s@%s"RESET":"BLUE"%s"RESET"]$ ", login, host, current_working_directory);
        
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
            exit(0);
        }
        else if (strcmp(paramater[0],"cd") == 0) {
            if (paramater_count == 1) {
                paramater[1] = "~";
            }
            change_directory(paramater[1]);
        }
        else if (strcmp(paramater[0], "mv") == 0) {  // after running mv function once (eg. mv alpha omega) printing cwd with ~ doesn't work. why?
            if (paramater_count != 3) {
                printf("mv: invalid number of arguments passed\n");
            }
            else move(paramater[1], paramater[2]);
        }
        else if (strcmp(paramater[0], "test") == 0) {
            
        }
        else if (strcmp(paramater[0], "clear") == 0) {
            clear();
        }
        else if (strcmp(paramater[0], "ls") == 0) {
            char location[PATH_MAX] = ".";
            char arguments[20] = "";

            for (int i = 1; i < paramater_count; i++) {
                if (paramater[i][0] == '-') {
                    strcat(arguments, paramater[i]);
                }
                else {
                    strcpy(location, paramater[i]);
                }
            }

            list_files(arguments, location);
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

// somewhere in my program, the "HOME" env var is being modified by concatinating '/'
// this isn't an elegant solution, but gets the job done
void fix_home_env()
{
    if(getenv("HOME")[strlen(getenv("HOME"))-1] == '/')
        getenv("HOME")[strlen(getenv("HOME"))-1] = '\0';
}
