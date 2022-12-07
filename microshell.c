#include <stdio.h>              /* printf, scanf */
#include <stdlib.h>             /* exit(0)31m */
#include <string.h>             /* strcmp */
#include <unistd.h>             /* getcwd, chdir */
#include <dirent.h>             /* void ls() - struct dirent, readdir, closedir */
#define RESET ""
#define RED ""
#define GREEN ""
#define BLUE ""
#define YELLOW ""
#define PINK ""
#define PURPLE ""
#define GREY ""
/*
#define RED "\033[91m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define PURPLE "\033[94m"
#define PINK "\033[95m"
#define BLUE "\033[96m"
#define GREY "\033[37m"
#define RESET "\033[0m"
*/

void cd(char *dest)
{
    if(chdir(dest) != 0){
            printf("Blad zmiany biezacego katalogu roboczego:"RED" Nie odnaleziono podanego katalogu\n"RESET);
        }
        else chdir(dest);

    return;
}

void help()                     
{
	printf("Obslugiwane polecenia:\n"
            BLUE"help"RESET"    - wyswietla pelna liste obslugiwanych polecen oraz informacje o projekcie\n"
            BLUE"history"RESET" - wyswietla pelna liste polecen wprowadzonych przez uztykownika\n"
            BLUE"exit"RESET"    - terminates the program\n"
            BLUE"ls"RESET"      - displays all directories and files in directory\n"
            BLUE"pwd"RESET"     - displays path to current working directory\n"
            "Autor: Wojciech Kubicki\n");
}

void history() /* MUST CHANGE - cannot use fopen and fgetc as these are system calls */
{
    FILE *fhistory;
    fhistory = fopen("history.txt", "r");
    char c, last;
    int i = 1;

    if(fhistory == NULL){
        printf("Otwarcie pliku z historia komend nie powiodlo sie.\n");
        return;
    }
    else{
        printf(" 1. ");
        c = fgetc(fhistory);
        while(c != EOF){
            if(last == '\n'){
                if(i<9){
                    printf(" ");
                }
                printf("%d. ",i);
                i++;
            }
            printf ("%c", c);
            last = c;
            c = fgetc(fhistory);
        }
    }
    fclose(fhistory);
    return;
}

void ls()                       
{
    DIR *dir;
    struct dirent *strdir;
    dir = opendir(".");

    while ((strdir = readdir(dir)) != NULL) 
    {
        printf("%s\n", strdir->d_name);
    }
    closedir(dir);
}   

void pwd(char cwd[256]) 
{
    getcwd(cwd, 256);
    printf("%s\n",cwd);
}

void ping()
{
    printf("Pong!\n");
}

int main()
{
	printf(YELLOW"Witaj w moim projekcie Microshell!"RESET"\nUzyj polecenia "BLUE"help"RESET", aby uzyskac pelne specyfikacje projektu.\n");

    char cwd[256];
	char command[256];
    char *param[256];
	FILE *fhistory;
    fhistory = fopen("history.txt", "a");
    int param_count;

	while(1)
	{
        /* Display current work directory | Get command from user | Save command to history */
        getcwd(cwd, sizeof(cwd)); /* MUST CHANGE - getcwd is a system call */
		printf("["GREEN"%s"RESET"]$ ", cwd);
		fgets(command, sizeof(command), stdin);
        fputs(command, fhistory);
        fclose(fhistory);
        /* Tokenize string into paramaters */
        param_count = 0;
        char *token = strtok(command, " \n");
        while(token != NULL){
            param[param_count] = token;
            token = strtok(NULL, " \n");
            param_count++;
        }
        /* Find command chosen by user */
		if(strcmp(param[0],"help")==0){
            help();
		}
		else if (strcmp(param[0],"history")==0){
            history();
		}
		else if(strcmp(param[0],"exit")==0){
			printf(YELLOW"Program zostal zakonczony.\n"RESET"Do zobaczenia!\n");
			exit(0);
		}
        else if(strcmp(param[0],"ls")==0){
            ls();
        }
        else if(strcmp(param[0],"pwd")==0){
            pwd(cwd);
        }
        else if(strcmp(param[0],"ping")==0){
            ping();
        }
        else if(strcmp(param[0],"cd")==0){
            if(param_count < 2){
                printf(RED"Blad zmiany biezacego katalogu roboczego:"RESET" Nie podano docelowego katalogu\n");
            } else cd(param[1]);
        }
        else if(strcmp(param[0], "\n")==0){
            continue;
        }
		else{
			printf(RED"Nieznane polecenie!\n"RESET"Uzyj komendy "BLUE"help"RESET", aby uzyskac pelna liste obslugiwanych polecen.\n");
        }
    }
}
