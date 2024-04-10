
#include <stdio.h>
#include <readline/readline.h>

int main()
{
    char *line_read = NULL;
    line_read = readline("(nemu) ");
    printf("%s\n",line_read);
    return 0;
}