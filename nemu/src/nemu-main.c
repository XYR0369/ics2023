/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
void testing_expr();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  testing_expr();

  /* Start engine. */
  //engine_start();
  
  return is_exit_status_bad();    // 正常 return 0，异常 return 1
}






void testing_expr()
{
  # define MAX_LINE_LENGTH 1024

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
      arg[length2-1] = '\0'; // 正常会读入 '\n', 需要将最后一个字符改为 '\0'
      if(arg[length2-2] =='\n') arg[length2-2] = '\0';      // 如果 arg 内原本没有空格，那么会替换原本的 '\0' 为空格，因此要把倒数两个都改成 '\0'
          
      // 输出结果
      printf("argument = '%s'\n", arg);
      printf("expected result = '%s'\n", result);
      uint32_t addr = expr(arg, NULL);
      printf("result = %u\n", addr);

      memset(line, 0, sizeof(line));
    }

    fclose(file); // 关闭文件
   
}