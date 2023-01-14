#include <stdio.h>              /* printf, scanf */
#include <stdlib.h>             /* exit(0)31m */
#include <string.h>             /* strcmp */
#include <unistd.h>             /* getcwd, chdir */
#include <dirent.h>             /* void ls() - struct dirent, readdir, closedir */
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <curses.h>             // getch();
#define RED "\033[91m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define BLUE "\033[94m"
#define PINK "\033[95m"
#define CYAN "\033[96m"
#define RESET "\033[0m"
#define COMMAND_COLOR PINK


char cwd[256] = "/home/";
// 'duszekciapcio' should be changed to '("/home/", getlogin_r, sizeof(getlogin_r));'. As of right now getlogin_r() doesn't seem to be working.
char previous_cwd[256] = "/home/";
char home_directory[256] = "/home/";


void cd(char *destination)
{
    if (strcmp(destination, "~") == 0) {
        // Copy cwd to previous_cwd.
        strncpy(previous_cwd, cwd, sizeof(cwd));

        // Change cwd to home diresctory.
        strncpy(cwd, home_directory, sizeof(home_directory));

        /* change directory here */
    }
    else if (strcmp(destination, ".") == 0) {
        // Do nothing.
    }
    else if (strcmp(destination, "..") == 0) {
        // Copy cwd to previous_cwd.
        strncpy(previous_cwd, cwd, sizeof(cwd));

        // Inseritng null terminator in position of last occurance of '/' in cwd variable.
        char* last_dir = strrchr(cwd, '/');
        int insert_null = last_dir - cwd;
        cwd[insert_null] = '\0';

        // Chcecking if cwd is an empty string. If yes, then set root ('/') as cwd variable.
        if (strcmp(cwd, "\0") == 0) {
            char root = '/';
            strncpy(cwd, &root, 2);
        }

        /* change directory here */
    }
    // BUG ALERT: multiple problems with below command. Ex. despite switching between 'cd ~' and root directory with 'cd -', pwd displays root directory. 
    else if (strcmp(destination, "-") == 0) {
        // Creating swap variable for storing cwd value in order to update the previous_cwd later.
        char swap[sizeof(cwd)];
        strncpy(swap, cwd, sizeof(cwd));

        strncpy(cwd, previous_cwd, sizeof(previous_cwd));
        
        // Copy cwd to previous_cwd.
        strncpy(previous_cwd, swap, sizeof(cwd));

        /* change directory here */
    } 
    else {
        DIR *directory = opendir(destination);

        if (directory) {
            closedir(directory);
            if (strcmp(cwd, "/") != 0) {  // If the cwd isn't the root folder, then append '/' to end of cwd (before appending name of destination folder).
                char slash = '/';
                strncat(cwd, &slash, 1);
            }
            strncat(cwd, destination, sizeof(cwd)+sizeof(destination)); // when using sizeof(destination) '/Microshell' gets cut down to '/Microshe'. Possible fix?
            // Should work now with 'sizeof(cwd)' added.

            /* change directory here */
        }
    }

    // Possibly bypass chdir by changing PWD or PATH environment variable.
    //chdir(destination);
    setenv("PATH", "/home", 1);  // Still doesn't work.  

    return;
}


void help()                     
{
	printf("\nObslugiwane polecenia:\n"
            COMMAND_COLOR"help"RESET" - wyswietla pelna liste obslugiwanych polecen oraz informacje o projekcie\n"
            COMMAND_COLOR"mv"RESET" - przenosi pliki, lub zmienia nazwę pliku\n"
            COMMAND_COLOR"exit"RESET" - konczy prace programu\n"
            "Autor: Wojciech Kubicki\n"
            "Nr Indeksu: 483780\n\n");

    return;
}


void mv(char *source, char *destination)
{
    char character;

    FILE *source_pointer;
    source_pointer = fopen(source, "r");

    FILE *destination_pointer;
    destination_pointer = fopen(destination, "w");

    while (character != EOF) {
        character = fgetc(source_pointer);
        fputc(character, destination_pointer);
    }

    fclose(source_pointer);
    fclose(destination_pointer);

    return;
}


