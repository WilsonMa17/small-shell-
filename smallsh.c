#define _GNU_SOURCE
#include <sys/wait.h> // for waitpid
#include <stdio.h>    // for printf and perror
#include <stdlib.h>   // for exit
#include <unistd.h>   // for execlp, getpid, fork
#include <string.h>   // for string actions
#include <fcntl.h>


/*
The following program creates a simple bash shell program. It runs 3 built-in commands exit,cd, and status. Non built-in commands is passed to the exec() family functions. This program handles input redirection in input and output, signal handlers and background/foreground processes. 
*/

int $$;
char command[30];
char exit1[5];
char status[7];
char cd[10];
char *token;
char *token1;
char *token2;
char *E1;
char A = '#';
char B[10];
char C[10];
char D[10];
char E[10];
char bg[2] = "&";
char redirection[2] = ">";
char redirection1[2] = "<";
char blank[2] = "";
int childStatus;
int sigstatus = 0;
int sigstatus1 = 0;
int backgroundprocess = 0;


/* signal handler for SIGINT */
void handle_SIGINT(int signo){
	sigstatus = 1;
  char* message = "TERMINATED BY signal 2\n";
  // We are using write rather than printf
	write(STDOUT_FILENO, message, 22);
  fflush(stdout);
}
/* signal handler for SIGTSTP */
void handle_SIGTSTP (int signo){
	sigstatus1 = 1;
  char* message = "Entering foreground-only mode (& is now ignored)";
  // We are using write rather than printf
	write(STDOUT_FILENO, message, 40);
  fflush(stdout);
}

