 //Jennifer Wasson
 //Operating Systems
 //Project 2
 #include <math.h>
 #include  <sys/shm.h>
 #include <sys/types.h>
  #include <sys/wait.h>
  #include <unistd.h>
  #include <time.h>
  #include <stdio.h>
  #include <stdlib.h> 
  #include <signal.h>
  #include <stdbool.h>
  #include <time.h>
 
//Sources used:  https://support.sas.com/documentation/onlinedoc/sasc/doc/lr2/waitpid.htm
//https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c
//https://stackoverflow.com/questions/13273836/how-to-kill-child-of-fork?rq=1
//https://stackoverflow.com/questions/1784136/simple-signals-c-programming-and-alarm-function

#define  NOT_READY  -1
#define  FILLED     0
#define  TAKEN      1

struct Memory {
     int  status;
     int  seconds;
     int  milliseconds;
};
  struct Memory  *ShmPTR;
  pid_t childID;
  int ShmID;
  
//catch control-c
void  INThandler(int sig)
{
     
     signal(sig, SIG_IGN);  
          
        shmdt((void *) ShmPTR);
        printf("Server has detached its shared memory...\n");
        shmctl(ShmID, IPC_RMID, NULL);
        printf("Server has removed its shared memory...\n");
        printf("Server exits...\n");
        kill(childID, SIGTERM);
        exit(0);
}
//catch alarm 
void  ALARMhandler(int sig)
{
  signal(SIGALRM, SIG_IGN);          /* ignore this signal       */
   shmdt((void *) ShmPTR);
        printf("Server has detached its shared memory...\n");
        shmctl(ShmID, IPC_RMID, NULL);
        printf("Server has removed its shared memory...\n");
        printf("Server exits...\n");
        kill(childID, SIGTERM);
        exit(0);
  signal(SIGALRM, ALARMhandler);     /* reinstall the handler    */
}

 int main(int argc, char* argv[])
  {
     signal(SIGALRM, ALARMhandler);
     alarm(2); //program can only run 2 seconds;
     float finaltime;
     double cpu_time_used;
     bool died = false;
     signal(SIGINT, INThandler);
     int i, status;
     pid_t endID;
     clock_t start,end;
     key_t ShmKEY;
     int childQueue = 0;
     int n = atoi(argv[1]);
     
     start = clock();
     ShmKEY = ftok(".", 'x');
     ShmID = shmget(ShmKEY, sizeof(struct Memory), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
      
//    printf("Server has received a shared memory of two integers...\n");

    ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
    if ((int) ShmPTR == -1) {
      printf("*** shmat error (server) ***\n");
      exit(1);
    }
  //  printf("Server has attached the shared memory...\n");

    ShmPTR->status  = NOT_READY;
    ShmPTR->seconds = 0;
    ShmPTR->milliseconds = 0;

    //printf("Server(Master) has filled %d %d to shared memory...\n",
      //      ShmPTR->seconds, ShmPTR->milliseconds);
    ShmPTR->status = FILLED;
 
     for(int i=0; i < 100; i++){ 
       
       if(childQueue <= 3){    
     if ((childID = fork()) == -1) {     /* Start a child process.      */
        perror("fork error");
        exit(EXIT_FAILURE);
     }
     
     else if (childID == 0) {            /* This is the child.          */
        childQueue++;   
        execvp(argv[1], &argv[1]);
        perror("child failed to execvp the command");
        return 1;
     }
     else {                              /* This is the parent.         */
         
         while(true){
         endID = waitpid(childID, &status, WNOHANG|WUNTRACED);
         
         while (ShmPTR->status != TAKEN)
           ;
         //printf("Server is reading from child.. %d seconds, %d milliseconds\n", ShmPTR->seconds, ShmPTR->milliseconds);
         if (endID == -1) {            /* error calling waitpid       */
              perror("waitpid error");
              exit(EXIT_FAILURE);
           }
         
          else if (endID == childID) {  /* child ended                 */
            //printf("Server has detected the completion of its child...\n");
                ShmPTR->status = FILLED;
                childQueue--;    
               if (WIFEXITED(status)){
                  printf("Child ended normally.\n");
                }
              else if (WIFSIGNALED(status))
                 printf("Child ended because of an uncaught signal.n");
              else if (WIFSTOPPED(status))
                 printf("Child process has stopped.n");
              
              died = true; break;
           }
          }
        }
       }
       else{ i = i - 1;} //wait for child to finish
      }
      if(died == false) kill(childID, SIGTERM);
      
     end = clock();
     clock_t finalTime = end - start;
     do{ 
       printf("Clock ticking..\n");
       sleep(1);
       finaltime = (float)finalTime/CLOCKS_PER_SEC;
     }while (finaltime < 2);
     shmdt((void *) ShmPTR);
     printf("Server has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("Server has removed its shared memory...\n");
     printf("Server exits...\n");
     exit(EXIT_SUCCESS);
  }
