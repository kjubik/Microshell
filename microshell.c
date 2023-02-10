#include <dirent.h>
#include <libgen.h>     // extract filenames, paths, translate relative paths
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>    // control terminal settings
#include <time.h>
#include <unistd.h>

// sizes
#define MAX_PATH_LENGTH 4096
#define MAX_INPUT_LENGTH 256

// navigation variables
char cwd[MAX_PATH_LENGTH];
char last_directory[MAX_PATH_LENGTH];

// command arguments
char *arguments[MAX_INPUT_LENGTH/2];
int argument_count = 0;

// history function variables
#define MAX_HISTORY_SIZE 100    // saves last 99 inputs
char history_buffer[MAX_HISTORY_SIZE][MAX_INPUT_LENGTH];
int history_index = 0;
int retrieve_index = 0;

// terminal settings
struct termios old_termios, raw_termios;

// environment variables
char *login;
char *host;
char *home;

// file type codes
#define NO_FILE 0
#define DIRECTORY 1
#define REGULAR_FILE 2
#define OTHER_FILE 3

// colors
#define RED "\033[91m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define BLUE "\033[94m"
#define PINK "\033[95m"
#define CYAN "\033[96m"
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define COMMAND_COLOR
#define USER_COLOR GREEN
#define AT_COLOR GREEN
#define HOST_COLOR GREEN
#define PATH_COLOR BLUE


// MINOR FUNCTIONS ------------------------------


void alert() 
{
    putchar('\a'); 
    fflush(stdout);
    return;
}


int check_file_type(char *path)
{
    struct stat filestat;
    
    if (stat(path, &filestat) != 0) return REGULAR_FILE;
    else if (S_ISDIR(filestat.st_mode)) return DIRECTORY;
    else return REGULAR_FILE;
}


void clear_terminal()
{
    printf("\033c");
    return;
}


int get_terminal_width() 
{
    struct winsize window;
    ioctl(0, TIOCGWINSZ, &window);
    return window.ws_col;
}


char *replace_symbol(char *path, char *target, char *replacement)
{
    while (strstr(path, target) != NULL) {
        memmove(path + strlen(replacement), path + strlen(target), strlen(path) - strlen(target) + 1);
        memcpy(path, replacement, strlen(replacement));  // Using strcpy() inserts '/0' after inserted substring.
    }

    return path;
}


// INPUT HISTORY --------------------------------


void save_input_to_history(char* input) 
{
    // checking if entered command is identical to previous
    if (history_index != 0)
    {
        if (strcmp(input, history_buffer[history_index-1]) == 0) 
            return;
    } 

    // saving entered command to history
    strcpy(history_buffer[history_index], input);
    history_index = (history_index + 1) % MAX_HISTORY_SIZE;
}


void display_history() 
{
    for (int i = 0; i < history_index; i++)
    {
        printf(" %d  %s\n", i + 1, history_buffer[i]);
    }
}


void replace_input_with_history(int direction)
{
    
}


// HANDLING USER INPUT --------------------------


void change_terminal_settings()
{
    tcgetattr(STDIN_FILENO, &old_termios);
    raw_termios = old_termios;
    raw_termios.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw_termios);
}


void revert_terminal_settings()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
}


