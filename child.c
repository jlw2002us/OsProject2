#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <stdint.h>

#define  NOT_READY  -1
#define  FILLED     0
#define  TAKEN      1

struct Memory {
     int  status;      
     long long  int seconds;
     long long int milliseconds;
     int childQueue;
};

int  main(int argc, char* argv[])
{
    
    //convert arg passed from parent to int      
    int n = atoi(argv[1]);
     //uint64_t integer;
    key_t          ShmKEY;
    int            ShmID;
    struct Memory  *ShmPTR;

    ShmKEY = ftok(".", 'x');
    ShmID = shmget(ShmKEY, sizeof(struct Memory), 0666);
    if (ShmID < 0) {
          printf("*** shmget error (client) ***\n");
          exit(1);
    }
    
    ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
    if ((int) ShmPTR == -1) {
          printf("*** shmat error (client) ***\n");
          exit(1);
     }
    
    while (ShmPTR->status != FILLED)
          ;

    ShmPTR->status = TAKEN;
    printf("Child incremented %d milliseconds\n", n*1000000);    
    //increment clock    
    
    for (int x = 0; x < n * 1000000; x++)
    {
      ShmPTR->milliseconds = ShmPTR->milliseconds + 1;    
      if(ShmPTR->milliseconds > 999)
      {
       ShmPTR->seconds = ShmPTR->seconds + 1;
       ShmPTR->milliseconds = ShmPTR->milliseconds - 1000;
       }
     }
    
        
    shmdt((void *) ShmPTR);
    
    exit(0);
}
