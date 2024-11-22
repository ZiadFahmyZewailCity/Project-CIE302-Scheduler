#include "headers.h"
#include <stdio.h>

void clearResources(int);
int Gen_Sched_MSGQ;

int main(int argc, char *argv[]) {
  signal(SIGINT, clearResources);
// TODO Initialization
// 1. Read the input files.
#pragma region "File Input"
  // 5. Create a data structure for processes and provide it with its
  // parameters.
  int count_processes;
  struct processData *processDataList = load("processes.txt", &count_processes);
#pragma endregion

// 2. Ask the user for the chosen scheduling algorithm and its parameters, if
// there are any.
#pragma region "User Input"
  printf("1. SJF: Shortest Job First\n"
         "2. PHPF: Preemptive Highest Priority First\n"
         "3. RR: Round Robin\n\n"
         "Choose the Scheduling Algorithm: ");

  enum schedulingAlgorithm schedulingAlg;
  unsigned int quantum = 0;

  scanf("%u", &schedulingAlg);

  while (schedulingAlg != SJF && schedulingAlg != PHPF && schedulingAlg != RR) {
    printf("Please select a valid option.\n"
           "1. SJF: Shortest Job First\n"
           "2. PHPF: Preemptive Highest Priority First\n"
           "3. RR: Round Robin\n\n"
           "Choose the Scheduling Algorithm: ");
    scanf("%u", &schedulingAlg);
  }

  if (schedulingAlg == RR) {
    printf("Type the Quantum length: ");
    scanf("%u", &quantum);
  }
#pragma endregion

  // 3. Initiate and create the scheduler and clock processes.
#pragma region "Clock Process"
  int pid = fork();
  if (pid == -1) {
    perror("Failure in forking");
  }

  if (pid == 0) {
    char *args[] = {"./clk.out", NULL};
    execv(args[0], args);
  };
#pragma endregion

#pragma region "Message Queue and Scheduler Process Creation"
  // Message Queue Creation
  key_t Gen_Sched_Key;
  int Gen_Sched_SND_VAL;
  Gen_Sched_Key = ftok("Gen_Sched_KeyFile", 1);
  Gen_Sched_MSGQ = msgget(Gen_Sched_Key, IPC_CREAT | 0666);
  if (Gen_Sched_MSGQ == -1) {
    perror("Error in Creating Generator to Scheduler Message Queue");
    exit(-1);
  }

  // Process creation
  pid = fork();
  if (pid == -1) {
    perror("Failure in forking");
  }
  //
  if (pid == 0) {
    char strSchedulingAlg[5];
    char strQuantum[5];
    sprintf(strSchedulingAlg, "%d", schedulingAlg);
    sprintf(strQuantum, "%d", quantum);
    char *args[] = {"./scheduler.out", strSchedulingAlg, strQuantum, NULL};
    execv(args[0], args);
  };
#pragma endregion

  // 4. Use this function after creating the clock process to initialize clock
  initClk();
  // To get time use this
  int x = getClk();
  printf("current time is %d\n", x);

  // TODO Generation Main Loop
  int i = 0;
  struct processMsgBuff message;

  // 6. Send the information to the scheduler at the appropriate time.
  while (i < count_processes) {
    x = getClk();
    if (x >= processDataList[i].arrivalTime) {
      message.process = processDataList[i];
      Gen_Sched_SND_VAL =
          msgsnd(Gen_Sched_MSGQ, &message, sizeof(message), !IPC_NOWAIT);
      i++;
    } else {
      sleep(1);
    }
  }

  // 7. Clear clock resources
  destroyClk(true);
}

void clearResources(int signum) {
  // TODO Clears all resources in case of interruption
  printf("Clearing resources...\n");
  struct msqid_ds temp;
  msgctl(Gen_Sched_MSGQ, IPC_RMID, &temp);
  destroyClk(true);
  // Incomplete

  exit(0);
}