#include "headers.h"

enum schedulingAlgorithm alg;

int main(int argc, char *argv[]) {
  signal(SIGINT, clearResources);
  initClk();

  alg = atoi(argv[1]);
  unsigned int quantum = atoi(argv[2]);

  // Initialize message queue
  key_t Gen_Sched_Key;
  int Gen_Sched_SND_VAL;
  Gen_Sched_Key = ftok("Gen_Sched_KeyFile", 1);
  Gen_Sched_MSGQ = msgget(Gen_Sched_Key, IPC_CREAT | 0666);
  if (Gen_Sched_MSGQ == -1) {
    perror("Error in Creating Generator to Scheduler Message Queue");
    exit(-1);
  }

  switch (alg) {
  case SJF:

    break;
  case PHPF:

    break;
  case RR:

    break;
  }

  // TODO implement the scheduler :)
  // upon termination release the clock resources

  destroyClk(false);
}

void clearResources(int signum) {
  // TODO Clears all resources in case of interruption
  printf("Clearing scheduler resources...\n");
  /*struct msqid_ds temp;*/
  /*msgctl(Gen_Sched_MSGQ, IPC_RMID, &temp);*/
  /*kill(SchedulerPID, SIGINT);*/
  // Incomplete
  switch (alg) {
  case SJF:

    break;
  case PHPF:

    break;
  case RR:

    break;
  }

  exit(0);
}
