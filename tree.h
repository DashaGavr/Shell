#include "list.h"

enum type_of_next_command {
	ZERO, NXT, AND, OR
};

typedef struct cmd_node* tree; 
typedef struct cmd_node {
	char** 	argv;
	char* 	infile;
	char*	outfile;
	int		backgrnd;
	int 	append;
	struct cmd_node* subcmd;
	struct cmd_node* pipe;
	struct cmd_node* next; 
	enum type_of_next_command type_of_next;
} cmd_node;

int specset (int c);
tree buildtree(list l);
tree level1 ( list *l);
tree level2 ( list *l);
tree level3 ( list *l);
tree level4 ( list *l);
tree level5 ( list *l);
void make_shift(int n);
void print_argv(char **p, int shift);
void print_tree(tree t, int shift);
void deletetree (tree *t);
tree backgrndpipe(tree t);

int notallowed (int c);
int symset (int c);
void error(int i);
char *getword();
list create() ;
void printlist(list l);
void deletelist(list *l);
int check (list ls);
void handler(int signum);