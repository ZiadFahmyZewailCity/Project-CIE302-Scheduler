#include "headers.h"

#define RUN_TIME atoi(argv[1])
#define PROCESS_ID atoi(argv[2])

/* Modify this file as needed*/
/*int remainingTime;*/
int Terminating_Process_MSGQ;
int Terminating_Process_RCV_VAL;

void stopProcess(int signum);

struct processStateInfoMsgBuff processInfo;

int main(int agrc, char *argv[]) {
  raise(SIGSTOP);
  signal(SIGUSR1, stopProcess); // use SIGUSR1 as a signal for stopping
  key_t Terminating_Process_Key;
  Terminating_Process_Key = ftok("Term_Proc_KeyFile", 1);
  Terminating_Process_MSGQ = msgget(Terminating_Process_Key, IPC_CREAT | 0666);
  if (Terminating_Process_MSGQ == -1) {
    perror("Error in Creating Generator to Scheduler Message Queue");
    exit(-1);
  }
  initClk();

  processInfo.mtype = 1;
  processInfo.startTime = getClk();
  processInfo.runTime = RUN_TIME;
  processInfo.id = PROCESS_ID;

  int x = getClk();
  processInfo.remainingTime = RUN_TIME;

  while (processInfo.remainingTime > 0) {
    if (getClk() != x) {
      processInfo.remainingTime--;
      x = getClk();
      // printf("%d\n", processInfo.remainingTime);
    }
  };

  destroyClk(false);

  processInfo.finishTime = x;

  kill(getppid(), SIGUSR2);
  printf("about to terminate this one process with remaining time %d\n",processInfo.remainingTime);
  processInfo.mtype=2;
  msgsnd(Terminating_Process_MSGQ, &processInfo, sizeof(processInfo), !IPC_NOWAIT);
  exit(0);
}

void stopProcess(int signum) {
   printf("stopping process %d!\n\n",processInfo.id);
  processInfo.pstate = waiting;
  msgsnd(Terminating_Process_MSGQ, &processInfo, sizeof(processInfo), !IPC_NOWAIT);
   printf("sent!\n\n");
  raise(SIGSTOP);
}
