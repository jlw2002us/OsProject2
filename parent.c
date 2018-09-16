 //Jennifer Wasson
 //Operating Systems
 //Project 2
  #include <string.h>
  #include <ctype.h> 
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

void helpoptions()
{
  printf("options for running Project 2:  \n");
  printf("type ./parent -h for help options\n");
  printf("type ./parent -n -s with values for n and s.\n");
  
}

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
     int i, z;     
     clock_t start,end;
     key_t ShmKEY;
     int c,n,s;
     int hflag;
     int index;
     char *nvalue = NULL;
     char *svalue = NULL;
      opterr = 0;
//     char* execArg = NULL;
//    if (argc != 6)
  //  {
    // fprintf(stderr, "Usage: %s process\n", argv[0]);
     //return 1;
   // }

    //get address for execvp call
    
    for (z=0; z < argc; z++)
    { 
       if((argv[z][0] == '-') && (argv[z][1] == 'n'))
         break;
    }
    printf("%s",argv[z]);
    while ((c = getopt (argc, argv, "hn:s:")) != -1){
      switch (c)
       {
         case 'h':
         hflag = 1;
         break;
      
         case 'n':
          nvalue = optarg;
          break;
         case 's':
          svalue = optarg;
          break;
         case '?':
          if ((optopt == 'n') ||  (optopt == 's'))
           fprintf (stderr, "Option -%c requires an argument.\n", optopt);
          else if (isprint (optopt))
           fprintf (stderr, "Unknown option `-%c'.\n", optopt);
          else
           fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
          return 1;
          default:
            abort ();
      }
    }
  //  printf ("hflag = %d, pflag = %d, nvalue = %s, kvalue = %s\n",
   //       hflag, pflag, nvalue,kvalue);

     for (index = optind; index < argc; index++)
      printf ("Non-option argument %s\n", argv[index]);


     if(hflag == 1)
     {
       helpoptions();
       return 1;
     }

     
     char str1[50];
     char str2[13];
     //check user input for executable
     if(strlen(argv[0]) > 20){
       printf("%s","Executable name too long");
       return 1;
     }
     strcpy(str1, argv[0]);
     strcpy(str2, " : Error: ");
     if(strlen(str2)  < 14)
      strcat(str1,str2);

      //change the number user entered into int
     if(nvalue != NULL){

       n = atoi(nvalue);
     if (n == 0){
       printf("Error:  non-numerical data entered\n");
       return 1;}
     }
     else{
      printf("%s","Error: no n value  entered\n");
      return 1;}

     //change the number user entered into int
     if(svalue!= NULL){

     s = atoi(svalue);
     if (s == 0){
       printf("Error:  non-numerical data entered\n");
       return 1;}
     }
     else{
       printf("%s","Error: no s value entered\n");
       return 1;
     }

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
    for(i = 0; i < n; i++){if(signal_interrupt == true) break;
     if (ShmPTR->childQueue  < s){ 
         
         ShmPTR->childQueue++;  //increment no. of children              
         if ((childID = fork()) == -1) {     /* Start a child process.      */
           perror("fork error");
           exit(EXIT_FAILURE);
        }
     
       else if (childID == 0) {             /* This is the child.          */
        signal(SIGINT, SIG_IGN);
        execvp("./child", &argv[z]);     
        //execvp(argv[1], &argv[2]);
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
     printf("\nServer is reading from child.. %llu seconds, %llu milliseconds\n", ShmPTR->seconds, ShmPTR->milliseconds);
     shmdt((void *) ShmPTR);
     printf("Server has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("Server has removed its shared memory...\n");
     printf("Server exits...\n");
     exit(EXIT_SUCCESS);
  }
