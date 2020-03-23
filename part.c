#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h> 
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <setjmp.h>
#include "tree.h"
#include "exec.h"

char c;
short ERR = 0;
int* backlist = NULL; 
int countback = 0;
int* pidlist = NULL;
int countpid = 0;
jmp_buf begin;

void handler(int signum) {
	int* del;
	int j;
	j = countback;
	while (--j) {
		del = backlist;
		backlist++;
		free(del);
		backlist = NULL;
	}
	j = countpid;
	while (--j) {
		del = pidlist;
		pidlist++;
		free(del);
		pidlist = NULL;
	}
	if (signum) longjmp(begin,1);
}

int main (void) {
	int j,status;
	list ls = NULL;
	tree tr = NULL;

	printf("Enter the command:");

	while(1) {
		putchar('\n');	
		setjmp(begin); 
		ERR = 0;
		if (ls) 
			if (tr) {
				deletetree(&tr);
				tr = NULL;
			}

		deletelist(&ls);

		if (feof(stdin)) break;

		tr = NULL;

		printf("$: ");

		ls = create();

		if (ERR) { // == flush ??
			while (c != '\n'  && c != EOF)
				c = getchar();
		}
		
		if (ls == NULL || ERR) {
			continue;
		}
		if (check(ls) == 0) {
		//	printlist(ls);
			if ((tr = buildtree(ls))) {
				/*if (tr)
					print_tree(tr, 5);*/
			}
			execute(tr);
			/*j = countback;
			while (--j) {
				waitpid(backlist[j],&status,0);
			}
			handler(0);*/

		}	// check
	}	// while 
}	//shell

