#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h> 
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include "tree.h"


tree backgrndpipe (tree t) {
	if (t != NULL) {
		t->backgrnd = 1;
		if (t->pipe != NULL)
			t->pipe  = backgrndpipe(t->pipe);
	}
	return t;
}

tree buildtree(list l) {
	tree tr = NULL;

	tr = level1(&l);

	return tr;
}

tree level1 (list *l) {
	// &, ;
	tree tr = level2 (l);

	if ( *l == NULL )
		return tr;
		
	if (!strcmp((*l)->s, "&")) {
		tr->backgrnd = 1;
		*l = (*l)->next;
		if (tr->pipe != NULL && tr->backgrnd == 1)
				tr = backgrndpipe(tr);
		if (*l != NULL && strcmp((*l)->s, ")")) {
			tr->type_of_next = NXT;
			tr->next = level1(l);
		}
	}
	else if (!strcmp((*l)->s, ";")) {
		*l = (*l)->next;
		if (*l != NULL && strcmp((*l)->s, ")")) {
			tr->type_of_next = NXT;
			tr->next = level1(l);
		}
	}

	return tr;
}

tree level2 (list *l) {	
	// ||, &&
	tree tr = level3(l);

	if ( *l == NULL )
		return tr;

	if (strcmp((*l)->s, "||") == 0) {
		tr->type_of_next = OR;
		*l = (*l)->next;
		tr->next = level1(l);
	}
	else if (strcmp((*l)->s, "&&") == 0) {
		tr->type_of_next = AND;
		*l = (*l)->next;
		tr->next = level1(l);
	}
	return tr;
}

tree level3 (list *l) {
	// |

	tree tr = level4(l);

	if (*l == NULL)
		return tr;

	if (!strcmp((*l)->s, "|")) {
		*l = (*l)->next; 
		tr->pipe = level3(l);
	}

	return tr;
}

tree level4 (list *l) {
	// >, >>, <
	tree tr = level5(l);

	if ( *l == NULL )
		return tr;
	while (*l != NULL && 
		(strcmp((*l)->s,">") == 0 || strcmp((*l)->s,">>") == 0 ||strcmp((*l)->s,"<") == 0)) {
		if (strcmp((*l)->s,">") == 0 || strcmp((*l)->s,">>") == 0) {
			if (strcmp((*l)->s,">>") == 0) tr->append = 1;
			*l = (*l)->next;
			tr->outfile = (char*) malloc( (strlen( (*l)->s ) + 1) * sizeof(char) );
			strcpy(tr->outfile, (*l)->s);
			*l = (*l)->next;
		}
		else if (strcmp((*l)->s,"<") == 0) {
			*l = (*l)->next;
			tr->infile = (char*) malloc( (strlen( (*l)->s ) + 1) * sizeof(char) );
			strcpy(tr->infile, (*l)->s );	
			*l = (*l)->next;
		}
		//fprintf(stderr, "%s\n",(*l)->s );
	}
	return tr;
}

tree level5 (list *l) {
	// subcmd + argv

	tree tr = NULL;
	int count = 1, i = 0;

	if (*l == NULL)
		return tr;

	// инициализация 
	tr = (tree) malloc(sizeof(struct cmd_node));	
	tr->argv = (char **) malloc(count * sizeof(char*));
	tr->argv[0] = NULL;
	tr->infile = NULL;
	tr->outfile = NULL;
	tr->subcmd = NULL;
	tr->pipe = NULL;
	tr->next = NULL;
	tr->backgrnd = 0;
	tr->append = 0;
	tr->type_of_next = ZERO;

	if (strcmp( (*l)->s, "(") == 0) {
		*l = (*l)->next; // прочитать открывающую скобку
		tr->subcmd = level1(l);
		*l = (*l)->next; // прочитать закрывающую скобку
	}
	else {
		while (*l != NULL) { // && !specset(((*l)->s)[0] ) ) {
			if (specset( ((*l)->s)[0]) && strlen((*l)->s) <= 2)  break;
			count++;
			tr->argv = (char **) realloc(tr->argv, count * sizeof(char*));
			tr->argv[i] = (char *) malloc( (strlen( (*l)->s ) + 1) * sizeof(char) );
			strcpy(tr->argv[i], (*l)->s );
			tr->argv[++i] = NULL;
			*l = (*l)->next;
		}
	}
	//fprintf(stderr, "last  %s\n", (*l)->s);
	return tr;
}

void make_shift(int n){
    while(n--)
        putc(' ', stderr);
}

void print_argv(char **p, int shift){
    char **q=p;
    if(p!=NULL){
        while(*p!=NULL){
             make_shift(shift);
             fprintf(stderr, "argv[%d] = %s\n",(int) (p-q), *p);
             p++;
        }
    }
}

void print_tree(tree t, int shift){
    char **p;
    if(t==NULL)
        return;
    p=t->argv;
    if(p!=NULL)
        print_argv(p, shift);
    else{
        make_shift(shift);
        fprintf(stderr, "psubshell \n");
    }
    make_shift(shift);
    if(t->infile==NULL)
        fprintf(stderr, "infile = NULL\n");
    else
        fprintf(stderr, "infile = %s\n", t->infile);
    make_shift(shift);
    if(t->outfile==NULL)
        fprintf(stderr, "outfile = NULL\n");
    else
        fprintf(stderr, "outfile = %s\n", t->outfile);
    make_shift(shift);
   	fprintf(stderr, "append = %d\n", t->append);
    make_shift(shift);
    fprintf(stderr, "background = %d\n", t->backgrnd);
    make_shift(shift);
    fprintf(stderr, "type = %s\n", t->type_of_next==NXT?"NXT": t->type_of_next==OR?"OR": t->type_of_next == ZERO?"ZERO": "AND" );
    make_shift(shift);
    if(t->subcmd==NULL)
        fprintf(stderr, "psubcmd = NULL \n");
    else{
        fprintf(stderr, "psubcmd--->  \n");
        print_tree(t->subcmd, shift+5);
    }
    make_shift(shift);
    if(t->pipe==NULL)
        fprintf(stderr, "pipe = NULL \n");
    else{
        fprintf(stderr, "pipe---> \n");
        print_tree(t->pipe, shift+5);
    }
    make_shift(shift);
    if(t->next==NULL)
        fprintf(stderr, "next = NULL \n");
    else{
        fprintf(stderr, "next---> \n");
        print_tree(t->next, shift+5);
    }
}
   
void deletetree (tree *t) {
	char* del;
	int i = 0;
    if (*t) {
        deletetree(&((*t)->subcmd));
        deletetree(&((*t)->pipe));
        deletetree(&((*t)->next));
        if ((*t)->argv) {
        	while ((*t)->argv[i]) {
        		del = (*t)->argv[i++];
        		free(del);
        	}
        	//free((*t)->argv[i]);
        }
        free((*t)->argv);
        if ((*t)->infile) {
        	free((*t)->infile);
        }
        if ((*t)->outfile) 
        	free((*t)->outfile);
        free(*t);
        *t = NULL;
	}
}