void test();  // This function is used only for testing new code.


int main()
{
    printf("%s\n", getenv("PWD"));

    // Setting starting point as user home directory. This should be later changed to setenv() or putenv().
    chdir("/home/duszekciapcio");
    printf("%s\n", getenv("PWD"));
    putenv("PWD=/home/duszekciapcio/Desktop");
    printf("%s\n", getenv("PWD"));

    // ASCII text art
    printf("           _                    _          _ _ \n"
            " _ __ ___ (_) ___ _ __ ___  ___| |__   ___| | |\n"
            "| '_ ` _ \\| |/ __| '__/ _ \\/ __| '_ \\ / _ \\ | |\n"
            "| | | | | | | (__| | | (_) \\__ \\ | | |  __/ | |\n"
            "|_| |_| |_|_|\\___|_|  \\___/|___/_| |_|\\___|_|_|\n\n"
            "Uzyj polecenia "COMMAND_COLOR"help"RESET", aby uzyskac pelne specyfikacje projektu.\n\n");

	char command[256];
    char *paramater[256];
    int paramater_count;

    char *login = getenv("USER");
    char *host = getenv("NAME");  // Ubuntu doesn't have HOST variable in env, uses 'NAME' instead. Grep returns 'NAME={machine_name}' when searching for machine name.
    strncat(cwd, login, sizeof(cwd)+sizeof(login));
    strncat(previous_cwd, login, sizeof(previous_cwd)+sizeof(login));
    strncat(home_directory, login, sizeof(home_directory)+sizeof(login));

	while(1)
	{
        char shortened_cwd[256];
        strcpy(shortened_cwd, cwd);
        char *home_directory_beginning = strstr(shortened_cwd, home_directory);
        
        if (home_directory_beginning != NULL) {
            memmove(home_directory_beginning + strlen("~"), home_directory_beginning + strlen(home_directory), strlen(home_directory_beginning) - strlen(home_directory) + 1);
            memcpy(home_directory_beginning, "~", strlen("~"));  // Using strcpy() inserts '/0' after inserted substring. 

            // Using username causes the '[' sign to disapear when printing string. Check if this problem also occures on lab compputer.
            printf("["GREEN"%s@%s"RESET":"CYAN"%s"RESET"]$ ", login, host, shortened_cwd);
        } else printf("["GREEN"%s@%s"RESET":"CYAN"%s"RESET"]$ ", login, host, cwd);
        
		fgets(command, sizeof(command), stdin);
        
        if (strcmp(command, "\n")!=0)  // Skip empty input.
        {
            // Tokenizing string into paramaters.
            paramater_count = 0;
            char *token = strtok(command, " \n");
            while(token != NULL){
                paramater[paramater_count] = token;
                token = strtok(NULL, " \n");
                paramater_count++;
            }
            paramater[paramater_count] = NULL;  // This fixed segmentation fold.
        }
        else continue;

        // Checking for command in my microshell function implementations.

		if (strcmp(paramater[0],"help") == 0) 
        {
            help();
		} 
        else if ((strcmp(paramater[0],"exit") == 0) || (strcmp(paramater[0],"q") == 0)) 
        {
			printf("\nProgram zakonczony\n\n");
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(paramater[0],"cd") == 0) 
        {
            if (paramater_count == 1) {
                paramater[1] = "~";
            }
            cd(paramater[1]);
        }
        else if (strcmp(paramater[0], "mv") == 0)
        {
            mv(paramater[1], paramater[2]);
        }
        else if (strcmp(paramater[0], "test") == 0)
        {
            test();
        }
		else  // If entered command isn't implemented in program, run the Linux command.
        {
            if (fork() == 0) {
				exit(execvp(paramater[0],paramater));
			}
			else {
				int status = 0;
				wait(&status);
                if(status == 65280) printf(RED"error code"RESET": %d\n", status);
			}
        } 
    }
}


void test()  // This function is only used for testing new code.
{

    return;
}
