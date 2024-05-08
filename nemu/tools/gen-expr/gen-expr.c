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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static int position = 0;

static uint32_t choose(uint32_t n) {
  return rand() % n;    // 对 n 取余
}

static void gen_num() {
  int len = choose(10) + 1;   // 生成一个长度为 1~10 的数字
  for (; position < len; ++position) {
    buf[position] = choose(10) + '0';    // 每一位是 0-10
  }
  buf[position] = '\0';
  ++position;
}

static void gen_rand_op() {
  switch (choose(4)) {
    case 0: buf[position] = '+'; buf[position+1] = '\0'; break;
    case 1: buf[position] = '-'; buf[position+1] = '\0'; break;
    case 2: buf[position] = '*'; buf[position+1] = '\0'; break;
    case 3: buf[position] = '/'; buf[position+1] = '\0'; break;
  }
  ++position;
}

static void gen(char c) {
  buf[position] = c;
  ++position;
}

static void gen_rand_expr() {
    switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;


  gen_rand_expr();
  for(i = 0;i<strlen(buf);++i)
  {
    printf("%s",buf);
  }

  // for (i = 0; i < loop; i ++) {
  //   gen_rand_expr();

  //   sprintf(code_buf, code_format, buf);    // 将 buf 中的表达式和上文中的主函数粘合起来

  //   FILE *fp = fopen("/tmp/.code.c", "w");
  //   assert(fp != NULL);
  //   fputs(code_buf, fp);
  //   fclose(fp);

  //   int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
  //   if (ret != 0) continue;

  //   fp = popen("/tmp/.expr", "r");        // popen 打开一个进程，将可执行文件的输出重定向到管道中，以“r”模式打开，返回一个文件指针
  //   assert(fp != NULL);

  //   int result;
  //   ret = fscanf(fp, "%d", &result);      // 从文件指针中读取一个整数，也就是 result
  //   pclose(fp);

  //   printf("%u %s\n", result, buf);
  // }

  

  return 0;
}
