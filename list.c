#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h> 
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include "tree.h"

extern char c;
extern short ERR;

int specset (int c) {
	return 	c == '|' ||
			c == '&' ||
			c == ';' ||
			c == '>' ||
			c == '<' ||
			c == '(' ||
			c == ')' ||
			c == '$';
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
	if (i == 1) { // ошибка лексическая: неправильный символ
		if (!ERR) printf ("Error! Wrong expression!\n");
		ERR = 1;
	}
	if (i == 2) // ошибка синтаксическая: неправильная конструкция
		printf("Error! Invalid syntax!\n");
}

char screening (char c) {
	switch (c) {
		case 'a':
			return '\a';
			break;
		case 'n':
			return '\n';
			break;
		case 'r':
			return '\r';
			break;
		case 't':
			return '\t';
			break;
		case 'b':
			return '\b';
			break;
		case 'v':
			return '\v';
			break;
		default:
			return c;
	}
}

char *getword() {
    int len = 1, cur = 0;
    char *s;
    char *st = NULL;
   	char *buf = NULL;
    int  count, temp;
    char *var;
    int pathflag = 0, filenameflag1 = 0, filenameflag2 = 0, wordflag = 0, subshellflag = 0;
    

    while((c = getchar()) != EOF && (c != '\n') && !ERR) {
    	
    	if (st == NULL && isspace(c)) continue;

    	st = s = (char *) realloc(st, (len++) * sizeof(char));	// длина 
	    s += cur++;		// позиция


        switch (c) {
        	
        	case '$':
        		count = 0;
        		while (isalnum(c = getchar())) {
        			buf = (char *) realloc(buf, (count + 1) * sizeof(char));
	   				buf[count++] = c;
        		}

        		ungetc(c, stdin);
        		buf = (char *) realloc(buf, (count) * sizeof(char));
	   			buf[count] = '\0';

        		if ((var = getenv(buf))) {
        			st = s = (char *) realloc(st, (len + strlen(var)) * sizeof(char));
        			s += cur;
        			len += strlen(var);
        			int i = 0;
        			while (var[i] != '\0') {
        				*s = var[i++];
        				s++;
        			}
        		}

        		else if (!strcmp("EUID", buf)) {
        			var = NULL;
        			if ((temp = geteuid())) {
        				var = (char*) malloc(12 * sizeof(char));
	        			sprintf(var,"%d",temp);
	        			st = s= (char*) realloc(st, (len + strlen(var)) * sizeof(char));
	        			s += cur;
        				len += strlen(var);
	        			int i = 0;
        				while (var[i] != '\0') {
	        				*s = var[i++];
	        				s++;
        				}
        				free(var);	
        			}
        		}
        		*s = '\0';
       			free(buf);
        		break;

        	case '(':

        		if (filenameflag1 || filenameflag2) 
        			*s = c;

        		else if (wordflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		}

				else if (pathflag) {
        			error(1);
        			return NULL;
        		}
        		
        		else {
					*s = c;
	     			st = s = ( char* ) realloc (st, (len) * sizeof (char) ); // 2
	     			*(s + cur) = '\0';
	     			return st;
	     		}

	     		break;

        	case '\"': 

        		if (filenameflag2 == 0) {

        			filenameflag1++;   
        			cur--;
        			len--;
        			
	        		if (filenameflag1 % 2 == 0) {
	        				*s = '\0';
	        				return st;
        			}
        			else if (wordflag && !pathflag) {
        				ungetc(c, stdin);
        				*s = '\0';
        				return st;
        			}
        		}
        		else *s = c;   
   				break;

        	case '\'':
        		if (filenameflag1 == 0) {

        			filenameflag2++;   
        			len--;
        			cur--;
        			if (filenameflag2 % 2 == 0) {
        				*s = '\0';
        				return st;
        			}
        			else if (wordflag && !pathflag) {
        				ungetc(c, stdin);
        				*s = '\0';
        				return st;
        			}
        		}
        		else *s = c; 
        		break;

	       	case '\\':
	       		
	       		if (filenameflag2 || filenameflag1) 
	       			*s  = c;

	        	else if ((c = getchar()) != EOF && c != '\n') {
	        		if (filenameflag2 || filenameflag1) 
	        			*s = c;
	        		else 
	        			*s = screening(c);
	        	}
	        	else {
	        		error(1);
	        		return NULL;
	        	}
	        	break;

        	case '-':
        		if (filenameflag1 || filenameflag2) {
        			*s = c;
        			break;
        		}

        		if (pathflag) {
        			error(1);
        			return NULL;
        		}

        		else if (wordflag) {
        			*s = c; 
        			break;       		
        		}

        		*s = c;
        		if ((c = getchar()) != -1 && !isspace(c) && c != '\n' && !specset(c)) {
        			st = s = (char *) realloc(st, (++len) * sizeof(char));
        			*(s + cur) = c;
        			*(s + cur + 1) = '\0';
        			return st;
	        	}
	        	else if (c == '-') *s = c;
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
	        	if (filenameflag1 || filenameflag2) 
        			*s = c;

        		else if (wordflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		}

	       		else if (pathflag) {
        			error(1);
        			return NULL;
        		}
        		
        		else {
	     			subshellflag--;
					*s = c;
	     			st = s = (char *) realloc(st, (len) * sizeof(char));
	     			*(s + cur) ='\0';
	     			return st;
	     		}
				break;

	        case '>':
	        	
	        	if (filenameflag1 || filenameflag2) 
        			*s = c;

	        	else if (wordflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		}

        		else if (pathflag ) {
        			error(1);
        			return NULL;
        		}

        		else {
        		   	*s = c;
		           	st = s = (char *) realloc(st, (len) * sizeof(char));
			       	*(s + cur) = '\0';
				}

		        return st;
		     
		        break;

	        case '<':

	        	if (filenameflag1|| filenameflag2) 
	        		*s = c;

	        	else if (wordflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		} 

        		else if (pathflag ) {
        			error(1);
        			return NULL;
        		}

	        	else {
		        	*s = c;
		        	st = s = (char *) realloc(st, (len) * sizeof(char));
		        	*(++s) = '\0';
		        	// + cur) ='\0';
		        	return st;
		        }
		        break;

	        case ';':
	        	
	        	if (filenameflag1 || filenameflag2) 
        			*s = c;

        		else if (wordflag || pathflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		}
        		else {
		        	*s = c;
		        	st = s = (char *) realloc(st, (len) * sizeof(char));
		        	*(s + cur) ='\0';
		        	return st;
				}
				break;

	        case '&':
	        	
	        	if (filenameflag1 || filenameflag2)
	        		*s = c;

	        	else if (wordflag || pathflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		} 
	        	else {
		        	*s = c;
		        	st = s = (char *) realloc(st, (len) * sizeof(char));
		        	*(s + cur) ='\0';
		        	return st;
		        }
		        break;	

	        case '|':
	        	
	        	if (filenameflag1 || filenameflag2) 
        			*s = c;

        		else if (wordflag || pathflag) {
        			ungetc(c,stdin);
        			*s = '\0';
        			return st;
        		}
        		else {
		        	*s = c;
		        	st = s = (char *) realloc (st, (len) * sizeof(char));
		        	*(s + cur) = '\0';

		        	return st;
		        }
		        break;

	        case '#':
	        	if (!wordflag) 
		        	while ( c != -1 && c != ' ' && c != '\n')
		        		c = getchar();
	        	else *s = c;
	        	break;

	        default:
	        	wordflag = 1;
	        	if ((filenameflag1 % 2) || (filenameflag2 % 2)) {
	        		 *s = c;
	        	}
	        	else if (c == '.') *s = c;
	        	else if (!isspace(c) && isalnum(c)) *s = c;
	        	else if (!isspace(c)) {
	        		free(st);
	        		error(1);
	        		return NULL;
	        	}
	        	else {
		        	*s = '\0';
		        	return st;
	        	}
	    }
	    
    }

   if (filenameflag1 % 2 || filenameflag2 % 2) {
   		free(st);
    	error(1);
    	return NULL;
    }
    if (ERR) {
		free(st);
		return NULL;
	}
	if (wordflag) {
		st = s = (char *) realloc(st, (len) * sizeof(char));
		*(s + cur) = '\0';
	}

    return st;
}

