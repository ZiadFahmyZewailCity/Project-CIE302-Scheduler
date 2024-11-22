#include "headers.h"

void clearResources(int);

int main(int argc, char *argv[]) {
  signal(SIGINT, clearResources);
// TODO Initialization
// 1. Read the input files.
#pragma region "File Input"
  int count_processes;
  struct processData *processDataList = load("processes.txt", count_processes);
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

#pragma region "Scheduler Process "
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
  // 5. Create a data structure for processes and provide it with its
  // parameters.
  // 6. Send the information to the scheduler at the appropriate time.
  // 7. Clear clock resources
  destroyClk(true);
}

void clearResources(int signum) {
  // TODO Clears all resources in case of interruption
}
