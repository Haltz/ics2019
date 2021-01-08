#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#define TOKEN_LEN 200

enum { TK_NOTYPE = 256, TK_EQ, TK_NUM, TK_PLUS, TK_SUB, TK_MUL, TK_DIV, TK_LBR, TK_RBR, TK_HEX, TK_REG, TK_AND, TK_OR, TK_DEREF };

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

	{ " +", TK_NOTYPE }, // spaces
	{ "\\+", TK_PLUS }, // plus
	{ "==", TK_EQ }, // equal
	{ "-", TK_SUB }, // substract
	{ "\\*", TK_MUL }, // multiply or derefrence
	{ "/", TK_DIV }, // divide
	{ "0[Xx][0-9a-fA-F]+", TK_HEX }, // hex (must before the TK_NUM)
	{ "[0-9]+", TK_NUM }, // number(dec)
	{ "\\(", TK_LBR }, // left bracket
	{ "\\)", TK_RBR }, // right bracket
	{ "\\$[a-zA-Z]+", TK_REG }, // register
	{ "&&", TK_AND }, // AND
	{ "\\|\\|", TK_OR } // OR
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[TOKEN_LEN];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0') {
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++) {
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

				switch (rules[i].token_type) {
				default:
					TODO();
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

uint32_t expr(char *e, bool *success) {
	if (!make_token(e)) {
		*success = false;
		return 0;
	}

	int p = 0, q = nr_token - 1, v = 1;
	/* TODO: Insert codes to evaluate the expression. */
	switch (tokens[p].type) { return 0; }

	return 0;
}
