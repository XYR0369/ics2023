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

// for C environment
// #define true	1
// #define false	0
// #define bool	_Bool

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_NUM,
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
  {"\\*", '*'},
  {"\\/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
  {"[0-9]+", TK_NUM},  // number
  {"==", TK_EQ},        // equal
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


// testing_code
// x 10 1 +(2*3/4) - 5
// x 10 1+(222*334/4123)-5
// x 10 1+(3*2)/3+4-1

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

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NUM: 
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
  }

  return true;
}

bool check_parentheses(int p, int q)
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
 
// 虽然看似一致，但是下面的 run 不了
  // int i = p;
  // for(; i <= q; ++i)
  // {
  //   if(tokens[i].type == '(') {++cnt;}
  //   else if(tokens[i].type == ')') {--cnt;}
  //   if(cnt == 0 && tokens[p].type == '(' && tokens[q].type == ')' && i == q) return true; else return false;   // 第一轮不符合就会 return false

  // }
  // return false;
}



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
    return atoi(tokens[p].str);
  }
  else if(check_par){
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case,
     * the expression can't be reduced to the value of the enclosed expression.
     */
    return eval(p + 1, q - 1);
  }
  else{
    // find the dominant operator
    int op = p;
    int cnt = 0;
    bool plus_minus_appear = false;
    for(int i = p; i <= q; ++i){
      if(tokens[i].type == '(') ++cnt;    // 主运算符不可能在 （） 里面, 因此要保证左右括号相消
      else if(tokens[i].type == ')') --cnt;
      else if(cnt == 0){
        if(tokens[i].type == '+' || tokens[i].type == '-'){   // 在有 + 或 - 的时候，主运算符一定是最后一个+ 或 -，当然前提是不能再括号内部
          op = i;
          plus_minus_appear = true;
          continue;
        }
        else if(tokens[i].type == TK_NUM){    // 不会是数字，因此跳过
          continue;
        }
        else if((tokens[i].type == '*' || tokens[i].type == '/') && !plus_minus_appear){    // 没有 + 或 - 的时候，主运算符是最后一个 * 或 /
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
      default: assert(0);
    }
  }
}

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
  

  return eval(0, nr_token-1);
}
