int execute(tree tr);				// 1 уровень


int exec_list_cmd(tree tr);			// идет по некстам и выполняет последвательно, 
									// предварительно вызывая конвеер, если есть!

int exec_conv_cmd(tree tr); 		// идет по пайпам и осуществляет перенаправление ввода вывода, 
									// ждет именно порожденный процесс, возвращает результат наверх

int exec_cmd(tree tr); 				// выполнение 1 команды, здесь же обрабатывается сабшелл
									// = рекурсивный  вызов execute (), подмена ввода и вывода 

void handler(int signum);			// находится в main'e