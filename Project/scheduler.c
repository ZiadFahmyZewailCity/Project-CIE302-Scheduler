#include "headers.h"

enum schedulingAlgorithm alg;

struct PriorityQueue *pq;

struct processData runningProcess;

void clearResources(int signum);

// Flag used to notify scheduler that the process it holds has been terminated
int processTerminate = 0;
// Number of children
/*int numberChildren = 0;*/
// When processes terminate they will notify the scheduler from here
void handler_SIGCHILD(int signal);

int main(int argc, char *argv[]) {
  signal(SIGINT, clearResources);
  initClk();
  int x = getClk();

  alg = atoi(argv[1]);
  unsigned int quantum = atoi(argv[2]);

#pragma region "Initializing message queue"
  key_t Gen_Sched_Key;
  int Gen_Sched_RCV_VAL;
  int Gen_Sched_MSGQ;
  Gen_Sched_Key = ftok("Gen_Sched_KeyFile", 1);
  Gen_Sched_MSGQ = msgget(Gen_Sched_Key, IPC_CREAT | 0666);
  if (Gen_Sched_MSGQ == -1) {
    perror("Error in Creating Generator to Scheduler Message Queue");
    exit(-1);
  }

  struct processMsgBuff RecievedProcess;
  runningProcess.pid = -1;
#pragma endregion

  PriorityQueue *pq = initialize_priQ();

  switch (alg) {
  case SJF:

    break;
  case PHPF:

    break;
  case RR:
#pragma region "Round Robin"

    int origin = 0;
    while (1) {

#pragma region "Recieving Process Data"
      Gen_Sched_RCV_VAL = msgrcv(Gen_Sched_MSGQ, &RecievedProcess,
                                 sizeof(struct processMsgBuff), 0, 0);
      if (Gen_Sched_RCV_VAL == -1) {
        perror("Error recieving process message");
        exit(-1);
      }
      if (Gen_Sched_RCV_VAL != 0) {
        int PID = fork();
        if (PID == 0) {
          char strrunTime[6];
          sprintf(strrunTime, "%d", RecievedProcess.process.runTime);
          char *processargs[] = {"./process.out", strrunTime, NULL};
          execv("process.out", processargs);
        };

        kill(PID, SIGSTOP);
        RecievedProcess.process.pid = PID;
        insert_RR_priQ(pq, RecievedProcess.process);
      }
#pragma endregion

#pragma region "Round Robin Implementation"
      x = getClk();
      // If time passed is the quantum or if time is less than quantum, then
      // continue/start process
      if (pq->head != NULL && x >= origin + quantum || x < quantum) {
        origin += 5;
        if (runningProcess.pid != -1) {
          kill(runningProcess.pid, SIGSTOP);
          insert_RR_priQ(pq, runningProcess);
        }
        runningProcess = extract_highestpri(pq);
        kill(runningProcess.pid, SIGCONT);
      }

      if (processTerminate == 1) {
        if (pq->head != NULL) {
          runningProcess = extract_highestpri(pq);
          kill(runningProcess.pid, SIGCONT);
        } else {
          runningProcess.pid = -1;
        }
        processTerminate = 0;
      }

#pragma endregion
    }
#pragma endregion
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
  while (runningProcess.pid != -1) {
    runningProcess = extract_highestpri(pq);
    kill(runningProcess.pid, SIGINT);
  }

  exit(0);
}

void handler_SIGCHILD(int signal) {
  processTerminate = 1;
  /*numberChildren -= 1;*/
  exit(0);
}
