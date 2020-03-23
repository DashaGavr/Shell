#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h> 
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>

int ERR = 0;
enum type_of_next_command {
	NXT, AND, OR
};
typedef struct cmd_node* tree; 
typedef struct cmd_node {
	char** 	argv;
	char* 	infile;
	char*	outfile;
	int		backgrnd;
	struct cmd_node* subcmd;
	struct cmd_node* pipe;
	struct cmd_node* next; 
	enum type_of_next_command type_of_next;
} cmd_node;


typedef struct elem *list;
typedef struct elem {
    char *s;
    list next;
} elem;


tree level1 (list l) ;
tree level2 (list l, tree tr);
tree level3 (list l, tree tr);
tree level4 (list l, tree tr);
tree level5 (list l, tree tr);

void make_shift(int n){
    while(n--)
        putc(' ', stderr);
}

void print_argv(char **p, int shift){
    char **q=p;
    if(p!=NULL){
        while(*p!=NULL){
             make_shift(shift);
             fprintf(stderr, "argv[%d]=%s\n",(int) (p-q), *p);
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
        fprintf(stderr, "psubshell\n");
    }
    make_shift(shift);
    if(t->infile==NULL)
        fprintf(stderr, "infile=NULL\n");
    else
        fprintf(stderr, "infile=%s\n", t->infile);
    make_shift(shift);
    if(t->outfile==NULL)
        fprintf(stderr, "outfile=NULL\n");
    else
        fprintf(stderr, "outfile=%s\n", t->outfile);
    make_shift(shift);
   // fprintf(stderr, "append=%d\n", t->append);
    //make_shift(shift);
    fprintf(stderr, "background=%d\n", t->backgrnd);
    make_shift(shift);
    fprintf(stderr, "type=%s\n", t->type_of_next==NXT?"NXT": t->type_of_next==OR?"OR": "AND" );
    make_shift(shift);
    if(t->subcmd==NULL)
        fprintf(stderr, "psubcmd=NULL \n");
    else{
        fprintf(stderr, "psubcmd---> \n");
        print_tree(t->subcmd, shift+5);
    }
    make_shift(shift);
    if(t->pipe==NULL)
        fprintf(stderr, "pipe=NULL \n");
    else{
        fprintf(stderr, "pipe---> \n");
        print_tree(t->pipe, shift+5);
    }
    make_shift(shift);
    if(t->next==NULL)
        fprintf(stderr, "next=NULL \n");
    else{
        fprintf(stderr, "next---> \n");
        print_tree(t->next, shift+5);
    }
}
                                  
/*void printLKR(tree t) {
    if (t != NULL) {
       	//printLKR(t->subcmd);
        if  ((t->argv) != NULL) {
        	while(*(t->argv) != NULL) {
        		printf("%s\n ", *(t->argv)); 
        		(t->argv)++; 
        	}
        }
        if (t->infile != NULL) printf("%s\n", t->infile);
        if (t->outfile != NULL) printf("%s\n", t->outfile);
        printf("%d\n",t->backgrnd);
        if (t->type_of_next == NXT) printf("NXT\n");
        else if (t->type_of_next == OR) printf("OR\n");
        else printf("AND\n");
        //printLKR(t->pipe);
        //printLKR(t->next);
    }
    return;
}*/

int specset (int c) {
	return 	c == '|' ||
			c == '&' ||
			c == ';' ||
			c == '>' ||
			c == '<' ||
			c == '(' ||
			c == ')' ;
}	

int notallowed (int c) {
	return 	c == '|' ||
			c == '&' ||
			c == ';' ||
			c == '>' ||
			c == '<' ||
			c == ';' ;
}

int symset (int c) {
	return 	c == '\'' ||
			c == '\"' ;
}

void error(int i) {
	ERR = 1;
	if (i == 1) // ошибка лексическая: неправильный символ
		printf ("Error! Wrong expression!\n");
	if (i == 2) // ошибка синтаксическая: неправильная конструкция
		printf("Error! Invalid syntax!\n");
}

char *getword() {
    int len = 0;
    char *s = NULL;
    char *st = NULL;
    char c;
   	char *buf = NULL;
    int i = 0, count = 0, temp;
    char *var;

    int pathflag = 0, filenameflag1 = 0, filenameflag2 = 0, wordflag = 0, subshellflag = 0;
    
    while((c = getchar()) != EOF) {
    	
    	if (st == NULL && isspace(c)) continue;

	    st = s = (char *) realloc(st, (len + 1) * sizeof(char));
	    s += len++;

        switch (c) {
        	case '$':
        		while (isalnum(c = getchar())) {
        			buf = (char *) realloc(buf, (count++ + 1) * sizeof(char));
	   				buf[i++] = c;
        		}
        		ungetc(c, stdin);
        		if ((var = getenv(buf))) {
        			st = (char *) realloc(st, (len+1+strlen(var)) * sizeof(char));
        			len += strlen(var)+1;
        			strcat(st,var);
        			free(buf);
        			//return st;
        		} 
        		else if (!strcmp("EUID", buf)) {

        			if ((temp = geteuid())) {
        				var = (char*) realloc(var,temp*sizeof(char));
	        			sprintf(var,"%d",temp);
	        			st = (char*) realloc(st, (strlen(var)+1+len)*sizeof(char));
	        			st = strcat(st,var);
	        			free(buf);
        				//return st;
        			}
        		}
        		else {
        			free(buf);
        		}
        		break;

        	case '(':
        		if (wordflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		}
				if (filenameflag1 || pathflag || filenameflag2) {
        			error(1);
        			return NULL;
        		}
				*s = c;
     			st = s = (char *) realloc(st, (len + 1) * sizeof(char));
     			*(s + len) ='\0';
     			return st;

        	case '\"': 

        		if (filenameflag2 == 0) {

        			filenameflag1++;   
        			len--;
        			if (filenameflag1 % 2 == 0) {
        				*s = '\0';
        				return st;
        			}
        			else if (wordflag && !pathflag ) {
        				error(1);
        				return NULL;
        			}
        		}
        		else *s = c;   
   				break;

        	case '\'':
        		if (filenameflag1 == 0) {

        			filenameflag2++;   
        			len--;
        			if (filenameflag2 % 2 == 0) {
        				*s = '\0';
        				return st;
        			}
        			else if (wordflag && !pathflag) {
        				error(1);
        				return NULL;
        			}
        		}
        		else *s = c; 
        		break;

	       	case '\\':
	        	if ((c = getchar()) != EOF) 
	        		*s = c;
	        	break;

        	case '-':
        		if (filenameflag1 || pathflag || filenameflag2) {
        			error(1);
        			return NULL;
        		}
        		*s = c;
        		if ((c = getchar()) != -1 && !isspace(c)) {
        			st = s = (char *) realloc(st, (++len + 1) * sizeof(char));
        			*(s + len-1) = c;
        			*(s + len) = '\0';
        			return st;
	        	}
	        	else {
	        		error(1);
	        		return NULL;
	        	}
	        	break;

	        case '/':
	        	pathflag = 1;
	        	*s = c;
				break;

	        case ')':
	        	if (wordflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		}
	       		if (filenameflag1 || pathflag || filenameflag2) {
        			error(1);
        			return NULL;
        		}
        		subshellflag--;
				*s = c;
     			st = s = (char *) realloc(st, (len + 1) * sizeof(char));
     			*(s + len) ='\0';
     			return st;

	        case '>':
	        	if (wordflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		}
	        	if (filenameflag1 || pathflag || filenameflag2) {
        			error(1);
        			return NULL;
        		}
	        	*s = c;
	        	st = s = (char *) realloc(st, (len + 1) * sizeof(char));
	        	*(s + len) ='\0';
	        	return st;

	        case '<':
	        	if (wordflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		}
	        	if (filenameflag1 || pathflag|| filenameflag2) {
        			error(1);
        			return NULL;
        		}
	        	*s = c;
	        	st = s = (char *) realloc(st, (len + 1) * sizeof(char));
	        	*(s + len) ='\0';
	        	return st;

	        case ';':
	        	if (wordflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		}
	        	if (filenameflag1 || pathflag || filenameflag2) {
        			error(1);
        			return NULL;
        		}
	        	*s = c;
	        	st = s = (char *) realloc(st, (len + 1) * sizeof(char));
	        	*(s + len) ='\0';
	        	return st;

	        case '&':
	        	if (wordflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		}
	        	if (filenameflag1 || pathflag || filenameflag2) {
        			error(1);
        			return NULL;
        		}
	        	*s = c;
	        	st = s = (char *) realloc(st, (len + 1) * sizeof(char));
	        	*(s + len) ='\0';
	        	return st;

	        case '|':
	        	if (wordflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		}
	        	if (filenameflag1 || pathflag || filenameflag2) {
        			error(1);
        			return NULL;
        		}
	        	*s = c;
	        	st = s = (char *) realloc(st, (len + 1) * sizeof(char));
	        	*(s + len) ='\0';
	        	return st;

	        case '#':
	        	printf("%d\n", wordflag);
	        	if (!wordflag) 
		        	while ( c != -1 && c != ' ')
		        		c = getchar();
	        	else *s = c;
	        	break;

	        default:
	        	wordflag = 1;
	        	if ((filenameflag1 % 2) || (filenameflag2 % 2)) {
	        		*s = c;
	        	}
	        	else if (!isspace(c) && isalnum(c)) *s = c;
	        	else if (specset(c) || symset(c)) {
	      
	        		ungetc(c,stdin);
	        	}
	        	else if (!isspace(c)) {
	        		error(1);
	        		return NULL;
	        	}
	        	else {
		        	*s = '\0';
		        	return st;
	        	}
	    }
    }
    if (filenameflag1 % 2  || filenameflag2 % 2) {
    	error(1);
    	return NULL;
    }
	if (c != EOF ) *s = '\0';
    return st;
}

list create() {
    list head;
    list curr;
    char *str = NULL;

    head = NULL;

    while ((str = getword()) != NULL) {
        if (head == NULL) {
            curr = head = (list) malloc(sizeof(elem));
            curr->s = str;
        }
        else {
            curr = curr->next = (list) malloc(sizeof(elem));
            curr->s = str;
        }
    }
    if (head != NULL)
        curr->next = NULL;
    return head;
}

void printlist(list p) {
    while (p != NULL){
        printf("%s\n", p->s);
        p = p->next;
    }
}

void deletelist(list *l) {
    list del;
    while (*l) {
        del = *l;
        *l = (*l)->next;
        free(del->s);
        free(del);
    }
}

int check (list ls) {
	list p = ls;
	list del;
	int subshellflag = 0, subshellfirst = 1, andorflag = 0, pipeflag = 0, outflag = 0;
	int  bcgrndflag = 0, nextcomflag = 0, flag = 0;
	
	while (p != NULL) {

		if (subshellfirst) {
			if (notallowed(*(p->s))) { 	//начинается со спец символа
				error(2);
				return 1;
			}
			subshellfirst = 0;
		}

		switch (*(p->s)) {
			case '&':
				if (outflag || andorflag || bcgrndflag || nextcomflag || pipeflag) {
					error(2);
					return 1;
				}
				if ((p->next) != NULL) {
					
					if (*(p->next->s) == '&') {
						p->s = strcat(p->s, p->next->s);
						del = p->next;
					if (p->next->next != NULL) 
							p->next = p->next->next;
						else p->next = NULL;
						free(del);
						andorflag = 1;
					}
					else bcgrndflag = 1;
				}
				else bcgrndflag = 1;
				flag = 0;
				break;
			case '|':
				if (outflag || andorflag || bcgrndflag || nextcomflag || pipeflag) {
					error(2);
					return 1;
				}
				if ((p->next) != NULL) {

					if (*(p->next->s) == '|') {
						p->s = strcat(p->s, p->next->s);
						del = p->next;
						p->next = p->next->next;
						free(del);
						andorflag = 1;
					}
					else pipeflag = 1;
				}
				else pipeflag = 1;
				flag = 0;
				break;
			case ';':
				if (outflag || andorflag || bcgrndflag || nextcomflag || pipeflag) {
					error(2);
					return 1;
				}
				
				nextcomflag = 1;
				flag = 0;
				break;
			case '<':
				if (outflag || andorflag || bcgrndflag || nextcomflag || pipeflag) {
					error(2);
					return 1;
				}
				outflag = 1;
				
				break;
			case '>':
				if (outflag || andorflag || bcgrndflag || nextcomflag || pipeflag) {
					error(2);
					return -1;
				}
				outflag = 1;
				if (p->next != NULL) {
					if (*(p->next->s) == '>') { 
						p->s = strcat(p->s, p->next->s);
						del = p->next;
						p->next = p->next->next;
						free(del);
					}
				}
				else {
					error(2);
					return 1;
				}
				break;
			case '(':
				if (andorflag || outflag || flag ) {
					error(2);
					return 1;
				}
				subshellfirst = 1;

				subshellflag += 1;

				break;
			case ')':
				if (outflag || andorflag || pipeflag) {
					error(2);
					return -1;
				}
				if (bcgrndflag) 
					bcgrndflag = 0;

				if (nextcomflag)
					nextcomflag = 0;

				subshellflag -= 1;
				flag = 0;
				break;
			case '-':	//если флаг для команды, то ничего не делаем, переходим на следующйи шаг
				flag = 1;
				break;
			default:
				if (!strcmp(p->s,"")) {
					error(2);
					return 1;
				}
				
				if (outflag) 
					outflag = 0;

				if (bcgrndflag)
					bcgrndflag = 0;

				if (andorflag)
					andorflag = 0;

				if (nextcomflag)
					nextcomflag = 0;

				if (pipeflag)
					pipeflag = 0;
				flag = 1;
		}	
		p = p->next;
	} 

	if (subshellflag || outflag || andorflag || pipeflag) {
		error(2);
		return 1;
	}

	return 0;
}

tree buildtree(list l){
	tree tr = NULL;
	list p = l;
	//tr = (tree) realloc (tr, sizeof(tree));
	tr = level1(p);
	//if (tr) printf("efwfef\n");
	return tr;
} 

tree level1 (list l) {
	tree tr = NULL;
	tr = (tree) realloc(tr, sizeof(tree));
	tr = level2 (l, tr);
	if (l != NULL) {
		
		if (*(l->s) == '&')
			tr->backgrnd = 1;
		else if (*(l->s) == ';') {
			tr->next = (tree) realloc (tr->next, sizeof(tree));
			tr->next = level1(l->next);
		}
		else {
			tr->backgrnd = 0;
			tr->next = NULL;
		}
	}
	return tr;
}

tree level2 (list l, tree tr) {
	tr = level3(l, tr);
	if (l != NULL) {
		
		if (!strcmp(l->s,"||"))
			tr->type_of_next = OR;
		else if (!strcmp(l->s,"&&"))
			tr->type_of_next = AND;
		else 
			tr->type_of_next = NXT;
	}

	return tr;
}

tree level3 (list l, tree tr) {
	tr = level4(l, tr);
	while(l != NULL && (*(l->s) != '(' || !strcmp(l->s,"|") )){
		l = l->next;
	}
	if (l != NULL) {
		
		if (!strcmp(l->s,"(")) {
			tr->subcmd = (tree) realloc(tr->subcmd, sizeof(tree));
			tr->subcmd = level1(l->next);
		} 
		else if (*(l->s) == '|') {
			tr->pipe = (tree) realloc (tr->pipe, sizeof(tree));
			tr->pipe = level1(l->next);
		}
		else {
			tr->subcmd = NULL;
			tr->pipe = NULL;
		}
	}
	return tr;
}

tree level4 (list l, tree tr) {
	tr = level5(l, tr);
	while(l != NULL && !specset(*(l->s))){
		l=l->next;
	}
	if (l != NULL) {
		if (*(l->s) == '>' || !strcmp(l->s,">>")) {
			tr->outfile = (char*) realloc(tr->outfile, (strlen(l->next->s)+1)*sizeof(char));
			strcpy(tr->outfile,l->next->s);
			l = l->next;
			tr->infile = NULL;
		}
		else if (*(l->s) == '<') {
			tr->infile = (char*) realloc(tr->infile, (strlen(l->next->s)+1)*sizeof(char));
			strcpy(tr->infile,l->next->s);
			l = l->next;
			tr->outfile = NULL;
		}	
	}

	return tr;
}

tree level5 (list l, tree tr) {
	//tree tr = NULL;
	int count = 0, len = 1;
	//tr = (tree) realloc(tr, sizeof(tree));
	if (l != NULL) {
		/*if (!strcmp(l->s,"(")) {
			tr->subcmd = (tree) realloc(tr->subcmd, sizeof(tree));
			tr->subcmd = level5(l->next);
		} 
		else tr->subcmd = NULL;
		while (l != NULL && *(l->s) != ')')
			l=l->next;
		l = l->next;*/
		printf("level 5 str %s\n", l->s);
		while (l != NULL && !specset (*(l->s)) ) {
			
			tr->argv = (char**) realloc(tr->argv, (count+1)*sizeof(char*));
			(tr->argv)[count] = (char*) realloc((tr->argv)[count], (strlen(l->s)+1)*sizeof(char));
			strcpy(tr->argv[count++],l->s);
			l = l->next;
			printf(" tree level5 = %s\n", (tr->argv)[count-1]);
		}		
		
		//tr = level4(l, tr);
	}
	/*else {
	 	free(tr);
	 	return NULL;
	}*/
	return tr;
}

int main (void) {
	list ls = NULL;
	tree tr = NULL;
	//while(1) {
		deletelist(&ls);
		//deletetree(&tr);
		printf("Enter the command:\n$: ");
		ls = create();
		
		printf("Check   ");
		if (check(ls) == 0) {
			printf("OK!!\n\n");
			printf("Print list: \n");
			printlist(ls);
			putchar('\n');
			
			/*if ((tr = buildtree(ls))) {
				putchar('\n');
				//printLKR(tr);
				print_tree(tr,4);
			}*/
		}
		else {
			printf("Print wrong list: \n");
			printlist(ls);
		}

	//}
}
