
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


    // 定义十进制和十六进制的变量
    // int decimal = 10;
    // int hex = 0x1a;

    // // 进行加法运算
    // int sum = decimal + hex;
    // printf("Sum of decimal %d and hex %X is %d\n", decimal, hex, sum);

    // // 进行减法运算
    // int difference = decimal - hex;
    // printf("Difference of decimal %d and hex %X is %d\n", decimal, hex, difference);

    // // 进行乘法运算
    // int product = decimal * hex;
    // printf("Product of decimal %d and hex %X is %d\n", decimal, hex, product);

    // // 进行除法运算
    // int quotient = hex / decimal;
    // printf("Quotient of hex %X divided by decimal %d is %d\n", hex, decimal, quotient);

    // // 注意：在C语言中，除法运算如果两个操作数都是整数，则结果也是整数，会进行截断。

    // int a = strtol("0x123", NULL, 16);
    // printf("%x\n", a+1);

    printf("0x%x\n",0xa+10);
    
    return 0;
}
