#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

#define TOKEN_LEN 200

enum { TK_NOTYPE = 256, TK_EQ, TK_INEQ, TK_HEX, TK_FIG, TK_AND, TK_SHFT, TK_REG, DEREF, NEGA };
enum { EXPR_INVAL, EXPR_VAL, EXPR_PAREN_MISMATCH };

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
					strncpy(tokens[nr_token].str, substr_start, substr_len);
					nr_token += 1;
					break;
				case TK_EQ:
				case TK_INEQ:
				case TK_SHFT:
				case TK_AND:
					tokens[nr_token].type = rules[i].token_type;
					strncpy(tokens[nr_token].str, substr_start, substr_len);
					nr_token += 1;
					break;
				case TK_FIG:
				case TK_HEX:
					tokens[nr_token].type = rules[i].token_type;
					int l = TOKEN_LEN - 1;
					if (substr_len > l)
						assert(0);
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
			// dereference
			if (tokens[i].type == (int)'*' && (i == 0 || tokens[i - 1].type == (int)'+' || tokens[i - 1].type == (int)'-' || tokens[i - 1].type == (int)'*' || tokens[i - 1].type == (int)'/' || tokens[i - 1].type == (int)'('))
				tokens[i].type = DEREF;
			// negative
			else if (tokens[i].type == (int)'-' && (i == 0 || tokens[i - 1].type == (int)'+' || tokens[i - 1].type == (int)'-' || tokens[i - 1].type == (int)'*' || tokens[i - 1].type == (int)'/' || tokens[i - 1].type == (int)'('))
				tokens[i].type = NEGA;
		}
	}

	return true;
}

int expr_match_parens(int begin_pos, int end_pos) { // examing parentheses
	if (!(tokens[begin_pos].type == (int)'(' && tokens[end_pos].type == (int)')'))
		return 0;

	int cnt = 0;
	for (int j = begin_pos + 1; j < end_pos; j++) {
		if (cnt < 0)
			return 0;

		if (tokens[j].type == (int)'(')
			cnt++;
		else if (tokens[j].type == (int)')')
			cnt--;
	}

	return 1;
}

int find_main_op(int begin_pos, int end_pos) {
	int mul_div[32]; //用来存储*, /的位置，第一个是最靠右的
	int dual[32]; //用来存储==，!=, <=等的位置
	int i = 0, j = 0;
	bool flag = false;
	for (int t = end_pos; t > begin_pos;) {
		if (tokens[t].type == (int)')') {
			int flag = 1;
			while (flag != 0) {
				t -= 1;
				if (tokens[t].type == (int)')')
					flag += 1;
				else if (tokens[t].type == (int)'(')
					flag -= 1;
			}
		} else if (tokens[t].type == (int)'+' || tokens[t].type == (int)'-')
			return t;
		else if (tokens[t].type == (int)'*' || tokens[t].type == (int)'/') {
			mul_div[i] = t;
			flag = true;
			i += 1;
		} else if (tokens[t].type == TK_EQ || tokens[t].type == TK_INEQ || tokens[t].type == TK_AND) {
			dual[j] = t;
			j += 1;
		}
		t -= 1;
	}
	if (flag == true)
		return mul_div[0];
	else
		return dual[0];
}

