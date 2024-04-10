
#include <stdio.h>
#include <readline/readline.h>

int main()
{
    char *line_read = NULL;
    line_read = readline("(nemu) ");
    char *cmd = strtok(line_read, " ");

    printf("%s\n",cmd);
    return 0;
}