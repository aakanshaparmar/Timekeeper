/*
#################################
#CSIS0230_3230 Lab 1 Exercise	#
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



int main(int argc, char** argv){
	
	// TODO: Process the arguments
	int i;
	char* a[argc];
	for(i = 0; i<argc; i++)
	{
	  a[i]=argv[i+1];
  	}  
	
	// TODO: Create the child process.
	pid_t cpid;
	cpid=fork();

	// TODO: Branch the program routine for parent and child
	if(cpid==0)
	{
	  
           if (execvp(a[0],a) == -1) {
		perror("Error: ");
		exit(1);
	}}

	int status;
	int returnValue;
	returnValue=waitpid(cpid,&status,0);
	
	if(WIFEXITED(status))
	{
	
		printf("The child process %s has returned with code %d \n",a[0],WEXITSTATUS(status));
	
	}
	
	

	return 0;			
  
}

