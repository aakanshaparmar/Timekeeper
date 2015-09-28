/*
#########################################################
#CSIS0230_3230 Timekeeper program			#
#Name:Aakansha Parmar 		 			#
#UID:2012622238						#
#Email:aakansha@connect.hku.hk				#	
#Development Platform:Built on UBUNTU 14.04 		#
#Compilation - To compile use gcc command 		#
	       and then call executable file with 	#
	       or without command line arguments	#		
#########################################################
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

//Function to split string to a string array 
char** stringToArray(char* input)
{
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

//Function to return difference in time 
double diff_s(struct timespec a, struct timespec b){
	return (double) (a.tv_sec - b.tv_sec + (a.tv_nsec - b.tv_nsec) / 1000000000.0); 
}

//Signal handler function 
void sigusr1_handler(int signum)
{
}


int main(int argc, char** argv)
{

	//Declarations
	int i;
	char str[1000];
	char *str2;
	char **temp;
	char **temp2;
	char* a[argc];
	pid_t cpid;
	int j;
	long int numberOfContextSwitches=0;
	int status;
	int returnValue;
	int countOfPipes=0;
	FILE* file;
	FILE* file2; 
	char fname[50], fname2[50];

	//Save signal names 
	char *signals[21];
	signals[1]="SIGHUP"; signals[2]="SIGINT";  signals[3]="SIGQUIT"; signals[4]="SIGILL"; signals[5]="SIGTRAP";
	signals[6]="SIGABRT"; signals[7]="SIGBUS";  signals[8]="SIGFPE"; signals[9]="SIGKILL"; signals[10]="SIGUSR1";
	signals[11]="SIGSEGV"; signals[12]="SIGUSR2";  signals[13]="SIGPIPE"; signals[14]="SIGALRM"; signals[15]="SIGTERM";
	signals[16]="SIGSTKFLT";  signals[17]="SIGCHLD";  signals[18]="SIGCONT";  signals[19]="SIGSTOP";  signals[20]="SIGTSTP";

	//Signal handlers 
	signal(SIGUSR1, sigusr1_handler);
	signal(SIGINT, SIG_IGN);
	
	
	//In case of no arguments exit
	if(argc==1)
	{exit(1);}

	//Counting number of pipes in instruction
	int numberOfPipes=0;
	for(i=0;i<argc;i++)
	{
		if(strcmp(argv[i],"!")==0)	
		{
			numberOfPipes++;
		}
	}

	//Declare pipes
	int mypipe[numberOfPipes][2];

	//process first argument 
	int check=0,count=0;
	for(i=1; i<argc && check==0; i++)
	{
		if(strcmp(argv[i],"!")==0)
		{check=1;}
		else {a[count]=argv[i];count++;}
		
	}

	a[count]=NULL;

	//Create first Pipe
	pipe(mypipe[0]);

	//Make child  process 
	cpid=fork();

	struct timespec start;
	struct timespec end;
	float real_time;

	if (cpid!=0)
	{
		
		printf("Process with id: %d created for the command: %s\n",cpid,a[0]); 
		//Start counting real time
		clock_gettime(CLOCK_REALTIME,&start);		
		//Sending signal to child process to begin execution from parent process
		kill(cpid, SIGUSR1);												
		siginfo_t signalInfo;
		waitid(P_PID, cpid, &signalInfo, WEXITED | WNOWAIT);
		//Stop counting real time
		clock_gettime(CLOCK_REALTIME,&end);									
		real_time = diff_s(end, start);
	}

	
	//Execute first instruction
	if (cpid == 0)
    	{
      		//Default action for SIGINT
		signal(SIGINT, SIG_DFL);

		//Open write end of pipe close read end if there are more instructions
		if(numberOfPipes>0)
		{close (mypipe[0][0]);
		dup2(mypipe[0][1], 1);}
		
		//Execute
		if (execvp(a[0],a) == -1) {
		perror("execvp: ");
		printf("timekeeper experienced an error in starting the command: %s\n",a[0]);
		exit(1);}

   	}
 	else if (cpid <  0)
   	{
		//The fork failed. 
		perror("Error: ");
		printf("timekeeper experienced an error in starting the command: %s\n",a[0]);
		exit(1);
	}
  	else
   	{
     		//Close open ends 
		close(mypipe[countOfPipes][1]);
		countOfPipes++;

		//Open stat and status files
		long int pid=cpid;
		sprintf(fname, "/proc/%ld/stat",pid); 
		sprintf(fname2, "/proc/%ld/status",pid);
		file = fopen(fname, "r");
		file2 = fopen(fname2, "r");

		//If file is empty return eror
		if(file==NULL || file2==NULL)
		{ perror("Error in opening file"); 
		  exit(1); }
	
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
	
		//waoit for child process
		returnValue=waitpid(cpid,&status,0);
	
		//Check if the child process terminated normally
		if(WEXITSTATUS(status)==0)
		{
			printf("The command \"%s\" has returned status code = %d \n",a[0],WEXITSTATUS(status));
	
		}
	
		//If child process was terminated by a signal
		else if(WIFSIGNALED(status))
		{
			int sig=WTERMSIG(status);
			printf("The command \"%s\" is interrupted by the signal number = %d (%s)\n",a[0],sig,signals[sig]);
	
		}

		//Displaying all information
		long int ut = atol(temp[13]);
		float utime= ut/sysconf(_SC_CLK_TCK);
		long int st = atol(temp[14]);
		float stime= st/sysconf(_SC_CLK_TCK);
		float time=diff_s(end, start);
		printf("Real: %.3fs, user: %.3fs, system: %.3fs, context switch: %ld\n", time, utime, stime, numberOfContextSwitches);

		free(str2);
		
		//Reset arguments
		for(j=0;j<count;j++)
		{ a[j]=NULL; }
		count=0;
	}

	/*------------------------------------------------------------------------------------------*/
	
	//Execute remaining instructions
	if(numberOfPipes>0)
	{for(j=i;j<argc;j++)
	{
		if(strcmp(argv[j],"!")==0)
		{
			int k;
			a[count]=NULL;

			//Create Pipe
			pipe(mypipe[countOfPipes]);

			struct timespec start1;
			struct timespec end1;

			//Make child process
			cpid=fork();

			if (cpid!=0)
			{
		
				printf("Process with id: %d created for the command: %s\n",cpid,a[0]); 
				//Start counting real time
				clock_gettime(CLOCK_REALTIME,&start1);		
				//Sending signal to child process to begin execution from parent
		               	kill(cpid,SIGUSR1); 	
				siginfo_t signalInfo;
				waitid(P_PID, cpid, &signalInfo, WEXITED | WNOWAIT);
				//Stop counting real time
				clock_gettime(CLOCK_REALTIME,&end1);
				real_time = diff_s(end, start1);
			}

			if(cpid==0)
			{
				//Default action for SIGINT
				signal(SIGINT, SIG_DFL);

				//Close write end of the pipe and open read end of last pipe
				close(mypipe[countOfPipes-1][1]);
				dup2(mypipe[countOfPipes-1][0],0);
				
				//Close read end of new pipe and open write end of new pipe
				close(mypipe[countOfPipes][0]);
				dup2(mypipe[countOfPipes][1],1);

				//Execute instruction
				if (execvp(a[0],a) == -1) {
				perror("execvp: ");
				printf("timekeeper experienced an error in starting the command: %s\n",a[0]);
				exit(1);}
			
		    	 }
			 else if (cpid < 0)
   			 {
				//The fork failed.
				perror("Error: "); 
				printf("timekeeper experienced an error in starting the command: %s\n",a[0]);
				exit(1);
			 }
			 else if(cpid>0)
		    	 {
				
				//Close open ends 
				close(mypipe[countOfPipes][1]);
				countOfPipes++;	
				
				//Open stat and status files 
				long int pid=cpid;
				sprintf(fname, "/proc/%ld/stat",pid); 
				sprintf(fname2, "/proc/%ld/status",pid);
				file = fopen(fname, "r");
				file2 = fopen(fname2, "r");

				//If file is empty return error
				if(file==NULL || file2==NULL)
				{ perror("Error in opening file"); 
			          exit(1); }
	
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
	
				//wait for child process
				returnValue=waitpid(cpid,&status,0);
	
				//Check if the child process terminated normally or was interrupted by signal
				if(WEXITSTATUS(status)==0)
					
				{printf("The command \"%s\" has returned status code = %d \n",a[0],WEXITSTATUS(status));}
	
				else if(WIFSIGNALED(status))
				{
				int sig=WTERMSIG(status);
				printf("The command \"%s\" is interrupted by the signal number = %d (%s)\n",a[0],sig,signals[sig]);
				}

				//Displaying all information
				long int ut = atol(temp[13]);
				float utime= ut/sysconf(_SC_CLK_TCK);
				long int st = atol(temp[14]);
				float stime= st/sysconf(_SC_CLK_TCK);
				float time=diff_s(end, start);
	printf("Real: %.3fs, user: %.3fs, system: %.3fs, context switch: %ld\n", time, utime, stime, numberOfContextSwitches);
				free(str2);

				//Reset arguments
				for(k=0;k<count;k++)
				{ a[k]=NULL; }
				count=0;

		
			} 	
		}
		else
		{
			//Add arguments to array a
			a[count]=argv[j];
			count++;
		}
	      }

	
	/*---------------------------------------------------------------------------------------------*/
	
	        //Executing last instruction 
		if(numberOfPipes==1)
		{ a[count]=NULL;}

		

		struct timespec start2;
		struct timespec end2;

		//Create child process for last instruction
		cpid=fork();
		
		if (cpid!=0)
		{
		
			printf("Process with id: %d created for the command: %s\n",cpid,a[0]); 
			//Start counting real time
			clock_gettime(CLOCK_REALTIME,&start2);		
			//Sending signal to child process to begin execution from parent
		        kill(cpid,SIGUSR1); 	
			siginfo_t signalInfo;
			waitid(P_PID, cpid, &signalInfo, WEXITED | WNOWAIT);
			//Stop counting real time
			clock_gettime(CLOCK_REALTIME,&end2);
			real_time = diff_s(end2, start2);
		}
	
		if(cpid==0)
		{
			//Default action for SIGINT
			signal(SIGINT, SIG_DFL);

			//Open read end of previous pipe and close write end 
			close(mypipe[countOfPipes-1][1]);
			dup2(mypipe[countOfPipes-1][0],0);

			//Execute
			if (execvp(a[0],a) == -1) {
			perror("execvp: ");
			printf("timekeeper experienced an error in starting the command: %s\n",a[0]);
			exit(1);}
		}
	 	else if (cpid < 0)
   		{
			//The fork failed.
			perror("Error: "); 
			printf("timekeeper experienced an error in starting the command: %s\n",a[0]);
			exit(1);
		}
		else if(cpid>0)
		{
			//Close open ends 
			close(mypipe[countOfPipes][1]);
			countOfPipes++;

			//Open stat and status files 
			long int pid=cpid;
			sprintf(fname, "/proc/%ld/stat",pid); 
			sprintf(fname2, "/proc/%ld/status",pid);
			file = fopen(fname, "r");
			file2 = fopen(fname2, "r");

			//If file is empty return eror
			if(file==NULL || file2==NULL)
			{ perror("Error in opening file");
			  exit(1); }
	
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
	
			//Wait for child process
			returnValue=waitpid(cpid,&status,0);
	
			//Check if the child process terminated normally or interrupted by signal
			if(WEXITSTATUS(status)==0)
				
			{printf("The command \"%s\" has returned status code = %d \n",a[0],WEXITSTATUS(status));}
	
			else if(WIFSIGNALED(status))
			{
				int sig=WTERMSIG(status);
				printf("The command \"%s\" is interrupted by the signal number = %d (%s)\n",a[0],sig,signals[sig]);
			}

			//Displaying all information
			long int ut = atol(temp[13]);
			float utime= ut/sysconf(_SC_CLK_TCK);
			long int st = atol(temp[14]);
			float stime= st/sysconf(_SC_CLK_TCK);
			float time=diff_s(end, start);
	printf("Real: %.3fs, user: %.3fs, system: %.3fs, context switch: %ld\n", time, utime, stime, numberOfContextSwitches);

			free(str2);
		
		} 
	}
		
	
}