void get_user_input(char *input)
{
    int input_index = 0;
    int cursor_index = 0;
    
    while (true)
    {
        char key = getchar();

        // enter
        if (key == '\n') {
            input[input_index] = '\0';
            printf("\n");
            break;
        }

        // backspace
        if (key == 127)
        {
            if (cursor_index > 0) {
                // removing characters from the end of the input
                if (cursor_index == input_index)
                {
                    putchar('\b');
                    putchar(' ');
                    putchar('\b');
                }
                // removing characters from the middle of the input
                else
                {
                    // shifting all characters after cursor position back one spot
                    for (int temp_index = cursor_index - 1; temp_index < input_index; temp_index++)
                    {
                        input[temp_index] = input[temp_index + 1];
                    }
                    input[input_index] = '\0';

                    // moving cursor back to begining of input
                    for (int temp_index = cursor_index; temp_index > 0; temp_index--)
                    {
                        printf("\b");
                    }

                    // re-printing input without removed character
                    for (int temp_index = 0; temp_index < input_index - 1; temp_index++)
                    {
                        putchar(input[temp_index]);
                    }
                    // covering up last character with blank space
                    putchar(' ');

                    // moving back cursor to proper position
                    for (int temp_index = input_index; temp_index > cursor_index - 1; temp_index--)
                    {
                        printf("\b");
                    }
                }

                // updating cursor and end of string
                input_index--;
                cursor_index--;
            }
            continue;
        }

        // tab
        if (key == '\t')
        {
            continue;
        }

        // escape character for arrow keys
        if (key == '\033')
        {
            getchar();  // getting '[' character
            switch (getchar())
            {
            case 'A':   // up arrow - replace input with previous input from history
                if (retrieve_index > 0)
                {
                    retrieve_index--;
                    int i;
                    for (i = 0; i < input_index; i++) printf("%c", '\b');   // move cursor back to input begining
                    for (i = 0; i < input_index; i++) printf("%c", ' ');    // overwriting all characters entered with empty space
                    for (i = 0; i < input_index; i++) printf("%c", '\b');   // move cursor back to input begining
                    printf("%s", history_buffer[retrieve_index]);           // insert command from history as input
                    strcpy(input, history_buffer[retrieve_index]);
                    input_index = strlen(history_buffer[retrieve_index]);
                    cursor_index = strlen(history_buffer[retrieve_index]);
                }
                break;

            case 'B':   // down arrow - replace input with next input from history
                if (retrieve_index < history_index)
                {
                    retrieve_index++;
                    int i;
                    for (i = 0; i < input_index; i++) printf("%c", '\b');   // move cursor back to input begining
                    for (i = 0; i < input_index; i++) printf("%c", ' ');    // overwriting all characters entered with empty space
                    for (i = 0; i < input_index; i++) printf("%c", '\b');   // move cursor back to input begining
                    printf("%s", history_buffer[retrieve_index]);           // insert command from history as input
                    strcpy(input, history_buffer[retrieve_index]);
                    input_index = strlen(history_buffer[retrieve_index]);
                    cursor_index = strlen(history_buffer[retrieve_index]);
                }
                break;

            case 'C':   // right arrow - move the cursor right one character
                if (cursor_index < input_index)
                {
                    printf("%c", input[cursor_index]);
                    cursor_index++;
                }
                else alert();
                break;

            case 'D':   // left arrow - move the cursor left one character
                if (cursor_index > 0)
                {
                    cursor_index--;
                    printf("\b");
                }
                else alert();
                break;
            }

            continue;
        }

        if (cursor_index == input_index)
        {
            input[input_index++] = key;
            cursor_index++;
            putchar(key);
        }
        else 
        {
            // shifting all charcters after cursor by one spot
            input_index++;
            for (int temp_index = input_index; temp_index > cursor_index; temp_index--)
            {
                input[temp_index] = input[temp_index-1];
            }

            // inserting character into middle of string
            input[cursor_index] = key;
            cursor_index++;
            putchar(key);

            // re-printing all characters from input, but shifted by one position
            for (int temp_index = cursor_index; temp_index < input_index; temp_index++)
            {
                putchar(input[temp_index]);
            }

            // moving cursor back to inserted character
            for (int temp_index = input_index; temp_index > cursor_index; temp_index--)
            {
                printf("\b");
            }
        }
    }
}


void tokenize(char *input)
{
    arguments[MAX_INPUT_LENGTH/2];
    argument_count = 0;
    
    char *token = strtok(input, " \t");
    while (token != NULL)
    {
        arguments[argument_count] = token;
        token = strtok(NULL, " \t");
        argument_count++;
    } 
    arguments[argument_count] = NULL;
}


// SHELL COMMANDS -------------------------------


