#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "isa/reg.h"

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets() {
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
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
	char *arg = strtok(NULL, " ");
	if (arg == NULL) {
		cpu_exec(1);
	} else {
		int steps;
		sscanf(arg, "%d", &steps);
		cpu_exec((uint64_t)steps);
	}
	return 0;
}

static int cmd_info(char *args) {
	char *arg = strtok(NULL, " ");
	if (strcmp(arg, "r") == 0) {
		isa_reg_display();
		return 0;
	} else if (strcmp(arg, "w") == 0) {
		print_all_wp();
		return 0;
	} else
		return -1;
}

static int cmd_scanm(char *args) {
	// doesn't test it.
	return 0;
}

static int cmd_expr(char *args) {
	bool result;
	printf("%d\n", expr(args, &result));
	return 0;
}

static int cmd_set_watchpoint(char *args) {
	WP *wp = new_wp();
	strcpy(wp->expr, args);
	bool  b	      = true;
	bool *success = &b;
	wp->val	      = expr(wp->expr, success);
	return 0;
}

static int cmd_del_watchpoint(char *args) {
	int n;

	sscanf(args, "%d", &n);

	if (free_wp(n)) {
		printf("Watchpoint deleted.\n");
		return 1;
	}

	return 0;
}

static struct {
	char *name;
	char *description;
	int (*handler)(char *);
} cmd_table[] = { { "help", "Display informations about all supported commands", cmd_help },
		  { "c", "Continue the execution of the program", cmd_c },
		  { "q", "Exit NEMU", cmd_q },
		  { "si", "Step n forward exactly", cmd_si },
		  { "info", "State of registers", cmd_info },
		  { "x", "Memory address", cmd_scanm },
		  { "p", "Compute expression", cmd_expr },
		  { "w", "Set a watchpoint", cmd_set_watchpoint },
		  { "d", "Delete a watchpoint", cmd_del_watchpoint }

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int   i;

	if (arg == NULL) {
		/* no argument given */
		for (i = 0; i < NR_CMD; i++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	} else {
		for (i = 0; i < NR_CMD; i++) {
			if (strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop(int is_batch_mode) {
	if (is_batch_mode) {
		cmd_c(NULL);
		return;
	}

	for (char *str; (str = rl_gets()) != NULL;) {
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if (cmd == NULL) {
			continue;
		}

		/* treat the remaining string as the arguments,
     * which may need further parsing
     */
		char *args = cmd + strlen(cmd) + 1;
		if (args >= str_end) {
			args = NULL;
		}

#ifdef HAS_IOE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for (i = 0; i < NR_CMD; i++) {
			if (strcmp(cmd, cmd_table[i].name) == 0) {
				if (cmd_table[i].handler(args) < 0) {
					return;
				}
				break;
			}
		}

		if (i == NR_CMD) {
			printf("Unknown command '%s'\n", cmd);
		}
	}
}
