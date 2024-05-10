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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/vaddr.h>
#include "common.h"
#include "sdb.h"
#include "utils.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);   // 由于接受的 n 是 unsigned，相当于传入了一个最大的数；n 代表 cpu 循环的次数，理应最大（让程序尽量向后执行直到指令决定何时退出）
  return 0;
}


static int cmd_q(char *args) {
  //修改 nemu_state 以正常退出；正常 nemu_state.state 的修改只在 cpu_exec 中进行
  nemu_state.state = NEMU_QUIT;
  return -1;      // return -1 意味着跳出程序，一定记得修改 state 为 NEMU_QUIT
}

static int cmd_help(char *args);

static int cmd_si(char* args){
    /* extract the first argument */
  char *arg = strtok(NULL, " ");
  if(arg == NULL)  cpu_exec(1);
  else
    cpu_exec(atoi(arg));
  return 0;
  
}

static int cmd_info(char* args){
      /* extract the first argument */
  char *arg = strtok(NULL, " ");
  if(strcmp("r", arg) == 0){    // 由于要比较的对象是 string literal（字符串常量），不能直接用 == 比较，而是要用 strcmp 函数
    isa_reg_display();  // TBD, 如何根据 reg 名称寻找其值？
  }
  else if (strcmp("b", arg) == 0){
    ;
  }
  else{
    printf("Unknown args '%s'\n", args);
  }
  return 0;
}

static int cmd_x(char* args){
  /* extract the first argument */

// 关于 strtok 的用法
// strtok 会改变传入的字符串，将字符串中的分隔符替换为 '\0'(每次调用仅修改遇到的第一个分隔字符)，因此传入的字符串必须是可修改的，如数组，而不能是字符串常量；其返回值为指向传入字符串的每个子串的指针，由于有 '\0'，的分隔，直接打印指针可以输出实际的子串


  char *arg = strtok(NULL, " ");
  if(arg == NULL){
    printf("No first argument given\n");
    return 0;
  }
  // int n = atoi(arg);
  arg = strtok(NULL, " ");
  if(arg == NULL){
    printf("No second argument given\n");
    return 0;
  }

  int length = strlen(arg);   // 将 strtok 更改的字符串改回空格 
  arg[length] = ' ';

  bool success = true;
  vaddr_t addr = expr(arg, &success);
  if(!success){
    Log("Invalid expression");
    return 0;
  }
  printf("%u\n",addr);
  // vaddr_t addr = strtol(arg, NULL, 16);   // 16 进制转换，简单验证正确性
  // for(int i = 0; i < n; i++){
  //   printf("0x%x: 0x%08x\n", addr, vaddr_read(addr, 4));    
  //   addr += 4;
  // }
  return 0;
}

static int cmd_p(char* args){
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  if(arg == NULL){
    printf("No argument given\n");
    return 0;
  }
  bool success = true;
  uint32_t result = expr(arg, &success);
  if(!success){
    Log("Invalid expression");
    return 0;
  }
  printf("0x%x\n", result);
  return 0;
}

static int cmd_w(char* args){
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  if(arg == NULL){
    printf("No argument given\n");
    return 0;
  }
  return 0;
}

static int cmd_d(char* args){
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  if(arg == NULL){
    printf("No argument given\n");
    return 0;
  }
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);    // 函数指针，调用 handler 相当于执行不同的函数
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Execute n instructions", cmd_si},
  { "info", "Print information of register(info r) or breakpoints(info b)", cmd_info},
  { "x", "x N EXPR - using the result of EXPR as the start of memory address and print consecutive N 4-byte values in hexadecimal", cmd_x},
  { "p", "p EXPR - print the value of EXPR", cmd_p},
  { "w", "w EXPR - set a watchpoint if the value of EXPR changes", cmd_w},
  { "d", "d N - delete the Nth watchpoint", cmd_d},

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");   // 字符串第一个位置是 cmd，接一个空格，之后是可能的参数
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;   // 跳过 cmd 和 空格，定位到指令可能的第一个参数地址
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }   // 这里是 return，说明返回 -1 时，直接跳出程序，进行程序执行状态的判断
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
