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

#include "sdb.h"

#define NR_WP 32
#define WP_EXPR_LENGTH 128

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  uint32_t old_val;
  char expr[WP_EXPR_LENGTH];   // this should be enough   
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp()
{
  if(free_ == NULL) {Log("To much watchpoints!");assert(0);}
  if(head == NULL)
  {
    head = free_;
    free_ = free_->next;
    head->next = NULL;
    return head;
  }
  else{
    WP *p = head;
    while(p->next != NULL)
    {
      p = p->next;    // p 指向最后一个节点
    }
    p->next = free_;
    free_ = free_->next;
    p->next->next = NULL;
    return p->next;
  }
}
void free_wp(WP *wp)
{
  if(head == NULL) {Log("No existing watchpoints!"); assert(0);}
  // find wp
  WP *p = head;
  if(p == wp)
  {
    head = wp->next;
    wp->next = free_;
    free_ = wp;
    return;
  }
  while(p->next != NULL)
  {
    if(p->next == wp)
    {
      p->next = wp->next;
      wp->next = free_;   // 加入 free_ 的表头
      free_ = wp;
      return;
    }
    p = p->next;
  }
  Log("No such watchpoint!");
}

void setup_watchpoint(char* e)
{
  WP *wp = new_wp();
  if(strlen(e)>WP_EXPR_LENGTH-1)  {free_wp(wp); Log("Expression too long!");}
  strcpy(wp->expr, e);
  bool success = true;
  wp->old_val = expr(e, &success);
  if(!success) {free_wp(wp); Log("Invalid expression!");}
}

void delete_watchpoint(char* e)
{
  WP *p = head;
  while(p != NULL)
  {
    if(strcmp(p->expr, e) == 0)
    {
      free_wp(p);
      return;
    }
    p = p->next;
  }
  Log("No such watchpoint!");
}

bool check_watchpoint()
{
  WP *p = head;
  while(p != NULL)
  {
    bool success = true;
    uint32_t new_val = expr(p->expr, &success);
    if(!success) {Log("Invalid expression!");}
    if(new_val != p->old_val)
    {
      Log("Watchpoint %d: %s\n", p->NO, p->expr);
      Log("Old value = %u\n", p->old_val);
      Log("New value = %u\n", new_val);
      p->old_val = new_val;
      return true;      // 通常每条指令只会使一个值变化
    }
    p = p->next;
  }
  return false;
}

void disp_watchpoint()
{
  WP *p = head;
  if(p == NULL) {Log("No watchpoints!");}
  while(p != NULL)
  {
    Log("Watchpoint %d: %s\n", p->NO, p->expr);
    p = p->next;
  }
}