list create() {
    list head = NULL;
    list curr = NULL;
    char *str = NULL;
    while ((str = getword()) != NULL) {
        if (head == NULL) {
            curr = head = (list) malloc (sizeof(elem));
            curr->s = str;
        }
        else {
            curr = curr->next = (list) malloc (sizeof(elem));
            curr->s = str;	
        }  
       
        if (c == '\n' || c == EOF) 
        	break;  
           
    }
    if (head != NULL)
        curr->next = NULL;

    return head;
}

void printlist(list l) {
    while (l != NULL){
        printf("%s\n", l->s);
        l = l->next;
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
    free(*l);
}

int check (list ls) {
	list p = ls;
	list del;
	int subshellflag = 0, subshellfirst = 1, andorflag = 0, pipeflag = 0, outflag = 0;
	int  bcgrndflag = 0, nextcomflag = 0, flag = 0, outsecond = 0, insecond = 0;
	
	while (p != NULL) {
		
		if (subshellfirst) {
			if (notallowed(*(p->s))) { 	//начинается со спец символа
				error(2);
				return 1;
			}
			subshellfirst = 0;
		}

		if (specset(*(p->s)) && strlen(p->s) > 1) {
			outflag = 0;
			bcgrndflag = 0;
			andorflag = 0;
			pipeflag = 0;
			nextcomflag = 0;
			flag = 1;
		}

		else {
			switch ((p->s)[0]) {
				case '&':
					if (outflag || andorflag || bcgrndflag || nextcomflag || pipeflag) {
						error(2);
						return 1;
					}
					if ((p->next) != NULL) {
						
						if (*(p->next->s) == '&') {
							p->s = (char*) realloc (p->s,(strlen(p->s)+2)*sizeof(char));
							p->s = strcat(p->s,"&");
							del = p->next;
							p->next = p->next->next;	
							free(del->s);	
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
							p->s = (char*) realloc (p->s, (strlen(p->s)+2)*sizeof(char));
							p->s = strcat(p->s,"|");
							del = p->next;
							p->next = p->next->next;
							free(del->s);	
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
					flag = 0;
					outflag = 1;
					if (insecond == 2) {
						error(2);
						return 1;
					}
					else 
						insecond++;
					break;
				case '>':
					
					if (outflag || andorflag || bcgrndflag || nextcomflag || pipeflag) {
						error(2);
						return -1;
					}
					outflag = 1;
					
					if (p->next != NULL) {
						if (*(p->next->s) == '>') { 
							p->s = (char*) realloc (p->s, (strlen(p->s)+2)*sizeof(char));
							p->s = strcat(p->s,">");
							del = p->next;
							p->next = p->next->next;
							free(del->s);	
							free(del);
						}
					}
					else {
						error(2);
						return 1;
					}
					if (outsecond == 2) {
						error(2);
						return 1;
					}
					else 
						outsecond++;
					flag = 0;	
					break;
				case '(':
					if (outflag) {
						error(2);
						return 1;
					}
					if (flag == 1) {
						error(2);
						return 1;
					}
					flag = 0;
					subshellfirst = 1;
					subshellflag += 1;
				
					break;
				case ')':
					if (outflag || andorflag || pipeflag) {
						error(2);
						return 1;
					}
					if (bcgrndflag) 
						bcgrndflag = 0;

					if (nextcomflag)
						nextcomflag = 0;

					subshellflag -= 1;
					flag = 0;
					break;
				default:

					if (!strcmp(p->s,"")) 
					//if (p->s == NULL)
						if (!flag || subshellflag != 0) {
							error(2);
							return 1;
						}

					if (outflag) {
						outflag = 0;
						if (outsecond) outsecond++;
						if (insecond) insecond++;
					}

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
		}
		p = p->next;
	} 

	if (subshellflag || outflag || andorflag || pipeflag) {
		error(2);
		return 1;
	}

	return 0;
}
