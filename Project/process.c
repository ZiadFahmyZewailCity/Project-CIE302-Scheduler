#include "headers.h"

#define RUN_TIME atoi(argv[1])
#define PROCESS_ID atoi(argv[2])

/* Modify this file as needed*/
/*int remainingTime;*/
int termMsgid;

void stopProcess(int signum);

struct processFinalInfo processInfo;
int main(int agrc, char *argv[]) {
  signal(SIGUSR1, stopProcess); // use SIGUSR1 as a signal for stopping
  key_t termKey = ftok("Terminating_Processes_KeyFile", 2);
  termMsgid = msgget(termKey, 0666);
  initClk();

  processInfo.msgType = 1;
  processInfo.startTime = getClk();
  processInfo.runTime = RUN_TIME;
  processInfo.id = PROCESS_ID;

  int x = getClk();
  processInfo.remainingTime = RUN_TIME;
  while (processInfo.remainingTime > 0) {
    if (getClk() != x) {
      processInfo.remainingTime--;
      x = getClk();
      printf("%d\n", processInfo.remainingTime);
    }
  }
  destroyClk(false);

  processInfo.finishTime = x;
  msgsnd(termMsgid, &processInfo, sizeof(struct processFinalInfo), 0);
  return 0;
}

void stopProcess(int signum) {
  printf("stopping!");
  msgsnd(termMsgid, &processInfo, sizeof(struct processFinalInfo), 0);
  raise(SIGSTOP);
}