void change_directory()
{
    if (argument_count > 2)
    {
        alert();
        printf("cd: too many arguments\n");
        return;
    }

    
    char destination[MAX_PATH_LENGTH];
    if (argument_count == 1) strcpy(destination, getenv("HOME"));   // if no destination was specified, the home directory is set as the destination
    else strcpy(destination, arguments[1]);     // second argument passed by user is the destination

    // Saves current working directory for updating last_directory later.
    char temp_directory[MAX_PATH_LENGTH];
    getcwd(temp_directory, sizeof(temp_directory));

    // note to self: should replace below if statement with code that inserts getenv("HOME") in place of '~' character
    if (strcmp(destination, "~") == 0)
    {
        chdir(getenv("HOME"));
    }
    else if (strcmp(destination, "-") == 0)
    {
        printf("%s\n", last_directory);
        chdir(last_directory);
    }
    else
    {
        if(chdir(destination) != 0)
        {
            alert();
            printf("cd: %s: No such file or directory\n", destination);
        }
    }

    // Updating last_directory location.
    strcpy(last_directory, temp_directory);
}


void help()
{
    printf("\n"
            BOLD"IMPLEMENTED COMMANDS:\n"
            COMMAND_COLOR"cd"RESET" - change current working directory\n"
            COMMAND_COLOR"clear"RESET" - clear terminal screen\n"
            COMMAND_COLOR"exit"RESET" - terminate program\n"
            COMMAND_COLOR"help"RESET" - display implemented commands and project specification\n"
            COMMAND_COLOR"history"RESET" - display previously entered commands\n"
            COMMAND_COLOR"ls"RESET" - list directory contents\n"
            "   [-a]  do not ignore entries starting with .\n"
            "   [-g]  like -l, but do not list owner\n"
            "   [-G]  in a long listing, don't print group names\n"
            "   [-l]  use a long listing format\n"
            "   [-p]  append / indicator to directories\n"
            "   [-Q]  enclose entry names in double quotes\n"
            COMMAND_COLOR"mv"RESET" - move (rename) files\n"
            "   Usage: mv {source} {destination}\n"
            "   Rename {source} to {destination}, or move {source} to {directory}.\n"
            "   Directories moved (renamed) recursively.\n"
            "\n"
            BOLD "BONUS FEATURES:\n" RESET
            "- extensive control of key input\n"
            "- history accessible through up/down arrow keys\n"
            "- shortened home directory path\n"
            "- color coded text\n"
            "- custom startup animation\n"
            "- sound alert for errors\n"
            "- user login and host name\n"
            "\n"
            "Written by Wojciech Kubicki.\n"
            "\n");
}


void move_file(char *source, char *destination)
{
    char byte;

    FILE *source_file;
    source_file = fopen(source, "r");
    if (source_file == NULL)
    {
        alert();
        printf("mv: cannot stat '%s': No such file or directory\n", source);
        return;
    }

    FILE *destination_file;
    destination_file = fopen(destination, "w");
    if (destination_file == NULL)
    {
        alert();
        printf("mv: cannot stat '%s': No such file or directory\n", destination);
        return;
    }

    while ((byte = fgetc(source_file)) != EOF)
    {
        fputc(byte, destination_file);
    }

    fclose(source_file);
    fclose(destination_file);

    remove(source);
}


void move_directory(char *source, char *destination)
{
    DIR *directory = opendir(source);
    if (directory == NULL)
    {
        alert();
        printf("mv: '%s' no such file or directory", source);
        return;
    }

    mkdir(destination, 0777);

    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL)
    {
        char child_source[MAX_PATH_LENGTH];
        strcpy(child_source, source);
        strcat(child_source, "/");
        strcat(child_source, entry->d_name);

        char child_destination[MAX_PATH_LENGTH];
        strcpy(child_destination, destination);
        strcat(child_destination, "/");
        strcat(child_destination, entry->d_name);

        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            move_directory(child_source, child_destination);
        }
        else if (entry->d_type == DT_REG)
        {
            move_file(child_source, child_destination);
        }
    }

    closedir(directory);
    rmdir(source);
}


