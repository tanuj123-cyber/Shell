#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include <sys/types.h>
#include <sys/wait.h>
//#include "list.h"
#include "fcntl.h"
#include <dirent.h>

//typedef int bool32;
#define _breakpoint() __asm__ volatile ("int $3\n")

int is_path(const char* s){
	int len = strlen(s);
	for(int i = 0; i < len; i++){
		if(s[i] == '/'){
			return 1;
		}
	}
	return 0;
}

int main(){
	char wd[256] = {0};
	size_t input = 0;
	char* buff = NULL;
	size_t buffsize = 32;
	char operators[3] = {'<','>','|'};
	char* appenders[2] = {"<<", ">>"};
	buff = (char*) malloc(buffsize * sizeof(char));
	if(buff == NULL){
		perror("Unable to allocate buffer");
		exit(1);
	}

	getcwd(wd, sizeof(wd));
	const char* rd = wd;
	int count = 0;
	for(int i = strlen(wd)-1; i > 0; i--){
		if (wd[i] == '/'){
			rd = wd + i + 1;
			break;
		}
	}
	printf("[nyush %s]$ ",rd);
	fflush(stdout);
	input = getline(&buff,&buffsize,stdin);
	buff[strcspn(buff, "\n")] = 0;
	while(!feof(stdin)){
		int i = 0;
		int operatorInd = 0;
	//	int operatorIndicator = 0;
		char theOpp = 0;
	//	char* theOper = 0;
		char* command = strtok(buff, " ");
		char* array[256] = {0};
		char commCopy[32];
		char* inputRedir;
		char* appL;
	//	_breakpoint();
		while (command != NULL){
			if (!strcmp(appenders[0], command) || !strcmp(appenders[1], command)){
				appL = command;	
				operatorInd = i;
			}
			else if (command[0] == operators[0] || command[0] == operators[1] || command[0] == operators[2]){
				theOpp = command[0];
				operatorInd = i;
			}
			else{
				array[i++] = command;
			}	
			command = strtok(NULL, " ");
		}
		array[i++] = NULL;
		char* firstArg = array[0];
		char buffer[12] = "/bin/";
		char* path = !is_path(array[0]) ? strcat(buffer,array[0]) : array[0];
		if (!strcmp(firstArg,"exit")){
		      	if (i > 2) fprintf(stderr, "Error: invalid command\n");
			else exit(0);
		}
		else if (!strcmp(firstArg,"cd")){
			if(chdir(array[1]) == -1){
			       	if(i > 2 && access(array[1],F_OK)) fprintf(stderr, "Error: invalid directory\n");
				else fprintf(stderr, "Error: invalid command\n");
			}
		}
		else{// if(!access(path, F_OK)){
			int stats;
		//	char* p[32] = {0};
			pid_t pid = fork();
			if (pid == 0){
				if (theOpp == '>'){
					int fd = open(array[operatorInd], O_CREAT | O_WRONLY | O_TRUNC, 
							S_IRUSR | S_IWUSR);
					dup2(fd,STDOUT_FILENO);
					close(fd);
					array[operatorInd] = NULL;
				}
				else if (theOpp == '<'){
					int fd = open(array[operatorInd], O_RDONLY, 
							S_IRUSR | S_IWUSR);
					dup2(fd,STDIN_FILENO);
					close(fd);
					array[operatorInd] = NULL;
				}
				else if (!strcmp(appL,">>")){
					int fd = open(array[operatorInd], O_CREAT | O_WRONLY | O_APPEND, 
							S_IRUSR | S_IWUSR);
					dup2(fd,STDOUT_FILENO);
					close(fd);
					array[operatorInd] = NULL;
				}
			//	_breakpoint();
				if (execv(path,array) == -1) fprintf(stderr, "Error: invalid program\n");//	else execv(path,array);
				exit(0);
			}
			else{
				waitpid(pid,&stats,0);
			}
		}
	//	else{
	//		fprintf(stderr, "Error: invalid command\n");
			//perror(path);
	//	}

		getcwd(wd, sizeof(wd));
		const char* rd = wd;
		for(int i = strlen(wd)-1; i > 0; i--){
			if (wd[i] == '/'){
				rd = wd + i + 1;
				break;
			}
		}
		printf("[nyush %s]$ ",rd);
		fflush(stdout);
		input = getline(&buff,&buffsize,stdin);
		buff[strcspn(buff, "\n")] = 0;
	}
	return 0;
}
