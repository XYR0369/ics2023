#include <stdio.h>
#include <stdlib.h>
#include  <string.h>

#define MAX_LINE_LENGTH 1024

int main() {
    FILE *file = fopen("selected-input", "r"); // 打开文件
    if (!file) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    char line[MAX_LINE_LENGTH] = {};
    // char strA[MAX_LINE_LENGTH], strB[MAX_LINE_LENGTH];
    
    while (fgets(line, sizeof(line), file)) { // 读取每一行
        char* result = strtok(line, " "); // 以空格为分隔符，分割字符串
        char* arg = strtok(NULL, " "); // 继续分割 str，
        int length1 = strlen(arg);   // 将 strtok 更改的字符串改回空格 
        arg[length1] = ' ';
        int length2 = strlen(arg);
        arg[length2-1] = '\0'; // 正常会读入 '\n', 需要将最后一个字符改为 '\0' (但要注意这样写的话最后一行为了保持一致，需要再空一行)
        if(arg[length2-2] =='\n') arg[length2-2] = '\0';      // 如果 arg 内原本没有空格，那么会替换原本的 '\0' 为空格，因此要把倒数两个都改成 '\0'
            // 输出结果
            printf("str A = '%s'\n", result);
            printf("str B = '%s'\n", arg);

        memset(line, 0, sizeof(line));
    }

    fclose(file); // 关闭文件
    return EXIT_SUCCESS;
}