void move()
{
    if (argument_count == 1)
    {
        alert();
        printf("cp: missing file operand\n");
        return;
    }

    if (argument_count == 2)
    {
        alert();
        printf("cp: missing destination file operand after '%s'\n", arguments[1]);
        return;
    }
    
    if (argument_count > 3)
    {
        alert();
        printf("cp: too many arguments\n");
        return;
    }

    char source[MAX_INPUT_LENGTH];
    strcpy(source, arguments[1]);
    int source_type = check_file_type(source);

    char destination[MAX_INPUT_LENGTH];
    strcpy(destination, arguments[2]);
    int destination_type = check_file_type(destination);

    if (source_type == REGULAR_FILE && destination_type == REGULAR_FILE)
    {
        move_file(source, destination);
    }
    else if (source_type == REGULAR_FILE && destination_type == DIRECTORY)
    {
        char *filename = basename(source);
        strcat(destination, "/");
        strcat(destination, filename);
        move_file(source, destination);
    }
    else if (source_type == DIRECTORY && destination_type == DIRECTORY)
    {
        strcat(destination, "/");
        strcat(destination, basename(source));
        move_directory(source, destination);
    }
    else if (source_type == DIRECTORY)
    {
        // if there are no '/' characters in the destination, the source will just be renamed
        if (strchr(destination, '/'))
        {
            // checking if the destination path exists, if yes, then the source directory will be moved under a new name
            char destination_path[MAX_PATH_LENGTH];
            strcpy(destination_path, destination);
            dirname(destination_path);
            if (check_file_type(destination_path) == DIRECTORY)
            {  
                move_directory(source, destination);
            }
            else
            {
                printf("cp: cannot overwrite non-directory '%s' with directory '%s'\n", destination, source);
                return;
            }
        }
        else
        {
            move_directory(source, destination);
        }
    }
}


void list()
{
    char path[MAX_PATH_LENGTH] = ".";
    bool a = false, g = false, G = false, i = false, l = false, p = false, Q = false;
    for (int i = 1; i < argument_count; i++)
    {
        if (arguments[i][0] == '-')
        {
            for (int j = 1; j < strlen(arguments[i]); j++)
            {
                char flag = arguments[i][j];
                if (flag == 'a')
                {
                    a = true;
                }
                else if (flag == 'g')
                {
                    g = true;
                }
                else if (flag == 'G')
                {
                    G = true;
                }
                else if (flag == 'i')
                {
                    i = true;
                }
                else if (flag == 'l')
                {
                    l = true;
                }
                else if (flag == 'p')
                {
                    p = true;
                }
                else if (flag == 'Q')
                {
                    Q = true;
                }
            }
        }
        else
        {
            strcpy(path, arguments[i]);
        }
    }

    DIR* directory = opendir(path);
    if (directory == NULL)
    {
        alert();
        printf("ls: could not open directory\n");
        return;
    }

    long int total_entries = 0;

    // reading every file in directory
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL)
    {
        char name[MAX_PATH_LENGTH];
        strcpy(name, entry->d_name);

        struct stat file_stat;
        stat(name, &file_stat);

        // arguments applied BEFORE entry has been displayed --------

        // [-a] do not ignore entries starting with .
        if (!a)
        {
            if (name[0] == '.') 
                continue;
        }

        // -l  use a long listing format
        // -g  like -l, but do not list owner
        if (l || g) 
        {
            if (S_ISDIR(file_stat.st_mode)) printf("d");
            else printf("-");
            if (file_stat.st_mode & S_IRUSR) printf("r");
            else printf("-");
            if (file_stat.st_mode & S_IWUSR) printf("w");
            else printf("-");
            if (file_stat.st_mode & S_IXUSR) printf("x");
            else printf("-");
            if (file_stat.st_mode & S_IRGRP) printf("r");
            else printf("-");
            if (file_stat.st_mode & S_IWGRP) printf("w");
            else printf("-");
            if (file_stat.st_mode & S_IXGRP) printf("x");
            else printf("-");
            if (file_stat.st_mode & S_IROTH) printf("r");
            else printf("-");
            if (file_stat.st_mode & S_IWOTH) printf("w");
            else printf("-");
            if (file_stat.st_mode & S_IXOTH) printf("x");
            else printf("-");
            printf(" ");

            printf("%ld ", file_stat.st_nlink);

            // like -l, but do not list owner
            if (!g) 
            {
                struct passwd *userid = getpwuid(file_stat.st_uid);
                printf("%s ", userid->pw_name);
            }

            // [-G] in a long listing, don't print group names
            if (!G) 
            {
                struct passwd *groupid = getpwuid(file_stat.st_gid);
                printf("%s ", groupid->pw_name);
            }

            printf("%ld ", file_stat.st_size);
            total_entries += file_stat.st_size;

            struct tm *time_st = localtime(&file_stat.st_ctime);  // no st_birthtime member, had to use st_ctime - Linux doesn't support it? Seems that only MacOS does :(
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

        // [-Q] enclose entry names in double quotes
        if (Q)
        {
            printf("\"");
        }

        // prints entry in directory
        if (entry->d_type == DT_DIR)
        {
            printf(BLUE);
        }
        else if (entry->d_type == DT_LNK)
        {
            printf(CYAN);
        }
        else if (entry->d_type == DT_REG && file_stat.st_mode & S_IXUSR)
        {
            printf(GREEN);
        }

        printf("%s" RESET, name);

        // arguments applied AFTER file has been displayed ----------

        // [-Q] enclose entry names in double quotes
        if (Q)
        {
            printf("\"");
        }

        // -p  append / indicator to directories
        if (p && entry->d_type == DT_DIR) 
        {
            printf("/");
        }

        printf("\n");
    }

    closedir(directory);

    // [-l] use a long listing format -> this includes the total number of entries
    if (l) 
    {
        printf("total %ld\n", total_entries);
    }
}


