#ifndef DDLLP_H
#define DDLLP_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*
 *		Data Structures
 */

typedef enum term {
	END, // Can be used for end-of-line or end-of-pattern
	LIT_INTEGER,
	LPAREN,
	RPAREN,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	// Non-terminals
	NONTERMINALS, // Used for comparisons. TODO: Find a way to check grammar[] instead of comparing to this
	EXPR,
	TERM,
	PRODUCT,
	NUM_TERMS // Used to count total Term types
} Term;

typedef enum term *Rule;

typedef struct prod {
	Term type;				// Term type; indicates value
	union {
		struct {
			Rule origin;
			struct prod *arr;	// Array of productions
		} rule; // TODO: Think of a better name for this member
		long int i;			// Integer
		char *s;			// String
	} val;
} Production;



/*
 *		Grammar Definition
 */

#define RULE (Term [])

static Term *grammar[][NUM_TERMS];



/*
 *		Declarations and Descriptions
 */

char *input_stream; // String input

int terminal_satisfied(Term);
// Check input stream for a valid sequence for Term; return length if found
Production *consume_terminal(Term, Production *);
// Create a Production for a Term from input, advancing the stream

// NB. Whether whitespace is ignored or not depends on the implementations of terminal_satisfied and consume_terminal.
//     If so, the number returned by terminal_satisfied includes whitespace.
// ^ (TODO: Is this a good decision? Or, should whitespace be part of the grammar?)

int term_satisfied(Term);
// Check each rule (or terminal_satisfied) for a given term; return length if found
Production *consume_term(Term, Production *);
// Creates a Production for a Term from input_stream, advancing the stream 
// TODO: How will this function know which Rule to follow?
//       Idea: Have term_satisfied return Rule AND length

#endif
