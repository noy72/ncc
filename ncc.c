#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	TK_NUM = 256,
	TK_EOF,			// A token representing the end of input
};

typedef struct {
	int ty;			// A type of token
	int val;		// When ty is TK_NUM, the numeric value
	char *input;	// Token string (for error message)
} Token;

// Save the token string as the tokenized result in this array
// It is assumed that no more than 100 tokens will come
Token tokens[100];

// split into tokens the string pointed by p and save.
void tokenize(char *p){
	int i = 0;
	while (*p) {
		// Skip space
		if (*p == ' ') {
			p++;
			continue;
		}

		if (*p == '+' || *p == '-') {
			tokens[i].ty = *p;
			tokens[i].input = p;
			i++;
			p++;
			continue;
		}

		if (isdigit(*p)) {
			tokens[i].ty = TK_NUM;
			tokens[i].input = p;
			tokens[i].val = strtol(p, &p, 10);
			i++;
			continue;
		}

		fprintf(stderr, "cannot tokenize: %s\n", p);
		exit(1);
	}

	tokens[i].ty = TK_EOF;
	tokens[i].input = p;
}

// Function for reporting errors
void error(int i) {
	fprintf(stderr, "Unexpected token: %s\n", tokens[i].input);
	exit(1);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "The number of arguments is incorrect\n");
		return 1;
	}

	tokenize(argv[1]);

	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");

	// Since the beginning of the expression must be a nubmer,
	// check it and output the first mov instruction.
	if (tokens[0].ty != TK_NUM) {
		error(0);
	}

	printf("	mov rax, %d\n", tokens[0].val);

	// Output assemblies while consuming the sequence of tokens `+ <number>` or `- <number>`
	int i = 1;
	while (tokens[i].ty != TK_EOF) {
		if (tokens[i].ty == '+') {
			i++;
			if (tokens[i].ty != TK_NUM){
				error(i);
			}
			printf("	add rax, %d\n", tokens[i].val);
			i++;
			continue;
		}

		if (tokens[i].ty == '-') {
			i++;
			if (tokens[i].ty != TK_NUM) {
				error(i);
			}
			printf("	sub rax, %d\n", tokens[i].val);
			i++;
			continue;
		}

		error(i);
	}

	printf("	ret\n");
	return 0;
}
