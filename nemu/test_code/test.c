
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>

int main()
{
    // char str[] = "This is a test string.";
    
    // char *cmd = strtok(str, " "); 

    // printf("%s\n",str);
    // while (cmd != NULL) {
    //     printf("%s\n", cmd);    


    //     cmd = strtok(NULL, " "); // 继续分割 str，
    // }
    // printf("%s\n",str);
    // return 0;

    // char *hexStr = "0x80000000";
    // // unsigned long num;
    // // sscanf(hexStr, "%lx", &num);
    // long num = strtol(hexStr, NULL, 16);

    // printf("The hexadecimal representation is: 0x%lx\n", num);

    // unsigned a = 1;
    // unsigned b = 0;
    // unsigned result1 = a/b;   
    // printf("%u\n", result1);
    // printf("hhahahah\n");    

    const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};
    int length = strlen(regs);
    return 0;
}
