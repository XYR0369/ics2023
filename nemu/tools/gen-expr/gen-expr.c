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

  // int len = choose(10) + 1;   // 生成一个长度为 1~10 的数字
  //printf("the length of number is %d\n",len);
  // for (int i = 0; i < len; ++i) {   // 循环变量是 i 而不是 len，bug 又调了半小时
  //   buf[position++] = choose(10) + '0';    // 每一位是 0-9
  // }

  uint32_t num = choose(100000);   
  snprintf(buf + position, sizeof(buf) - position, "%u", num);   // 将 num 转换为字符串，存入 buf 中; sizeof(buf) - position 是为了防止溢出
  position = position + (strlen(buf+position));   // 将 position 移到字符串末尾

  //++position;   // 已经加过了，bug 调了半小时
  //printf("the string is %s\n",buf);
}

static void gen_rand_op() {

  switch (choose(4)) {
    case 0: buf[position] = '+'; break;
    case 1: buf[position] = '-'; break;
    case 2: buf[position] = '*'; break;
    case 3: buf[position] = '/'; break;
  }

  ++position;
  // printf("the string is %s\n",buf);
}

static void gen(char c) {
  buf[position] = c;
  ++position;
  // printf("the string is %s\n",buf);
}

static void gen_rand_expr() {
  if (strlen(buf) > 65536) {
    printf("Too long!\n");
    // 清空这次结果
    position = 0;
    memset(buf, 0, sizeof(buf));
    return;
    }
  
  switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); if(choose(6)==0) gen(' ');gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); if(choose(6)==0) gen(' ');gen_rand_expr(); break;
  }
  
}



int main(int argc, char *argv[]) {
  int seed = time(0);   // 由于每次获取的是当前时间，如果在同一秒内命令行多次执行该程序，则会得到相似的结果；因此还是应该使用 for 循环，
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;

  // for(int i = 0; i < 10; i ++){

  //   // 初始化一定不能忘了
  //   position = 0;
  //   memset(buf, 0, sizeof(buf));
      
  //   gen_rand_expr();
  //   *(buf + position) = '\0';./
  
  //   printf("buf:%s\n",buf);}

  for (i = 0; i < loop; i ++) {

// 初始化一定不能忘了
    position = 0;
    memset(buf, 0, sizeof(buf));

    gen_rand_expr();

    sprintf(code_buf, code_format, buf);    // 将 buf 中的表达式和上文中的主函数粘合起来

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");        // popen 打开一个进程，将可执行文件的输出重定向到管道中，以“r”模式打开，返回一个文件指针
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);      // 从文件指针中读取一个整数，也就是 result
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
