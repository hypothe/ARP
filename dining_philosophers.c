#include <stdlib.h>
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <sys/time.h>
#include <unistd.h> 

#define PH_NUM 5

int spawn(char *ex_name, int id);
/* 	Function to generate a child, a philosopher inside a new shell window thanks to "konsole"
	Arguments:
		(int) id: the id of the philospher, in order to determine the FIFOs it should write to/ read from
	Return:
		(int) child_pid: the PID of the generated new shell window or a negative value in case of error during the fork()

 */
int clean_prev();
/*	Function to clean previously opened named pipes
	
	Forks and executes a bash executable which performs a simple rm -filenames
	Note that it's not mandatory, utilized simply to clean the environment.
	
	Return:
		(int) c_pid: PID of the cleaning process
	
*/

// ------------------------------------------------------------------------------

int main(int argc, char *argv[]){

	int c_pid;
	pid_t ph_pid[PH_NUM];
	pid_t wtr_pid;
	char *str1 = "/tmp/ph_rqst";		// suppose their name is already known by philosophers
	char *str2 = "/tmp/ph_rls";
	char fifo_rqst[PH_NUM][20];
	char fifo_rls[PH_NUM][20];
	char *str_phil = "./philosopher";
	char *str_wtr = "./waiter";
	
	c_pid=clean_prev();
	waitpid(c_pid, NULL, 0);	// note that it's necessary to wait for it to end, or we could end up cleaning those files while the waiter or philosophers try to open them
	printf("Cleaning previous FIFO\n"); fflush(stdout);

	if((wtr_pid = spawn(str_wtr, PH_NUM))<0){
		perror("Waiter generation");
		exit(1);
	}

	for(int i=0; i<PH_NUM; i++){
		strcpy(fifo_rqst[i], "");
		strcpy(fifo_rls[i], "");
		sprintf(fifo_rqst[i], "%s%d", str1, i);
		sprintf(fifo_rls[i], "%s%d", str2, i);
		if (mkfifo(fifo_rqst[i], 0666))
			perror("Cannot create request fifo");
		if (mkfifo(fifo_rls[i], 0666))
			perror("Cannot create release fifo");
		
		if((ph_pid[i] = spawn(str_phil, i))<0){
			perror("Philosphers generation"); 
			for(int j=0; j<i; j++){ waitpid(ph_pid[j], NULL, 0); }  return 1;
		}
	}

	return 0;
}

// ------------------------------------------------------------------------------

int spawn(char* ex_name, int id) {
	pid_t child_pid = fork();
	if (child_pid != 0)
	{	
		return child_pid;
	}
	else 
	{
		char tmp[5]="";
		sprintf(tmp, "%d", id);
		char * args[] = { "/usr/bin/konsole",  "-e", ex_name, tmp, (char*)NULL };
		execvp(args[0], args);
		perror("exec failed");
		return 1;
	}
}

int clean_prev(){
	char tmp[5];
	sprintf(tmp, "%d", PH_NUM);
	char *args[] = {"./clean_tmp_fifo", tmp, NULL};
	int pid = fork();
	if (pid<0){
		perror("clean child");
		exit(1);
	}
	if (pid==0){
		execvp(args[0], args);
	}
	return pid;
}