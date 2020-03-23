
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tree.h"
#include "exec.h"

int execute(tree tr);				// 1 уровень


int exec_list_cmd(tree tr);			// идет по некстам и выполняет последвательно, 
									// предварительно вызывая конвеер, если есть!

int exec_conv_cmd(tree tr); 		// идет по пайпам и осуществляет перенаправление ввода вывода, 
									// ждет именно порожденный процесс, возвращает результат наверх

int exec_cmd(tree tr); 				// выполнение 1 команды, здесь же обрабатывается сабшелл
									// = рекурсивный  вызов execute (), подмена ввода и вывода 

void handler(int signum);


extern int* backlist;
extern int* pidlist;
extern countback;
extern countpid;

int execute(tree tr) {
	if (tr == NULL) return 1;
	else return exec_list_cmd(tr);
}

int exec_list_cmd(tree tr) {
	int i = 0, result = 0, first = 1;
	tree cur = tr;

	/*if (cur->backgrnd == 1)
		signal(SIGINT,SIG_IGN);
	else 
		signal(SIGINT, handler);*/

	if (cur->argv[0] != NULL) {
		if (!strcmp(cur->argv[i], "exit")) {
			return 0;
		}

		if (strcmp(cur->argv[i],"cd") == 0) {
			if (cur->argv[i+1] != NULL) {
				chdir(cur->argv[i+1]);
				if (errno != 0) 
				perror("chdir");
			}
			else {
				chdir(getenv("HOME"));
			}
		}
	}
		while (cur != NULL) {
		if ((result == 0) && (cur->type_of_next == OR)) {
			if (first) {
				result = exec_conv_cmd(cur);
				cur = cur->next;
				first = 0;
			}
			else {
				cur = cur->next;
				continue;
			}
		}
		else if ((result == 0) && (cur->type_of_next == AND)) {
			result = exec_conv_cmd(cur);
			cur = cur->next;
		}
		else if (cur->type_of_next == NXT || cur->type_of_next == ZERO) {
			result = exec_conv_cmd(cur);
			cur = cur->next;
		}
	}
			

	if (result == 0) return 0;

	else return -1;
}

int exec_conv_cmd(tree tr) {
	tree cur = tr;
	int fd[2], in, out, next_in, i = 0, j = 0, pid;
	int  status;

	if (cur->pipe == NULL) {
		if ((pid = fork()) == 0) {
			if (exec_cmd(cur) != 0) {
				perror("command");
				_exit(-1);
			}
		}
		wait(NULL);
		return WIFEXITED(status) && !WEXITSTATUS(status);
	}
		pipe(fd);
		out = fd[1];
		next_in = fd[0];
		
		if ((pid = fork()) == 0) {
			close(next_in);
			dup2(out, 1);
			close(out);
			if (exec_cmd(cur) != 0) {
				perror("command");
				_exit(-1);
			}
		}
		/*else {
			if (cur->backgrnd == 1) {
				backlist = (int*) malloc(sizeof(int));
				backlist[countback++] = pid;
			}
			else {
				pidlist = (int*) malloc(sizeof(int));
				pidlist[countpid++] = pid;
			}
		}*/

		in = next_in;

		while (cur->pipe->pipe != NULL) {
			close(out);
			pipe(fd);
			out = fd[1];
			next_in = fd[0];
			cur = cur->pipe;
			if ((pid = fork()) == 0) {
				dup2(in, 0);
				close(in);
				close(next_in);
				dup2(out,1);
				close(out);
				if (exec_cmd(cur) != 0) {
					perror("command");
					handler(0);//????
					_exit(-1);
				}
			}
			/*else {
				if (cur->backgrnd == 1) {
					backlist = (int*) realloc(backlist, (j + 1) * sizeof(int));
					backlist[countback++] = pid;
				}
				else {
					pidlist = (int*) realloc(pidlist, (i + 1) * sizeof(int));
					pidlist[countpid++] = pid;
				}
			}*/
			close(in);
			in = next_in;
		}
			close(out);
			cur = cur->pipe;
			if (fork() == 0) {
				dup2(in, 0);
				close(in);
				if (exec_cmd(cur) != 0) {
					perror("command");
					handler(0);
					_exit(-1);
				}
			}
			else {
				if (cur->backgrnd == 1) {
					backlist = (int*) realloc(backlist, (j + 1) * sizeof(int));
					backlist[countback++] = pid;
				}
				else {
					pidlist = (int*) realloc(pidlist, (i + 1) * sizeof(int));
					pidlist[countpid++] = pid;
				}
			}

	close(in);

	/*i = countpid;
	while (--i) {
		waitpid(pidlist[i], &status, 0);

	}*/
	return WIFEXITED(status) && !WEXITSTATUS(status);
}

int exec_cmd(tree tr) {
	int in, out; //resultsubshell = 0, result = 0;
	if (tr == NULL)  return 0;

	if (tr->infile != NULL) {
		in = open(tr->infile, O_RDONLY);
		dup2(in,0);
	}

	if (tr->outfile != NULL) {
		if (tr->append) {
			if ((out = open(tr->outfile, O_WRONLY|O_TRUNC|O_CREAT|O_APPEND|0666)) == -1)
				perror("open file");
		}
		else 
			if ((out = open(tr->outfile, O_WRONLY|O_TRUNC|O_CREAT|0666)) == -1)
				perror("open file");
		dup2(out,1);
	}

	if (tr->subcmd != NULL) { 
		exec_list_cmd(tr->subcmd);
		if (errno != 0) {
			perror("subshell");
			_exit(-1);
		}
	}
	else {
		execvp(tr->argv[0], tr->argv);
		exit(1);
	}
}
