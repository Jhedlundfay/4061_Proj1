/* CSci4061 F2018 Assignment 1
* login: cselabs login name (login used to submit)
* date: mm/dd/yy
* name: full name1, full name2, full name3 (for partner(s))
* id: id for first name, id for second name, id for third name */

// This is the main file for the code
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"

/*-------------------------------------------------------HELPER FUNCTIONS PROTOTYPES---------------------------------*/
void show_error_message(char * ExecName);
//Write your functions prototypes here
void show_targets(target_t targets[], int nTargetCount);
/*-------------------------------------------------------END OF HELPER FUNCTIONS PROTOTYPES--------------------------*/


/*-------------------------------------------------------HELPER FUNCTIONS--------------------------------------------*/

//This is the function for writing an error to the stream
//It prints the same message in all the cases
void show_error_message(char * ExecName)
{
	fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", ExecName);
	fprintf(stderr, "-f FILE\t\tRead FILE as a makefile.\n");
	fprintf(stderr, "-h\t\tPrint this message and exit.\n");
	exit(0);
}



//Our (students) functions start here

//Phase1: Warmup phase for parsing the structure here. Did it as per the PDF (Writeup)
/* The function show_targets takes in a list (array) of nodes (defined in util.h),
   where each struct has properties that we'd like to print out / traverse for this warm up. 
   show_targets also takes in an integer : nTargetCount, which will be the starting address 
   of this array structure. For each node in the array, we print out it's target. Now, for 
   each target in a node, we print out the number of dependencies it has, along with the names
   of those dependencies. Then for each dependency, we print out its command.
*/
void show_targets(target_t targets[], int nTargetCount){
	for(int i=0;i<nTargetCount;i++){
		target_t curtarget = targets[i];
		printf("Name of target: %s\n", curtarget.TargetName);
		printf("\tNumber of dependencies: %d\n", curtarget.DependencyCount);
		for(int j = 0;j<curtarget.DependencyCount;j++){
			int x = find_target(curtarget.DependencyNames[j],targets,nTargetCount);
				if(x == -1){
					printf("\t\tTarget not found: %s\n",curtarget.DependencyNames[j]);
				}
				else{
					printf("\t\tDependency %d is %s and location is index %d\n",j,curtarget.DependencyNames[j],x);
				}
		}
	  printf("\tCommand: %s\n",curtarget.Command);
	  printf("\n\n");
        }
}

/* Thie funciton file_check is all in the name. Here, we use a given helpful function to 
   check if a file exists. If it doesn't, an error is thrown. */
int file_check(char filename[]){
	int access_result = does_file_exist(filename);
	if (access_result == -1) {
		perror("File not found");
	}
	else{
		return 0;
	}
 }	


/* 
   The funciton fork_exec a helper function that contains our fork and excecution of
   our processes. The format of using fork in fork_exec follows closely the discussion 
   of fork() in class. To execute / print the correct command, we rely on the use of 
   parse_into_tokens to make a command "string" (array of characters). 
*/

int fork_exec(char current_command[]) {
  pid_t parent = getpid();
  pid_t pid = fork();

// Assumed that maximum characters on a line = 1023
  char *cmd[1023];  

// result helps us print later in the function
  int result = parse_into_tokens(current_command,cmd," "); 

  if (pid == -1) {
    perror("Failed to fork");
    exit(-1);
  }
  //parent process
  if (pid > 0) {   
    int status;

   //wait for child
    wait(&status); 
    if(WEXITSTATUS(status) != 0)
	{
	  printf("Child exited with error code = %d\n", WEXITSTATUS(status));
	  exit (-1);
	}
  } 
  else{
  //child process  

    // Print out the command being executed here
    for (int i=0; i<result;i++)  
	{
	 printf("%s ",cmd[i]);
	}
     printf("\n"); 

     // Execute command
     execvp(cmd[0], cmd);        
  }
  
return FINISHED;
}


