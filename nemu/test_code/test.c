
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>

int main()
{
    char str[] = "This is a test string.";
    char *cmd = strtok(str, " "); 
    while (cmd != NULL) {
        printf("%s\n", cmd);    

       
        cmd = strtok(NULL, " "); // 继续分割 str，
    }
}
