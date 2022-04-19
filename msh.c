
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include <stdbool.h>


#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports 10 additional arguments!


/*run_execvp is to execute the execvp(cmmd, &arg[]) function.
*Expects the token array from user input, 
*Expects the passes_through_history for the Not found in history alternative error.
*
*no return
*
*This helper function is to fork and, while within a child process, 
do a system command ( execvp ) using the token.
*/
void run_execvp(char* token[MAX_NUM_ARGUMENTS] , bool passed_through_history){
      pid_t pid = fork();

      // If Child, pid is 0. 
      if( pid == 0 ){
        // Notice you can add as many NULLs on the end as you want
        int ret = execvp( token[0], &token[0] );  

        //Ran repeat history (!n) command... was not found/allowed.
        if(ret == -1 && passed_through_history == true){
          printf("Command not in history\n");
        }
        else if( ret == -1 ){ 
          /*perror("Error");*/
          printf("%s: Command not found\n", token[0]);
        }
      }
      //Else, parent (non-zero pid) & must wait for child process (above).
      else{
        int status;
        wait( & status );
      }
}


int main()
{
  char * command_string = (char*) malloc( MAX_COMMAND_SIZE );

  //history_index keeps track of history (especially how full it is, for history cmmd).
  int history_index = 0;

  //For keeping track of pid history:
  int pid_index = 0;
  pid_t pids[]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //Will not print 0's !

  while(1){
    //Used for the !n history command.
    //If ! history command is called, we wanted a different error message.
    //So this gets passed to run_execvp in the case of ret == -1
    //True = Not found in history. False = Command not found.
    bool passed_through_history = false; 


    // Print out the msh prompt
    printf ("msh> ");

    //Used for the history command!
    char history[MAX_COMMAND_SIZE][15];

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];
    int   token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;

    char *working_string  = strdup( command_string );

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Fill History List 
    char* string_ptr = strtok(command_string, "\n"); 

    //History list not full:
    //Copy next user input into history.
    if(history_index < 15){
      //strcpy( history[history_index++], token[0] );
      strcpy(history[history_index++], string_ptr); //history shows entire commands.
    }
    //History list is full:
    //Move elements over, copy user input again.
    else{ 
      int l = 0;
      while(l<15){
        strcpy( history[l], history[l+1] ); //Shifts elements to left
        ++l;
      }
      //Then copies next input:
      //strcpy( history[history_index], token[0] );
      strcpy(history[history_index], string_ptr);
      
    }


    // !n command  

    //!n command for history execution cmmd:
    if(command_string != NULL && command_string[0] == '!'){
      //We want to get the n from "!n" by itself, 
      //so we can access history and then execute what is inside history!

      //Takes away "!" from token, to get integer by self.
      char *token_ptr = strtok(command_string, "!");
      //Converts, result from above, to integer type:
      int cmmd_index = atoi(token_ptr);

      
      if(history[cmmd_index] != NULL){
        strcpy(token[0], history[cmmd_index]);//Put history into token[0] to repeat nth history command.


        // Handles special case for cd (still inside the !n command)

        char * dir_hold = strtok(token[0], "cd \n"); //dir_hold is holding ONLY the directory name.
  
        //Compare to see if in history we start w/ "cd", and if so, chdir to dir_hold's directory.
        if(dir_hold != NULL && strncmp(history[cmmd_index], "cd", 2) == 0){
          strcpy(token[0], "cd");
          chdir(dir_hold);
        }
        passed_through_history = true; //For "Command not in history" error in run_execvp.
      }
      else{
        //Backup statement, not in reference with comment above.
        printf("Command not in history!\n"); 
      }
      // * And then token[0] passes through program, executing the nth command. *
    }


    // Get pids for Pidhistory 
    //If pid history list is full:
    //Shift elements, replace last.
    if(pid_index>15){
      int k = 0;
      int temp = 0;
      //shifts list left by 1
      while(k<15){
        if((k+1)<16){
          pids[k] = pids[k+1];
        }
        ++k;
      }
      //& replaces 15th spot:
      pids[15] = getpid();
    }
    //Else, if not full:
    else{ 
      pids[pid_index] = getpid();   //find current pid, add to array
      pid_index++; //Pid history list is not full, so we increase index.
    }


    // History of user commands 

    //Seg faults if strcmp is NULL.
    //Checks for history command, if "history", prints off found history:
    if(token[0] != NULL && strcmp(token[0], "history") == 0){

      //Ensures we do not print off more than requested. 
      //Then prints off each history in array.

      //In other words, if history array only has 5 in the history (0-4),
      //then history_index == 4 and history array 0 - 4 prints!
      if(history_index>0) { printf("0: %s\n", history[0]); }
      if(history_index>1) { printf("1: %s\n", history[1]); }
      if(history_index>2) { printf("2: %s\n", history[2]); }
      if(history_index>3) { printf("3: %s\n", history[3]); }
      if(history_index>4) { printf("4: %s\n", history[4]); }
      if(history_index>5) { printf("5: %s\n", history[5]); }
      if(history_index>6) { printf("6: %s\n", history[6]); }
      if(history_index>7) { printf("7: %s\n", history[7]); }
      if(history_index>8) { printf("8: %s\n", history[8]); }
      if(history_index>9) { printf("9: %s\n", history[9]); }
      if(history_index>10){ printf("10: %s\n",history[10]);}
      if(history_index>11){ printf("11: %s\n",history[11]);}
      if(history_index>12){ printf("12: %s\n",history[12]);}
      if(history_index>13){ printf("13: %s\n",history[13]);}
      if(history_index>14){ printf("14: %s\n",history[14]);}
    }
    //If user wishes to exit ...
    else if(strcmp(token[0], "exit") == 0  ||  strcmp(token[0], "quit") == 0){ 
      exit(EXIT_SUCCESS);
    }
    //Else if user wishes to change dir ...
    else if(strcmp(token[0], "cd") == 0){
      chdir(token[1]);
    }
    //Print last 15 (or less) commands, for pidhistory:
    else if(strcmp(token[0], "pidhistory") == 0){
      int x  = 0;
      //Prints pid history from list of pids, 
      //pids cannot == 0 due to array initialization:
      while(x<15 && pids[x]!=0){  
        printf("%d: %lu\n", x, pids[x]);
        ++x;
      }
    }
    //Else, system call ...
    else{ 
      run_execvp(token, passed_through_history); 
    }

    free( head_ptr );

  }

  return 0;
}
