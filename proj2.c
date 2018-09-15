 //Jennifer Wasson
 //Operating Systems
 //Project 2
 
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
//https://github.com/angrave/SystemProgramming/wiki/Forking,-Part-2:-Fork,-Exec,-Wait

#define  NOT_READY  -1
#define  FILLED     0
#define  TAKEN      1

struct Memory {
     int  status;
     unsigned long long int  seconds;
     unsigned long long int  milliseconds;
     int  childQueue;
};
  struct Memory  *ShmPTR;
  pid_t childID;
  int ShmID;
  bool signal_interrupt = false;

//zombie cleanup
void cleanup(int signal) {
 
  while (waitpid((pid_t) (-1), 0, WNOHANG) > 0) {}
  ShmPTR->childQueue--;  
}  

//catch control-c
void  INThandler(int sig)
{
      
    signal(sig, SIG_IGN);  
    write(STDOUT_FILENO,"  Control-C hit ...\n\n",16);
//    printf("Server is reading from child.. %llu seconds, %llu milliseconds\n %d child processes\n", ShmPTR->seconds, ShmPTR->milliseconds,ShmPTR->childQueue);   
  //  shmdt((void *) ShmPTR);
    // printf("\nServer has detached its shared memory...\n");
    //shmctl(ShmID, IPC_RMID, NULL);
    //printf("Server has removed its shared memory...\n");
    //printf("Server exits...\n");
    //exit(0);
    signal_interrupt = true; alarm(0);
}

//catch alarm 
void  ALARMhandler(int sig)
{ if(signal_interrupt == false)
  write (STDOUT_FILENO,"Alarm went off\n",16);
        signal(SIGALRM, SIG_IGN);
//        printf("Server is reading from child.. %llu seconds, %llu milliseconds\n %d child processes\n", ShmPTR->seconds, ShmPTR->milliseconds,ShmPTR->childQueue);              
  //      shmdt((void *) ShmPTR);
    //    printf("\nServer has detached its shared memory...\n");
      //  shmctl(ShmID, IPC_RMID, NULL);
        //printf("Server has removed its shared memory...\n");
        //printf("Server exits...\n");
        //exit(0);
  signal(SIGALRM, ALARMhandler);
       signal_interrupt = true;
}

 int main(int argc, char* argv[])
  {

     signal(SIGALRM, ALARMhandler);
     signal(SIGCHLD, cleanup);
     alarm(2); //program can only run 2 seconds;
     float finaltime;
     signal(SIGINT, INThandler);
     int i;
     
     clock_t start,end;
     key_t ShmKEY;
     start = clock();
     ShmKEY = ftok(".", 'x');
     ShmID = shmget(ShmKEY, sizeof(struct Memory), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
      
    printf("Server has received a shared memory of two integers...\n");

    ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
    if ((int) ShmPTR == -1) {
      printf("*** shmat error (server) ***\n");
      exit(1);
    }
    printf("Server has attached the shared memory...\n");

    ShmPTR->status  = NOT_READY;
    ShmPTR->seconds = 0;
    ShmPTR->milliseconds = 0;
    ShmPTR->childQueue = 0;

    printf("Server(Master) has filled %llu %llu %d to shared memory...\n",
            ShmPTR->seconds, ShmPTR->milliseconds, ShmPTR->childQueue);
    ShmPTR->status = FILLED;
    while(signal_interrupt == false){
    for(i = 0; i < 100; i++){if(signal_interrupt == true) break;
     if (ShmPTR->childQueue  < 50){ 
         
         ShmPTR->childQueue++;                
         if ((childID = fork()) == -1) {     /* Start a child process.      */
           perror("fork error");
           exit(EXIT_FAILURE);
        }
     
       else if (childID == 0) {             /* This is the child.          */
        signal(SIGINT, SIG_IGN);     
        execvp(argv[1], &argv[1]);
        perror("child failed to execvp the command");
        return 1;
       }
       else {                              /* This is the parent.         */
          	     
                     
           while (ShmPTR->status != TAKEN)
           ;
         
                ShmPTR->status = FILLED;
             
        }}
        else{ i = i - 1; } //wait for a child to finish
//         printf("%d", ShmPTR->childQueue);   
       
    }       
           
      
     if (signal_interrupt == true) break;
     end = clock();
     clock_t finalTime = end - start;
     //printf("Server is reading from child.. %llu seconds, %llu milliseconds\n %d child processes\n", ShmPTR->seconds, ShmPTR->milliseconds,ShmPTR->childQueue);
     
       do{ if(signal_interrupt == true) break; 
       printf("Clock ticking..\n");
       sleep(1);
     finaltime = (float)finalTime/CLOCKS_PER_SEC;
     }while (finaltime < 2);}
     printf("Server is reading from child.. %llu seconds, %llu milliseconds\n %d child processes\n", ShmPTR->seconds, ShmPTR->milliseconds,ShmPTR->childQueue);
     shmdt((void *) ShmPTR);
     printf("Server has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("Server has removed its shared memory...\n");
     printf("Server exits...\n");
     exit(EXIT_SUCCESS);
  }
