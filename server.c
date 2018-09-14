#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include  <sys/types.h>
#include  <sys/shm.h>
//sources used:  http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/shm/example-2.html


#define  NOT_READY  -1
#define  FILLED     0
#define  TAKEN      1

struct Memory {
     int  status; //taken, filled, or not_ready
     int  seconds;
     int  milliseconds;
};

int  main(int  argc, char *argv[])
{
  key_t          ShmKEY;
  int            ShmID;
  struct Memory  *ShmPTR;
  pid_t childpid;

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

   printf("Server(Master) has filled %d %d to shared memory...\n",
            ShmPTR->seconds, ShmPTR->milliseconds);
   ShmPTR->status = FILLED;

  
 for(int i=0; i<100; i++){     
  childpid = fork();
  if (childpid == 0)
  {
    execvp(argv[1], &argv[1]);
    perror("child failed to execvp the command");
    return 1;
  }
  else{
     wait(NULL);
     
     while (ShmPTR->status != TAKEN)
       sleep(1);
     printf("Server is reading from child.. %d seconds, %d milliseconds\n", ShmPTR->seconds, ShmPTR->milliseconds);

     printf("Server has detected the completion of its child...\n");
     ShmPTR->status = FILLED;
    }
   }
      
     shmdt((void *) ShmPTR);
     printf("Server has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("Server has removed its shared memory...\n");
     printf("Server exits...\n");
     exit(0);
}
