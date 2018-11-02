#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
	int ty;			// A type of token
	int val;		// When ty is TK_NUM, numeric value
	char *input;
} Token;

enum {
	TK_NUM = 256,
	TK_EOF,			// A token representing the end of input
};

enum {
	ND_NUM = 256,	// A type of integer node
};

typedef struct Node {
	int ty;				// Operator or ND_NUM
	struct Node *lhs;	// Left side
	struct Node *rhs;	// Right side
	int val;			// Use only when ty is ND_NUM
} Node;

Node *expr();

// Save the token string as the tokenized result in this array
// It is assumed that no more than 100 tokens will come
Token tokens[100];

int pos;

Node *new_node(int op, Node *lhs, Node *rhs) {
	Node *node = malloc(sizeof(Node));
	node->ty = op;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_NUM;
	node->val = val;
	return node;
}


Node *term() {
	if (tokens[pos].ty == TK_NUM) {
		return new_node_num(tokens[pos++].val);
	}

	if (tokens[pos].ty == '(') {
		pos++;
		Node *node = expr();
		if (tokens[pos].ty != ')'){
			//error
		}
		pos++;
		return node;
	}

	return NULL;
}

Node *mul() {
	Node *lhs = term();
	if (tokens[pos].ty == TK_EOF)  return lhs;
	if (tokens[pos].ty == '*') {
		pos++;
		return new_node('*', lhs, mul());
	}
	if (tokens[pos].ty == '/') {
		pos++;
		return new_node('/', lhs, mul());
	}

	return lhs;
}

Node *expr() {
	Node *lhs = mul();
	if (tokens[pos].ty == TK_EOF)  return lhs;
	if (tokens[pos].ty == '+') {
		pos++;
		return new_node('+', lhs, expr());
	}
	if (tokens[pos].ty == '-') {
		pos++;
		return new_node('-', lhs, expr());
	}

	return lhs;
}

void gen(Node *node) {
	if (node->ty == ND_NUM) {
		printf("	push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("	pop rdi\n");
	printf("	pop rax\n");

	switch (node->ty){
		case '+':
			printf("	add rax, rdi\n");
			break;
		case '-':
			printf("	sub rax, rdi\n");
			break;
		case '*':
			printf("	mul rdi\n");
			break;
		case '/':
			printf("	mov rdx, 0\n");
			printf("	div rdi\n");
	}

	printf("	push rax\n");
}

// split into tokens the string pointed by p and save.
void tokenize(char *p){
	int i = 0;
	while (*p) {
		// Skip space
		if (*p == ' ' ) {
			p++;
			continue;
		}

		if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
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

	// Code generation while descending AST
	gen(expr());

	// The value of expression remains on the top of the stack
	// Load it into RAX and use it as a return value from the function
	printf("	pop rax\n");
	printf("	ret\n");
	return 0;
}
