/*
#################################
#CSIS0230_3230 Lab 3 Exercise	#
#
#Name:Aakansha Parmar													#
#UID:2012622238													#
#Email:aakansha@connect.hku.hk												#
#################################
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <limits.h>


char** stringToArray(char* input){
	char* str = input;	
	char ** output = (char**) malloc(sizeof(char*) * (strlen(input)+1));
	char * pch = strtok(str, " \t\n");
	int count = 0;
	while (pch != NULL){
		output[count++] =  pch;
		pch = strtok(NULL, " \t\n");
	}
	output[count] = NULL;
	return output;
}

double diff_s(struct timespec a, struct timespec b){
	return (double) (a.tv_sec - b.tv_sec + (a.tv_nsec - b.tv_nsec) / 1000000000.0); 
}

void sigusr1_handler(int signum)
{

	printf("SIGUSR1 received, start execute command\n");
	
}

int main(int argc, char** argv){

	char str[1000];
	char *str2;
	char **temp;
	char **temp2;
	long int numberOfContextSwitches=0;

	int i;
	char* a[argc];

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
	
	for(i = 0; i<argc; i++)
	{
	  a[i]=argv[i+1];
  	}  
	
	// TODO: Create the child process.
	
	cpid=fork();

	//Record start time
	struct timespec start;
	struct timespec end;
	clock_gettime(CLOCK_REALTIME, &start);


	//Parent process sends SIGUSR1 to child process
	if(cpid!=0)
	{
	   kill(cpid,SIGUSR1);
	   printf("SIGUSR1 sending to child process\n");
	}

	//Wait for child process to end
	siginfo_t sigInfo;
	waitid(P_PID,cpid,&sigInfo,WEXITED | WNOWAIT);
	//Record end time
	clock_gettime(CLOCK_REALTIME, &end);
	
	

	// TODO: Branch the program routine for parent and child
	if(cpid==0)
	{
	   signal(SIGINT, SIG_DFL);
	   if (execvp(a[0],a) == -1) {
		perror("Error: ");
		exit(1);
	}}

	

	if(cpid!=0){
	//Open stat and status files 
	FILE* file;
	FILE* file2; 
	long int pid=cpid;
	char fname[50], fname2[50]; 
	sprintf(fname, "/proc/%ld/stat",pid); 
	sprintf(fname2, "/proc/%ld/status",pid);
	file = fopen(fname, "r");
	file2 = fopen(fname2, "r");

	//If file is empty return eror
	if(file==NULL || file2==NULL)
	{ perror("Error in opening file"); exit(1); }
	
	//Read the stat file
	if(fgets(str, 1000, file)!=NULL)
	{
	  temp=stringToArray(str);
	}

	//Read the status file
	ssize_t line;
	size_t length=0;
	
	while((line=getline(&str2, &length, file2)) != -1)
	{
	  
	  if( strstr(str2, "voluntary_ctxt_switches:	")!=NULL)
	  { 
	    temp2=stringToArray(str2);
	    numberOfContextSwitches=numberOfContextSwitches+ atol(temp2[1]);
	  }
		 
	}
	
	//Close files 
	fclose(file);
	fclose(file2);
	
	}	
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

	//Displaying all information
	long int ut = atol(temp[13]);
	float utime= ut/sysconf(_SC_CLK_TCK);
	long int st = atol(temp[14]);
	float stime= st/sysconf(_SC_CLK_TCK);
	float time=diff_s(end, start);
	printf("Real: %.3fs, user: %.3fs, system: %.3fs, context switch: %ld\n", time, utime, stime, numberOfContextSwitches );

	free(str2);
	return 0;
				
  
}

