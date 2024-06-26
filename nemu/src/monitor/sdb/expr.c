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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdbool.h>
#include <memory/vaddr.h>

// for C environment
// #define true	1
// #define false	0
// #define bool	_Bool

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  // TK_NEQ, TK_AND, TK_DEREF, TK_REG, TK_HEX, TK_DEC
  TK_NEQ, TK_AND, TK_DEREF, TK_REG, TK_DEC, TK_HEX
};

// 结构体数组
static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\-", '-'},
  {"\\*", '*'},         // multiply or TK_DEREF
  {"\\/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
  {"==", TK_EQ},       
  {"!=", TK_NEQ},       
  {"&&", TK_AND},
  {"\\$[a-zA-Z0-9]+",TK_REG},
  {"0x[0-9a-fA-F]+",TK_HEX},
  {"[0-9]+", TK_DEC},  // 按照顺序匹配，有限 HEX, 防止 0x123 中的 0 被读成 DEC
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);    // 这是 POSIX 正则表达式库中的函数，将提供的规则编译成进行 pattern 匹配的内部信息
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;


static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

    // 字符匹配调试信息
        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_DEC: case TK_HEX:
          {
            if(substr_len > 31) {
              Log("The input number is too long!");
              return false;
            }
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            ++nr_token;
            break;
          }
          case TK_NOTYPE: break;
          default: 
          {
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            ++nr_token;
            break;
          }
        }

        
        if(nr_token > 32) {
          Log("The token number of the input expression is too large!");
          return false;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  } // end while
  return true;
}


bool check_parentheses(size_t p, size_t q);
uint32_t eval(int p, int q){
// split the expression into two parts, and evaluate them recursively
// p for the start position of the expression, q for the end position of the expression
  bool check_par = check_parentheses(p, q);
  if(p > q){
    /* Bad expression */
    Log("Invalid expression!");   // 严格上说是非法的，为了程序运行可以 return 0
    return 0;
  }
  else if(p == q){
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    if(tokens[p].type == TK_REG)
    {
      bool success = true;
      uint32_t reg_val = isa_reg_str2val(tokens[p].str+1, &success);
      if (!success) {Log("Invalid register name!"); return 0;}
      else return reg_val; 
    }
    else if(tokens[p].type == TK_DEC) return atoi(tokens[p].str);
    else if(tokens[p].type == TK_HEX) return strtol(tokens[p].str, NULL, 16);
    else return 0;
  }
    else if(tokens[p].type == TK_DEREF){    
    vaddr_t addr = eval(p+1, q);  // 不能仅仅是 p+1 的值，因为解引用后面可能是一个表达式
    return vaddr_read(addr, 4);
  }
  else if(check_par){
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case,
     * the expression can't be reduced to the value of the enclosed expression.
     */
    return eval(p + 1, q - 1);
  }
  else{
    // find the dominant operator, since we have judged and discarded parentheses, the expr can't be warped by paired parentheses
    int op = p;
    int cnt = 0;
    bool and_appear = false;
    bool eq_neq_appear = false;
    bool plus_minus_appear = false;
    for(int i = p; i <= q; ++i){
      if(tokens[i].type == '(') ++cnt;    // 主运算符不可能在 （） 里面, 因此要保证左右括号相消
      else if(tokens[i].type == ')') --cnt;
      else if(cnt == 0){
        if (tokens[i].type == TK_AND){
          op = i;
          and_appear = true;
          continue;
        }
        else if(!and_appear && (tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ)){
          op = i;
          eq_neq_appear = true;
          continue;
        }
        else if(!and_appear && !eq_neq_appear && (tokens[i].type == '+' || tokens[i].type == '-')){   // [只考虑加减乘除括号] 在有 + 或 - 的时候，主运算符一定是最后一个 + 或 -，当然前提是不能再括号内部
          op = i;
          plus_minus_appear = true;
          continue;
        }
        else if(tokens[i].type == TK_DEC || tokens[i].type == TK_HEX || tokens[i].type == TK_DEREF || tokens[i].type == TK_REG){    // 不会是数字/寄存器/解引用等，因此跳过
          continue;
        }
        else if(!and_appear && !eq_neq_appear && !plus_minus_appear && (tokens[i].type == '*' || tokens[i].type == '/') ){    // 没有 + 或 - 的时候，主运算符是最后一个 * 或 /
          op = i;
          continue;
        }
      }
    }
    if(cnt) Log("Invalid expression!");
    // calculate the value of each side and combine them
    uint32_t val1 = eval(p, op - 1);
    uint32_t val2 = eval(op + 1, q);
    switch(tokens[op].type){
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      case TK_EQ: return val1 == val2;
      case TK_NEQ: return val1 != val2;
      case TK_AND: return (!val1) ? 0: (!(val2 == 0));   // 考虑短路运算
      default: assert(0);
    }
  }
}

//p 1&&(0x123+($s1+1)*10-0x12C)

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  // print the content of tokens
  // for(int i = 0; i < 32; ++i) {
  //   Log("tokens[%d].type = %d, tokens[%d].str = %s", i, tokens[i].type, i, tokens[i].str);
  // }
  
  /* TODO: Insert codes to evaluate the expression. */
  
  // tell between multiply & dereference
  for (int i = 0; i < nr_token; i ++) {
  if (tokens[i].type == '*' && (i == 0 || tokens[i - 1].type == '+' || tokens[i - 1].type == '-'  || tokens[i - 1].type == '*' || tokens[i - 1].type == '/' || tokens[i - 1].type == TK_EQ || tokens[i - 1].type == TK_NEQ || tokens[i - 1].type == TK_AND)) {
    tokens[i].type = TK_DEREF;
  }
}

  return eval(0, nr_token-1);
}



bool check_parentheses(size_t p, size_t q)
{
// 不应仅仅判断 p 是 '(' , 而 q 是 ')', 否则在 （1+2）* (3+4)  会返回正确，实际并非正确
  int cnt = 0;
  int i = p;
  for(; i <= q; ++i)
  {
    if(tokens[i].type == '(') {++cnt;}
    else if(tokens[i].type == ')') {--cnt;}
    if(cnt == 0 && tokens[p].type == '(' && tokens[q].type == ')') {break;}   // 第一轮不符合仍然继续循环
  }
  if(i == q)  return true; else return false;
  return false;
}
// Stack Overflow 大佬给的其他建议：1. 变量规范，数组下标用 size_t;  2. q 一般定义为数组末尾的下一个元素，这样循环使用 < ，同时也方便直接传入数组的大小；当然对于我们的框架代码不需要这么做

// 注意虽然看似一致，但是下面的 run 不了
  // int i = p;
  // for(; i <= q; ++i)
  // {
  //   if(tokens[i].type == '(') {++cnt;}
  //   else if(tokens[i].type == ')') {--cnt;}
  //   if(cnt == 0 && tokens[p].type == '(' && tokens[q].type == ')' && i == q) return true; else return false;   // 第一轮不符合就会 return false
  // }
  // return false;