int main(void){

while(1){

// Initialize SIGINT and SIGTSTP_action struct to be empty
	struct sigaction SIGINT_action = {0};
  struct sigaction SIGTSTP_action = {0};

  // Fill out the SIGINT_action struct
  // Register handle_SIGINT as the signal handler
	SIGINT_action.sa_handler = handle_SIGINT;
  SIGTSTP_action.sa_handler = handle_SIGTSTP;
  // Block all catchable signals while handle_SIGINT is running
	sigfillset(&SIGINT_action.sa_mask);
  sigfillset(&SIGTSTP_action.sa_mask);
  // No flags set
	SIGINT_action.sa_flags = 0;
  SIGTSTP_action.sa_flags = 0;

  // Install our signal handler
	sigaction(SIGINT, &SIGINT_action, NULL);
  sigaction(SIGTSTP, &SIGTSTP_action, NULL);
 
  // set $$ to the main parent process id
 int $$ = getpid();
 
  // setup buffer size for getline() to retrieve string input from the user 
 char *buffer = NULL;
    size_t bufsize = 0;
    size_t nread;
    buffer = (char *)malloc(bufsize * sizeof(char));

  // : is used as a display prompt to show user that they can input in a command
  printf(": ");
  nread = getline(&buffer,&bufsize,stdin);
  buffer[strcspn(buffer, "\n")] = '\0';

  // tokens are used to retrieve strings that are seperated by spaces
  token = strtok(buffer, " "); 
  token = strtok(NULL, " ");
  token1 = strtok(NULL, " "); 
  token2 = strtok(NULL, " ");  

  // copies the second string into char, later used for input/output redirection if needed
  if(NULL != token)
{   
    strcpy(B, token);
    strcpy(C,token);
}
if(NULL != token1)
{   
    strcpy(E, token1);
}
  //printf("Command : \n%s\n",buffer);
  //printf("\nHere is the token : -%s-\n",token);
  //printf("\nHere is the 2nd token : %s\n",token1);

  // cd, exit ,and status built-in command 
  strcpy(cd,"cd");
  strcpy(exit1,"exit");
  strcpy(status,"status");
  strcpy(D,"^C");
  
  //comparing user input with the built-in commands
  int chdirect = strcmp(cd,buffer);
  int exitprogram = strcmp(exit1,buffer);
  int programstatus = strcmp(status,buffer);
  
  // gets exit status of last run command 
  if (programstatus == 0){
    if(WIFEXITED(childStatus)){
      printf("exit value: %d\n", WEXITSTATUS(childStatus));
      continue;
    } else{
      printf("exit value: %d\n", WTERMSIG(childStatus));
    }
  }
  // changes the current directory to specified location using chdir()
  else if (chdirect == 0){
  chdir(token);
  printf("Changing dir to %s",token);
  continue;
  }
  // exits the program and terminates all processess
  else if (exitprogram == 0){
    printf("Exiting shell");
    exit(0);
  }
  // Prompts shell again if input is a comment or blank line
  else if (buffer[0] == A || strcmp(buffer,blank) == 0){
    continue;
  }
  else{
  // Non built-in commands will be passed to the execlp() function with a forked child process
  // Fork a new process
  pid_t spawnPid = fork();
  int childId = spawnPid;
  
  switch(spawnPid){
    case -1:
      perror("fork()\n");
      exit(1);
      break;
    case 0:
    
      // if stdin, opens/creates file descriptor file and output
      if (strcmp(B,redirection) == 0) { 
      int targetFD = open(token1, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      int result = dup2(targetFD, 1);
      execlp(buffer,buffer, NULL);
      //flushes out buffer stdout
      fflush(stdout);
      if (targetFD == -1) {
          perror("open()");
          fflush(stdout);
          exit(1);
          break;
      }
      if (result == -1) {
          perror("dup2"); 
          fflush(stdout);
          exit(2); 
          break;
     }
      // execlp only returns if there is an error
      perror("Incorrect command");
      fflush(stdout);
      exit(1);
      break;
      }
      // if redirection input is detected for reading, opens file if exist, else error
   else if (strcmp( C, redirection1) == 0){
        int sourceFD = open(token1, O_RDONLY);
         if (sourceFD == -1) {
            printf("Cannot open %s for input\n",token1);
            fflush(stdout);
            exit(1);
          }
        int result = dup2(sourceFD, 0);
          if (result == -1) {
            perror("dup2"); 
            fflush(stdout);
            exit(2); 
           }
      execlp(buffer,buffer, NULL);
        //flushes out buffer stdout
      fflush(stdout);
       // execlp only returns if there is an error
      perror("Incorrect command");
      fflush(stdout);
      exit(1);
      break;
      }
  // if & is entered to place command into background
  else if (strcmp(bg, E) == 0){
    printf("background pid is %d\n",getpid());
    fflush(stdout);
    execlp(buffer,buffer,token, NULL);
      //flushes out buffer stdout
    fflush(stdout);
      // execlp only returns if there is an error
    perror("Incorrect command");
    exit(1);
    break;
  }
  else {
      // In the forked child process
      // Replace the current program with the path name of the command
      execlp(buffer,buffer,token,token,token1, NULL);
      //flushes out buffer stdout
      fflush(stdout);
      // execlp only returns if there is an error
      perror("Incorrect command");
      fflush(stdout);
      exit(1);
      break;
  }
   //In the parent process waiting for child's termination
    default:
      /*reset the following chars to prevent infinite
        looping in the reading and writing errors */
    strcpy(B,buffer);
    strcpy(C,buffer);
    strcpy(E,buffer);
    if (strcmp(bg, E) == 0){
      printf("ENDING %d\n",spawnPid);
      fflush(stdout);
    spawnPid = waitpid(spawnPid, &childStatus, WNOHANG);
    }
    // parent waits for child to terminate 
    spawnPid = waitpid(spawnPid, &childStatus, 0);
    /*SIGINT handler will set off flag that will run the following code, 
     the child process stopped by CTRL-C will terminate itself by running SIGKILL */
    if (sigstatus == 1 ){
      printf("\nSIGINT terminated child process: %d\n",childId);
      fflush(stdout);
      spawnPid = waitpid(spawnPid, &childStatus, 0);
      sigstatus = 0;
    }   
      //printf("\nPARENT(%d): child(%d) terminated. Exiting\n", getpid(), spawnPid);
      fflush(stdout);
      //exit(0);
      break;    
  }
}
}}