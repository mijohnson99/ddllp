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

Production *consume_terminal(Term t, Production *p)
{
	if (!terminal_satisfied(t)) // FIXME: Is this necessary?
		return NULL;
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

Rule last_rule = NULL;
int term_satisfied(Term t)
{
	// If terminal, use other function to check.
	if (is_terminal(t))
		return terminal_satisfied(t);
	// Memoize satisfaction check results
	int subterm_met[NUM_TERMS];
	for (int i = 0; i < NUM_TERMS; i++)
		subterm_met[i] = -1; // >0 = satisfied length; 0 = not satisfied; <0 = not checked
	// Check each Rule for nonterminal Term
	for (int i = 0; grammar[t][i] != END; i++) { // For each Rule satisfying the Term
		int skipped = 0;
		for (int j = 0; grammar[t][i][j] != END; j++) { // For each Term in the Rule
			Term q = grammar[t][i][j];
			if (subterm_met[q] < 0) // If the term has not been checked before:
				subterm_met[q] = term_satisfied(q); // ...do so, recursively.
			if (subterm_met[q] > 0) { // If the term has been satisfied:
				input_stream += subterm_met[q]; // ...move past it and continue.
				skipped += subterm_met[q];
				continue;
			} else if (subterm_met[q] == 0) { // If not, the rule can not be satisfied.
				goto failure;
			}
		}
success:	// Expected fall through label
		last_rule = grammar[t][i];
		input_stream -= skipped;
		return skipped;
failure:
		input_stream -= skipped;
		continue;
	}
	return 0;
}

Production *consume_term(Term t, Production *p)
{
	// Check if the term is satisfied.
	if (!term_satisfied(t))
		return NULL;
	// If terminal, use other function to consume.
	if (is_terminal(t))
		return consume_terminal(t, p);
	// Otherwise, count the Terms in the Rule.
	Rule r = last_rule;
	int n = 0;
	while (r[n] != END)
		n++;
	// Format the Production
	p->type = t;
	p->val.rule.origin = r;
	p->val.rule.arr = malloc(n * sizeof(Production)); // Allocate space for children
	for (int i = 0; i < n; i++)
		consume_term(r[i], &p->val.rule.arr[i]);
	return p;
}



/*
 *		Test program
 */

static const char *term_strings[NUM_TERMS] = {
	[END] = "END",
	[LIT_INTEGER] = "LIT_INTEGER",
	[LPAREN] = "LPAREN",
	[RPAREN] = "RPAREN",
	[OP_ADD] = "OP_ADD",
	[OP_SUB] = "OP_SUB",
	[OP_MUL] = "OP_MUL",
	[OP_DIV] = "OP_DIV",
	[NONTERMINALS] = "NONTERMINALS",
	[EXPR] = "EXPR",
	[TERM] = "TERM",
	[PRODUCT] = "PRODUCT"
};

void print_production(Production *p)
{ // Simple tree traversal
	printf("[%s ", term_strings[p->type]);
	switch (p->type) {
	case LIT_INTEGER:
		printf("%ld", p->val.i);
		break;
	case OP_ADD:
		printf("+");
		break;
	case OP_SUB:
		printf("-");
		break;
	case OP_MUL:
		printf("*");
		break;
	case OP_DIV:
		printf("/");
		break;
	case LPAREN:
		printf("(");
		break;
	case RPAREN:
		printf(")");
		break;
	case END:
		printf("<end>");
		break;
	default: // Non-terminals
		for (int i = 0; p->val.rule.origin[i] != END; i++)
			print_production(&p->val.rule.arr[i]);
	}
	printf("]");
}

void free_children(Production *p)
{
	if (is_terminal(p->type))
		return;
	for (int i = 0; p->val.rule.origin[i] != END; i++)
		free_children(&p->val.rule.arr[i]);
	free(p->val.rule.arr);
}

int main()
{
	char buf[1024];
	while (fgets(buf, sizeof(buf), stdin)) {
		input_stream = buf;
		int len = term_satisfied(EXPR);

		input_stream += len;
		if (terminal_satisfied(END))
			printf("Valid expression!\n");
		else
			continue;
		input_stream -= len;

		Production *p = malloc(sizeof(Production));
		consume_term(EXPR, p);
		print_production(p);
		putchar('\n');

		free_children(p);
		free(p);
	}
}