/* The function traverse_graph uses a Depth-first-search traversal to enable the bottom-> up 
   approach of executing leaf commands in a dependency tree first and working our way back 
   "up" via recursion. 

*/
int traverse_graph(target_t targets[],int nTargetCount,char target_name[]){
		//find if target_name is name of target in array
		int target_location = find_target(target_name,targets,nTargetCount);
		if (target_location == -1){
			if (file_check(target_name) == 1){
				perror("Invalid name");
				return ERROR;
			}
			else {

			  //A base case of recursion - returning 1
			  return FINISHED;
	               	}
		}
		else {

                  //Set a point of reference to the current target
		  target_t current_target = targets[target_location];
		
		  //If the current target has dependencies, recursively call that dependency
		  for(int j=0; j<current_target.DependencyCount; j++){ 

		    traverse_graph(targets,nTargetCount,current_target.DependencyNames[j]);
			
		  }
		}
		
		 //If we end up here, we are at a leaf
		 target_t current_target = targets[target_location];
 
		//If the leaf has no dependencies, execute its command
		if (current_target.DependencyCount == 0){
			 return(fork_exec(current_target.Command));
		   }
		 //If the leaf has dependencies, then only execute the command if the files were updated 
		 for(int j=0; j<current_target.DependencyCount; j++){
			int comp = compare_modification_time(target_name,current_target.DependencyNames[j]);
			if ((comp == NEEDS_BUILDING) || (comp == UNFINISHED) || (comp == ERROR))
			   {
				return(fork_exec(current_target.Command));
			   }
		     }

		return FINISHED;
            
		
}

//Our (students) functions end here
/*-------------------------------------------------------END OF HELPER FUNCTIONS-------------------------------------*/


/*-------------------------------------------------------MAIN PROGRAM------------------------------------------------*/
//Main commencement
int main(int argc, char *argv[])
{
  target_t targets[MAX_NODES];
  int nTargetCount = 0;

  /* Variables you'll want to use */
  char Makefile[64] = "Makefile";
  char TargetName[64];

  /* Declarations for getopt. For better understanding of the function use the man command i.e. "man getopt" */
  extern int optind;   		// It is the index of the next element of the argv[] that is going to be processed
  extern char * optarg;		// It points to the option argument
  int ch;
  char *format = "f:h";
  char *temp;

  //Getopt function is used to access the command line arguments. However there can be arguments which may or may not need the parameters after the command
  //Example -f <filename> needs a finename, and therefore we need to give a colon after that sort of argument
  //Ex. f: for h there won't be any argument hence we are not going to do the same for h, hence "f:h"
  while((ch = getopt(argc, argv, format)) != -1)
  {
	  switch(ch)
	  {
	  	  case 'f':
	  		  temp = strdup(optarg);
	  		  strcpy(Makefile, temp);  // here the strdup returns a string and that is later copied using the strcpy
	  		  free(temp);	//need to manually free the pointer
	  		  break;

	  	  case 'h':
	  	  default:
	  		  show_error_message(argv[0]);
	  		  exit(1);
	  }

  }

  argc -= optind;
  if(argc > 1)   //Means that we are giving more than 1 target which is not accepted
  {
	  show_error_message(argv[0]);
	  return -1;   //This line is not needed
  }

  /* Init Targets */
  memset(targets, 0, sizeof(targets));   //initialize all the nodes first, just to avoid the valgrind checks

  /* Parse graph file or die, This is the main function to perform the toplogical sort and hence populate the structure */
  if((nTargetCount = parse(Makefile, targets)) == -1)  //here the parser returns the starting address of the array of the structure. Here we gave the makefile and then it just does the parsing of the makefile and then it has created array of the nodes
	  return -1;


  //Phase1: Warmup-----------------------------------------------------------------------------------------------------
  //Parse the structure elements and print them as mentioned in the Project Writeup
  /* Comment out the following line before Phase2 */


  //If you un-comment our show_targets (below), you may see our warmup work
  //show_targets(targets, nTargetCount);  


  //End of Warmup------------------------------------------------------------------------------------------------------

  /*
   * Set Targetname
   * If target is not set, set it to default (first target from makefile)
   */
  if(argc == 1) {
	strcpy(TargetName, argv[optind]);
  } // here we have the given target, this acts as a method to begin the building
  else {
  	strcpy(TargetName, targets[0].TargetName);
  } // default part is the first target

  /*
   * Now, the file has been parsed and the targets have been named.
   * You'll now want to check all dependencies (whether they are
   * available targets or files) and then execute the target that
   * was specified on the command line, along with their dependencies,
   * etc. Else if no target is mentioned then build the first target
   * found in Makefile.
   */

  //Phase2: Begins ----------------------------------------------------------------------------------------------------
  /*Your code begins here*/
  return (traverse_graph(targets,nTargetCount,TargetName));




  /*End of your code*/
  //End of Phase2------------------------------------------------------------------------------------------------------

  return 0;
}

/*-------------------------------------------------------END OF MAIN PROGRAM------------------------------------------*/
