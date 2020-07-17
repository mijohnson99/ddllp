#include "ddllp.h"

/*
 *		Grammar Definition
 */

static Term *grammar[][NUM_TERMS] = {
	[EXPR] = {
		RULE {TERM, OP_ADD, EXPR, END},
		RULE {TERM, OP_SUB, EXPR, END},
		RULE {TERM, END},
		NULL
	},
	[TERM] = {
		RULE {PRODUCT, OP_MUL, TERM, END},
		RULE {PRODUCT, OP_DIV, TERM, END},
		RULE {PRODUCT, END},
		NULL
	},
	[PRODUCT] = {
		RULE {LPAREN, EXPR, RPAREN, END},
		RULE {LIT_INTEGER, END},
		NULL
	},
};

/*
 *		Function Definitions
 */

static inline bool is_terminal(Term t)
{
	return t < NONTERMINALS;
}

char *ltrim(char *s)
{
	while (*s && *s <= ' ')
		s++;
	return s;
}

int terminal_satisfied(Term t)
{ // Returns number of input symbols to skip to pass t if satisfied
	char *s = input_stream;
	int i = ltrim(s) - s; // Skip whitespace
	char c = s[i];
	// Depending on term:
	switch (t) {
	case END:
		return c == '\0' || c == '\n' ? i+1 : 0;
	case OP_ADD:
		return c == '+' ? i+1 : 0;
	case OP_SUB:
		return c == '-' ? i+1 : 0;
	case OP_MUL:
		return c == '*' ? i+1 : 0;
	case OP_DIV:
		return c == '/' ? i+1 : 0;
	case LIT_INTEGER:
		if (c < '0' || c > '9')
			return 0;
		while (c >= '0' && c <= '9')
			c = s[++i];
		return i;
	case LPAREN:
		return c == '(' ? i+1 : 0;
	case RPAREN:
		return c == ')' ? i+1 : 0;
	default:
		return 0;
	}
}

Production *consume_terminal(Term t)
{
	if (!terminal_satisfied(t))
		return NULL;
	Production *p = malloc(sizeof(Production));
	p->type = t;
	// Apply scanning (if necessary)
	char *s = ltrim(input_stream);
	long int n = 0;
	switch (t) {
	case OP_ADD:
	case OP_SUB:
	case OP_MUL:
	case OP_DIV:
	case LPAREN:
	case RPAREN:
		input_stream = s+1;
		break;
	case LIT_INTEGER:
		for (char c = *s; c >= '0' && c <= '9'; c = *(++s))
			n = (n * 10) + (c - '0');
		input_stream = s;
		p->val.i = n;
		break;
	default:
		break;
	}
	return p;
}

int term_satisfied(Term t)
{
	if (is_terminal(t)) {
		return terminal_satisfied(t);
	} else {
		// Memoize satisfaction check results
		int subterm_met[NUM_TERMS];
		for (int i = 0; i < NUM_TERMS; i++)
			subterm_met[i] = -1; // >0 = satisfied length; 0 = not satisfied; <0 = not checked
		// Check each Rule for nonterminal Term
		for (int i = 0; grammar[t][i] != END; i++) { // For each Rule satisfying the Term
			int skipped = 0;
			for (int j = 0; grammar[t][i][j] != END; j++) { // For each Term in the Rule
				Term q = grammar[t][i][j];
				if (subterm_met[q] < 0)
					subterm_met[q] = term_satisfied(q);
				if (subterm_met[q] > 0) {
					int last_result = subterm_met[q];
					skipped += last_result;
					input_stream += last_result;
					continue;
				} else if (subterm_met[q] == 0) {
					goto failure;
				}
			}
success:		// Expected fall through label
			input_stream -= skipped;
			return skipped;
failure:
			input_stream -= skipped;
			continue;
		}
		return 0;
	}
}

Production *consume_term(Term t)
{
	return NULL; // TODO
}



/*
 *		Test program
 */

int main()
{
	char buf[1024];
	while (fgets(buf, sizeof(buf), stdin)) {
		input_stream = buf;
		input_stream += term_satisfied(EXPR);
		if (terminal_satisfied(END))
			printf("Valid expression!\n");
	}
}
