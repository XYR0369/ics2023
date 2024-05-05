
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>

int main()
{
    // char str[] = "This is a test string.";
    // char *cmd = strtok(str, " "); 
    // while (cmd != NULL) {
    //     printf("%s\n", cmd);    

       
    //     cmd = strtok(NULL, " "); // 继续分割 str，
    // }
    // return 1;

    char *hexStr = "0x80000000";
    // unsigned long num;
    // sscanf(hexStr, "%lx", &num);
    long num = strtol(hexStr, NULL, 16);

    printf("The hexadecimal representation is: 0x%lx\n", num);
}
