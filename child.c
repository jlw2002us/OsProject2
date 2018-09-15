#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>


#define  NOT_READY  -1
#define  FILLED     0
#define  TAKEN      1

struct Memory {
     int  status;
     unsigned long long int  seconds;
     unsigned long long int milliseconds;
     int childQueue;
};

int  main(int argc, char* argv[])
{
    
    //convert arg passed from parent to int      
    int n = atoi(argv[1]);
       
    key_t          ShmKEY;
    int            ShmID;
    struct Memory  *ShmPTR;

    ShmKEY = ftok(".", 'x');
    ShmID = shmget(ShmKEY, sizeof(struct Memory), 0666);
    if (ShmID < 0) {
          printf("*** shmget error (client) ***\n");
          exit(1);
    }
    printf("   Client has received a shared memory of two integers...\n");

    ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
    if ((int) ShmPTR == -1) {
          printf("*** shmat error (client) ***\n");
          exit(1);
     }
    printf("   Client has attached the shared memory...\n");

    while (ShmPTR->status != FILLED)
          ;
    printf("   Client found the data is ready...\n");
    printf("   Client found %llu %llu %d in shared memory...\n",
                ShmPTR->seconds, ShmPTR->milliseconds,ShmPTR->childQueue);

    ShmPTR->status = TAKEN;
    //ShmPTR->seconds = ShmPTR->seconds + 1;
    ShmPTR->milliseconds = ShmPTR->milliseconds +(n * 1000);
    printf("%llu", ShmPTR->milliseconds);    
    while(ShmPTR->milliseconds > 999)
    {
       ShmPTR->seconds = ShmPTR->seconds + 1;
       ShmPTR->milliseconds = ShmPTR->milliseconds - 1000;
    }
    printf("   Client has informed server data have been taken...\n");
    shmdt((void *) ShmPTR);
    printf("   Client has detached its shared memory...\n");
    printf("   Client exits...\n");
    exit(0);
}