// ----------------------------------------------


int main()
{
    printf("Welcome to my microshell project! Use the 'help' command to view the project specifications.\n\n");

    // getting environment varaibles
    login = getenv("USER");
    host = getenv("NAME");
    home = getenv("HOME");

    while (true) {
        char input[MAX_INPUT_LENGTH];
        retrieve_index = history_index;    // updates the command which the user will retrieve with up arrow key to the last saved command

        getcwd(cwd, sizeof(cwd));
        strcpy(cwd, replace_symbol(cwd, home, "~"));

        // displaying: [user@host:path]$
        printf("|"USER_COLOR"%s"AT_COLOR"@"HOST_COLOR"%s"RESET":"PATH_COLOR"%s"RESET"|$ ", login, host, cwd);

        change_terminal_settings();

        get_user_input(input);

        revert_terminal_settings();

        if (strlen(input) != 0)     // only operate on non empty input
        {
            // saving input to history
            save_input_to_history(input);

            // tokenizing input into arguments
            tokenize(input);

            // find command entered by user
            if (strcmp(arguments[0], "alert") == 0) 
            {
                alert();
            }
            else if (strcmp(arguments[0], "cd") == 0) 
            {
                change_directory();
            } 
            else if (strcmp(arguments[0], "clear") == 0) 
            {
                clear_terminal();
            } 
            else if (strcmp(arguments[0], "mv") == 0) 
            {
                move();
            }
            else if (strcmp(arguments[0], "exit") == 0) 
            {
                clear_terminal();
                return 0;
            } 
            else if(strcmp(arguments[0], "help") == 0) 
            {
                help();
            } 
            else if (strcmp(arguments[0], "history") == 0) 
            {
                display_history();
            }
            else if (strcmp(arguments[0], "ls") == 0)
            {
                list();
            }
            else 
            {
                if (fork() == 0) {
                    exit(execvp(arguments[0],arguments));
                }
                else {
                    int status = 0;
                    wait(&status);
                    if(status == 65280) {
                        alert();
                        printf("%s: command not found\n", arguments[0]);
                    }
                }
            }
        }
    }
}
