#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP  wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
	int i;
	for (i = 0; i < NR_WP; i++) {
		wp_pool[i].NO	= i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head  = NULL;
	free_ = wp_pool;
}

inline WP *return_head() {
	return head;
}

WP *new_wp() {
	if (free_ == NULL) {
		printf("free_ is NULL.\n");
		assert(0);
	}

	if (head == NULL) {
		head	   = free_;
		free_	   = free_->next;
		head->next = NULL;
		return head;
	}

	WP *p = head;
	while (p->next != NULL)
		p = p->next;
	p->next = free_;
	free_	= free_->next;
	p	= p->next;
	p->next = NULL;
	return p;
}

bool free_wp(int n) {
	if (head == NULL) {
		printf("head is NULL.\n");
		assert(0);
	}

	WP *wp	 = head;
	int flag = 0;
	while (wp != NULL) {
		if (wp->NO == n) {
			flag = 1;
			break;
		}
		wp = wp->next;
	}

	if (flag == 0) {
		printf("wrong number.\n");
		return false;
	}

	memset(wp->expr, '\0', sizeof(wp->expr) - 1);

	if (wp == head)
		head = wp->next;
	else {
		WP *p = head;
		while (p->next != wp)
			p = p->next;
		p->next = wp->next;
	}

	if (free_ == NULL) {
		free_	 = wp;
		wp->next = NULL;
		return true;
	} else if (free_->next == NULL) {
		if (n > free_->NO) {
			wp->next = free_;
			free_	 = wp;
			return true;
		} else {
			free_->next = wp;
			wp->next    = NULL;
			return true;
		}
	} else {
		WP *p = free_;
		while (p->next != NULL) {
			if (p->NO < n && p->next->NO > n) {
				wp->next = p->next;
				p->next	 = wp;
				return true;
			}
			p = p->next;
		}
		p->next	 = wp;
		wp->next = NULL;
		return true;
	}
	return false;
}

void print_all_wp() {
	if (head != NULL) {
		printf("No.   expr\n ");
		WP *p = head;
		while (p != NULL) {
			printf("%d  %s\n", p->NO, p->expr);
			p = p->next;
		}
	} else
		printf("No watchpoint\n");
}

bool check_wp() {
	WP *  p	      = head;
	bool  b	      = true;
	bool *success = &b;
	int   flag    = 1;
	while (p != NULL) {
		int nv = expr(p->expr, success);
		if (nv != p->val) {
			flag = 0;
			// printf("%d wp's old value is %d\n", p->NO, p->val);
			// printf("%d wp'snew value is %d\n", p->NO, nv);
			p->val = nv;
		}
		p = p->next;
	}
	if (flag == 0)
		return false;
	else
		return true;
}
