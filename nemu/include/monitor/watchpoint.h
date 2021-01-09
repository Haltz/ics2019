#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

#define MAX_EXPR_LEN 200

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	/* TODO: Add more members if necessary */

	char expr[MAX_EXPR_LEN];
	int val;

} WP;

WP *return_head();
WP *new_wp();
bool free_wp(int n);
void print_all_wp();
bool check_wp();

#endif
