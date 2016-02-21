/*
#################################
#CSIS0230_3230 Lab 2 Exercise	#
#
#Name:Aakansha Parmar													#
#UID:2012622238													#
#Email:aakansha@connect.hku.hk												#
#################################
*/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>


void sigusr1_handler(int signum)
{

	printf("SIGUSR1 received, start execute command\n");
	
}

int main(int argc, char** argv){

	pid_t cpid;

	//Save signal names 
	char *signals[21];
	signals[1]="SIGHUP"; signals[2]="SIGINT";  signals[3]="SIGQUIT"; signals[4]="SIGILL"; signals[5]="SIGTRAP";
	signals[6]="SIGABRT"; signals[7]="SIGBUS";  signals[8]="SIGFPE"; signals[9]="SIGKILL"; signals[10]="SIGUSR1";
	signals[11]="SIGSEGV"; signals[12]="SIGUSR2";  signals[13]="SIGPIPE"; signals[14]="SIGALRM"; signals[15]="SIGTERM";
	signals[16]="SIGSTKFLT";  signals[17]="SIGCHLD";  signals[18]="SIGCONT";  signals[19]="SIGSTOP";  signals[20]="SIGTSTP";

	//Ignore the SIGINT signal
	signal(SIGUSR1, sigusr1_handler);
	signal(SIGINT, SIG_IGN);

	
	// TODO: Process the arguments
	int i;
	char* a[argc];
	for(i = 0; i<argc; i++)
	{
	  a[i]=argv[i+1];
  	}  
	
	// TODO: Create the child process.
	
	cpid=fork();

	//Parent process sends SIGUSR1 to child process
	if(cpid!=0)
	{
	   kill(cpid,SIGUSR1);
	   printf("SIGUSR1 sending to child process\n");
	}

	// TODO: Branch the program routine for parent and child
	if(cpid==0)
	{
	   signal(SIGINT, SIG_DFL);
	   if (execvp(a[0],a) == -1) {
		perror("Error: ");
		exit(1);
	}}

	int status;
	int returnValue;
	returnValue=waitpid(cpid,&status,0);
	
	
	//Check if the child process terminated normally
	if(WIFEXITED(status))
	{
	
		printf("The command %s has returned status code = %d \n",a[0],WEXITSTATUS(status));
	
	}
	
	else if(WIFSIGNALED(status))
	{
		int sig=WTERMSIG(status);
		printf("The command %s is interrupted by the signal number = %d (%s)\n",a[0],sig,signals[sig]);
	
	}
	

	return 0;			
  
}