uint32_t eval(int head, int tail) {
	if (head == tail) { //最简单的表达式形式，一个数字或寄存器
		if (tokens[head].type == TK_FIG) {
			uint32_t number;
			sscanf(tokens[head].str, "%d", &number);
			return number;
		} else if (tokens[head].type == TK_REG) {
			char s[4] = "reg";
			int i = 1, j = 0;
			while (tokens[head].str[i] != '\0' && s[j] != '\0') {
				s[j] = tokens[head].str[i];
				j += 1;
				i += 1;
			}
			bool p = true;
			bool *success = &p;
			return isa_reg_str2val(s, success);
		} else if (tokens[head].type == TK_HEX) {
			uint32_t number;
			number = strtol(tokens[head].str, NULL, 0);
			return number;
		}
	} else if (expr_match_parens(head, tail) == true) //两头都有括号且匹配
		return eval(head + 1, tail - 1);
	else if (tokens[head].type == DEREF && tail - head == 1) { //指针解引符时
		int add;
		add = strtol(tokens[tail].str, NULL, 0);
		vaddr_t data = vaddr_read(add, 32);
		return data;
	} else if (tokens[head].type == NEGA && tail - head == 1) { //负号时
		uint32_t number;
		sscanf(tokens[tail].str, "%d", &number);
		number = -number;
		return number;
	} else { //主运算符处理法
		int op = find_main_op(head, tail);
		uint32_t val1 = eval(head, op - 1);
		uint32_t val2 = eval(op + 1, tail);
		switch (tokens[op].type) {
		case (int)'+':
			return val1 + val2;
			break;
		case (int)'-':
			return val1 - val2;
			break;
		case (int)'*':
			return val1 * val2;
			break;
		case (int)'/':
			return val1 / val2;
			break;
		case TK_EQ:
			return val1 == val2;
			break;
		case TK_INEQ:
			return val1 != val2;
			break;
		case TK_AND:
			return val1 && val2;
			break;
		case TK_SHFT:
			break;
		default:
			break;
		}
	}
	return 0;
}

uint32_t expr(char *e, bool *success) {
	if (!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	int begin_pos = 0, end_pos = nr_token - 1;

	/*check validity of expr*/
	int valid_state = EXPR_VAL;

	if (!(tokens[begin_pos].type == (int)'(' || tokens[begin_pos].type == TK_FIG || tokens[begin_pos].type == NEGA || tokens[begin_pos].type == DEREF || tokens[begin_pos].type == TK_REG || tokens[begin_pos].type == TK_HEX))
		valid_state = EXPR_INVAL;
	if (!(tokens[end_pos].type == (int)')' || tokens[end_pos].type == TK_FIG || tokens[end_pos].type == TK_HEX || tokens[end_pos].type == TK_REG))
		valid_state = EXPR_INVAL;

	for (int i = 0; i < end_pos; i++) {
		switch (tokens[i].type) {
		case (int)'+':
		case (int)'-':
		case (int)'*':
		case (int)'/':
		case (int)'(':
			if (!(tokens[i + 1].type == TK_FIG || tokens[i + 1].type == (int)'(' || tokens[i + 1].type == NEGA || tokens[i + 1].type == DEREF || tokens[i + 1].type == TK_REG || tokens[i + 1].type == TK_HEX))
				valid_state = EXPR_INVAL;
			break;
		case DEREF:
		case NEGA:
			if (!(tokens[i + 1].type == TK_FIG || tokens[i + 1].type == TK_HEX || tokens[i + 1].type == TK_REG))
				valid_state = EXPR_INVAL;
			break;
		case (int)')':
			if (tokens[i + 1].type == TK_FIG || tokens[i + 1].type == (int)'(' || tokens[i + 1].type == TK_REG || tokens[i + 1].type == TK_HEX)
				valid_state = EXPR_INVAL;
			break;
		case TK_FIG:
		case TK_HEX:
		case TK_REG:
			if (tokens[i + 1].type == (int)'(')
				valid_state = EXPR_INVAL;
			break;
		case TK_EQ:
		case TK_INEQ:
		case TK_AND:
			if (!(tokens[i + 1].type == TK_FIG || tokens[i + 1].type == DEREF || tokens[i + 1].type == NEGA || tokens[i + 1].type == TK_REG || tokens[i + 1].type == TK_HEX))
				valid_state = EXPR_INVAL;
			break;
		default:
			valid_state = EXPR_INVAL;
			break;
		}

		if (valid_state == EXPR_INVAL)
			break;
	}

	// check parens match
	int par = 0;
	for (int i = 0; i <= end_pos; i++) {
		if (tokens[i].type == (int)'(')
			par += 1;
		else if (tokens[i].type == (int)')')
			par -= 1;

		if (par < 0) {
			valid_state = EXPR_PAREN_MISMATCH; //右括号多了
			break;
		}
	}
	if (par > 0)
		valid_state = EXPR_PAREN_MISMATCH;

	switch (valid_state) {
	case EXPR_VAL:
		return eval(begin_pos, end_pos);
	case EXPR_INVAL:
		printf("INVAL\n");
		break;
	case EXPR_PAREN_MISMATCH:
		printf("PAREN_ERR\n");
		break;
	}

	return 0;
}
