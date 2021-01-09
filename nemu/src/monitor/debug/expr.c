#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#define TOKEN_LEN 200

enum { TK_NOTYPE = 256, TK_EQ, TK_INEQ, TK_HEX, TK_FIG, TK_AND, TK_SHFT, TK_REG, DEREF, NEGA };

static struct rule {
	char *regex;
	int token_type;
} rules[] = {
	{ " +", TK_NOTYPE }, // spaces
	{ "\\(", '(' }, // left parenthesis
	{ "\\)", ')' }, // right parenthesis
	{ "\\+", '+' }, // plus
	{ "-", '-' }, // minus
	{ "\\*", '*' }, // multiply
	{ "/", '/' }, // divide
	{ "==", TK_EQ }, // equal
	{ "!=", TK_INEQ }, // inequal
	{ "\\b0[xX][0-9a-fA-F]+\\b", TK_HEX }, // hexnumber
	{ "[0-9]+", TK_FIG }, //figures
	{ "&&", TK_AND }, // and
	{ "<=", TK_SHFT }, //shift
	{ "\\$[a-zA-Z_]+", TK_REG } //register
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

#define NR_TOKENS 64
static Token tokens[NR_TOKENS] __attribute__((used)) = {};
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
				if (substr_len > TOKEN_LEN) {
					printf("Err: token length overflow.\n");
					return false;
				}
				if (nr_token > NR_TOKENS) {
					printf("Err: token number exceed.\n");
					return false;
				}

				switch (rules[i].token_type) {
				case TK_NOTYPE:
					break;
				case (int)'(':
				case (int)')':
				case (int)'*':
				case (int)'/':
				case (int)'+':
				case (int)'-':
					tokens[nr_token].type = rules[i].token_type;
					//tokens[nr_token].str[0] = e[position - substr_len];
					strncpy(tokens[nr_token].str, substr_start, substr_len);
					nr_token += 1;
					break;
				case TK_EQ:
				case TK_INEQ:
				case TK_SHFT:
				case TK_AND:
					tokens[nr_token].type = rules[i].token_type;
					/*tokens[nr_token].str[0] = e[position - substr_len];
               tokens[nr_token].str[1] = e[position - substr_len + 1];*/
					strncpy(tokens[nr_token].str, substr_start, substr_len);
					nr_token += 1;
					break;
				case TK_FIG:
				case TK_HEX:
					tokens[nr_token].type = rules[i].token_type;
					int l = TOKEN_LEN - 1;
					if (substr_len > l)
						assert(0);
					/*int t = position - substr_len 
               for (int j = 0; j< substr_len; j++){
                tokens[nr_token].str[j] = e[t];
                t+=1;
               }*/
					strncpy(tokens[nr_token].str, substr_start, substr_len);
					nr_token += 1;
					break;
				case TK_REG:
					tokens[nr_token].type = rules[i].token_type;
					strncpy(tokens[nr_token].str, substr_start, substr_len);
					nr_token += 1;
					break;
				default:
					break;
				}

				break;
			}
		}

		if (i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}

		for (int i = 0; i < nr_token; i++) {
			if (tokens[i].type == (int)'*' && (i == 0 || tokens[i - 1].type == (int)'+' || tokens[i - 1].type == (int)'-' || tokens[i - 1].type == (int)'*' || tokens[i - 1].type == (int)'/' || tokens[i - 1].type == (int)'('))
				tokens[i].type = DEREF; //对于解引符号的判断
			else if (tokens[i].type == (int)'-' && (i == 0 || tokens[i - 1].type == (int)'+' || tokens[i - 1].type == (int)'-' || tokens[i - 1].type == (int)'*' || tokens[i - 1].type == (int)'/' || tokens[i - 1].type == (int)'('))
				tokens[i].type = NEGA; //对负号进行判断
		}
	}

	return true;
}

uint32_t expr(char *e, bool *success) {
	if (!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	switch (tokens[0].type) {
	default:
		return 0;
	}

	return 0;
}
