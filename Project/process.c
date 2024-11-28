#include "headers.h"

#define RUN_TIME atoi(argv[1])
#define PROCESS_ID atoi(argv[2])
#define ARRIVAL_TIME atoi(argv[3])

/* Modify this file as needed*/
/*int remainingTime;*/
int termMsgid;

void stopProcess(int signum);

struct processStateInfo processInfo;
struct processStateInfoMsgBuff processMessage;

int main(int agrc, char *argv[]) {
  // raise(SIGSTOP);
  signal(SIGUSR1, stopProcess); // use SIGUSR1 as a signal for stopping
  key_t termKey = ftok("Terminating_Processes_KeyFile", 2);
  termMsgid = msgget(termKey, 0666);
  initClk();

  processMessage.mtype = 1;

  processInfo.startTime = getClk();
  processInfo.runTime = RUN_TIME;
  processInfo.id = PROCESS_ID;
  processInfo.arrivalTime = ARRIVAL_TIME;

  int x = getClk();
  processInfo.remainingTime = RUN_TIME;
  while (processInfo.remainingTime > 0) {
    if (getClk() != x) {
      processInfo.remainingTime--;
      x = getClk();
      printf("%d\t %d\n",processInfo.id ,processInfo.remainingTime);
    }
  }
  destroyClk(0);

  processInfo.finishTime = x;
  processMessage.processState = processInfo;

  msgsnd(termMsgid, &processMessage,
         sizeof(struct processStateInfoMsgBuff) - sizeof(long), !IPC_NOWAIT);
  kill(getppid(), SIGUSR2);
  return 0;
}

void stopProcess(int signum) {
  // printf("stopping!");
  processMessage.processState = processInfo;
  msgsnd(termMsgid, &processMessage,
         sizeof(struct processStateInfoMsgBuff) - sizeof(long), !IPC_NOWAIT);
  raise(SIGSTOP);
}
