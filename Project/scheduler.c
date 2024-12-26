#include "headers.h"

#pragma region "Static variables needed to clear resources"
enum schedulingAlgorithm alg;

struct PriorityQueue *pq;

struct processData runningProcess;

struct processStateInfo current_process_info;
struct processStateInfoMsgBuff current_process_msg;

int Terminating_Process_MSGQ;
int Terminating_Process_RCV_VAL;

struct processStateInfo *ProcessTable;

int x;

void clearResources(int signum);

#pragma endregion

#pragma region "SIGCHLD Handler things"

// Flag used to notify scheduler that the process it holds has been terminated
int processTerminate = 0;
int proccessRunningSJF = 0;
int terminatedProcessId = -2;

int numberFinishedProcesses = 0;

// round robin origin
int origin = 0;

// Number of children <<PROBABLY UNNECESSARY>>
/*int numberChildren = 0;*/

// When processes terminate they will notify the scheduler from here
void handler_SIGCHILD(int signal);

#pragma endregion

bool processAllocated = 0;
bool processWaitingFlag = 0;

struct memBlock *allocatedMemoryBlocks;
struct memBlock *emptyMemoryBlocks;

int main(int argc, char *argv[]) {
#pragma region "initialization"
  signal(SIGINT, clearResources);
  signal(SIGUSR2, handler_SIGCHILD);
  initClk();
  x = getClk();

  alg = atoi(argv[1]);
  unsigned int quantum = atoi(argv[2]);
  unsigned int countProcesses = atoi(argv[3]);

#pragma endregion

  emptyMemoryBlocks = (struct memBlock *)malloc(sizeof(struct memBlock));
  emptyMemoryBlocks->starts = 0;
  emptyMemoryBlocks->PID = -1;
  emptyMemoryBlocks->size = 1024;
  emptyMemoryBlocks->next = NULL;

  ProcessTable = (struct processStateInfo *)malloc(
      countProcesses * sizeof(struct processStateInfo));

#pragma region "Initializing message queue"
#pragma region "Generator to Scheduler Message Queue"
  key_t Gen_Sched_Key;
  int Gen_Sched_RCV_VAL;
  int Gen_Sched_MSGQ;
  Gen_Sched_Key = ftok("Gen_Sched_KeyFile", 1);
  Gen_Sched_MSGQ = msgget(Gen_Sched_Key, IPC_CREAT | 0666);
  if (Gen_Sched_MSGQ == -1) {
    perror("Error in Creating Generator to Scheduler Message Queue");
    exit(-1);
  }
#pragma endregion

#pragma region "Terminiating Process Message Queue"
  key_t Terminating_Process_Key;
  Terminating_Process_Key = ftok("Terminating_Processes_KeyFile", 2);
  Terminating_Process_MSGQ = msgget(Terminating_Process_Key, IPC_CREAT | 0666);
  if (Terminating_Process_MSGQ == -1) {
    perror("Error in Creating Generator to Scheduler Message Queue");
    exit(-1);
  }
#pragma endregion

  struct processMsgBuff RecievedProcess;
  runningProcess.pid = -1;
  struct processStateInfoMsgBuff Process_Info;
#pragma endregion

  PriorityQueue *pq = initialize_priQ();
#pragma region "output init"
  p_out = fopen("Scheduler.log", "w");
  fprintf(p_out, "# At \ttime x \tprocess y \tstate arr w \ttotal z \tremain y "
                 "\twait k\n");
  fclose(p_out);
#pragma endregion
  p_out = fopen("memory.log", "w");
  fprintf(p_out,
          "# At \ttime x \tallocated y \tbytes for process z \tfrom i to j\n");
  fclose(p_out);

  // printf("I have reached before the switchcase");

  // TODO implement the scheduler :)
  printf("algo = %d\n", alg);
  switch (alg) {
  case SJF: {
    int currentNumberProccess = 0;
    while (numberFinishedProcesses < countProcesses) {

      x = getClk();
      if (!processWaitingFlag) {

        Gen_Sched_RCV_VAL =
            msgrcv(Gen_Sched_MSGQ, &RecievedProcess,
                   sizeof(struct processMsgBuff) - sizeof(long), 0, IPC_NOWAIT);
        if (Gen_Sched_RCV_VAL != -1 && Gen_Sched_RCV_VAL != 0) {
          printf("Recieved a Process! %d\n\n\n", RecievedProcess.process.id);

          processWaitingFlag = 1;
          processAllocated = memoryAllocate(
              RecievedProcess.process.memsize, RecievedProcess.process.id,
              &allocatedMemoryBlocks, &emptyMemoryBlocks);
        }
      } else
        {
          if (processAllocated) {

            int PID = fork();

            if (PID == 0) {
              char strrunTime[6];
              char strid[6];
              char strarrivalTime[6];
              sprintf(strrunTime, "%d", RecievedProcess.process.runTime);
              sprintf(strid, "%d", RecievedProcess.process.id);
              sprintf(strarrivalTime, "%d",
                      RecievedProcess.process.arrivalTime);
              char *processargs[] = {"./process.out", strrunTime, strid,
                                     strarrivalTime, NULL};
              execv("process.out", processargs);
              perror("Execv failed!\n");
            }
            printf("Child Fork! %d\n\n\n", RecievedProcess.process.id);
            // Parent process (scheduler)
            RecievedProcess.process.pid = PID;

            // Finding the allocated memory block
            struct memBlock *tempMemoryBlock = allocatedMemoryBlocks;
            while (tempMemoryBlock->PID != RecievedProcess.process.id) {
              tempMemoryBlock = tempMemoryBlock->next;
            }

#pragma region "Initializing some variables for process in the process table"

            ProcessTable[RecievedProcess.process.id - 1].arrivalTime =
                RecievedProcess.process.arrivalTime;
            ProcessTable[RecievedProcess.process.id - 1].runTime =
                RecievedProcess.process.runTime;
            ProcessTable[RecievedProcess.process.id - 1].remainingTime =
                RecievedProcess.process.runTime;
            ProcessTable[RecievedProcess.process.id - 1].id =
                RecievedProcess.process.id;

            ProcessTable[RecievedProcess.process.id - 1].memSize =
                RecievedProcess.process.memsize;
            ProcessTable[RecievedProcess.process.id - 1].offset =
                tempMemoryBlock->starts;

#pragma endregion

            RecievedProcess.process.pid = PID;

            currentNumberProccess += 1;
            printf("Current number proccess section 1 = %d\n",
                   currentNumberProccess);

            // Adding process to queue
            insert_SJF_priQ(pq, RecievedProcess.process);

            // Stopping proccess that has just been forked
            kill(PID, SIGSTOP);

            outputMEM(ProcessTable[RecievedProcess.process.id - 1], x,
                      ALLOCATE);

            processWaitingFlag = 0;
            processAllocated = 0;

          } else {
            processAllocated = memoryAllocate(
                RecievedProcess.process.memsize, RecievedProcess.process.id,
                &allocatedMemoryBlocks, &emptyMemoryBlocks);
          }
        }

      if (proccessRunningSJF == 0) {
        struct processData highestprio = extract_highestpri(pq);
        if (highestprio.pid != -1) {
          printf("Process Found!\n");
          currentNumberProccess -= 1;

          proccessRunningSJF = 1;

          kill(highestprio.pid, SIGCONT);

          struct processStateInfo proccesRunning =
              ProcessTable[highestprio.id - 1];

          output(proccesRunning, x, running);
        }
      }
    }
  } break;
  case PHPF:
#pragma region "Preemptive HPF"
    // signal()
    {
      {
        int currentNumberProcess = 0;
        int processesCompleted = 0;

        // Main scheduling loop
        while (processesCompleted < countProcesses) {
          x = getClk(); // Get current system time

          // PART 1: HANDLE NEW PROCESS ARRIVALS
          int Gen_Sched_RCV_VAL =
              msgrcv(Gen_Sched_MSGQ, &RecievedProcess,
                     sizeof(struct processMsgBuff), 0, IPC_NOWAIT);

          // If a new process was received
          if (Gen_Sched_RCV_VAL != -1) {
            int PID = fork();

            if (PID == -1) {
              perror("Fork failed");
              exit(-1);
            } else if (PID == 0) {
              // Child process: prepare arguments for process execution
              char strrunTime[6];
              char strid[6];
              char strarrivalTime[6];
              sprintf(strrunTime, "%d", RecievedProcess.process.runTime);
              sprintf(strid, "%d", RecievedProcess.process.id);
              sprintf(strarrivalTime, "%d",
                      RecievedProcess.process.arrivalTime);

              char *processargs[] = {"./process.out", strrunTime, strid,
                                     strarrivalTime, NULL};
              execv("./process.out", processargs);
            }

            else {
              // Parent process (scheduler)
              RecievedProcess.process.pid = PID;

              // Initialize process state information
              struct processStateInfo processInfo;
              processInfo.id = RecievedProcess.process.id;
              processInfo.arrivalTime = RecievedProcess.process.arrivalTime;
              processInfo.runTime = RecievedProcess.process.runTime;
              processInfo.remainingTime = RecievedProcess.process.runTime;

              // Store process information in process table
              ProcessTable[RecievedProcess.process.id - 1] = processInfo;

              // Add process to priority queue
              insert_PHPF_priQ(pq, RecievedProcess.process);

              // Log process arrival
              output(processInfo, x, waiting);

              currentNumberProcess++;
            }
            kill(PID, SIGSTOP);
            printf("process Stopped\n");
          }
          // PART 2: HANDLE PROCESS TERMINATION

          if (processTerminate) {
            currentNumberProcess--;
            processesCompleted++;
            runningProcess.pid = -1;
            // output(terminatedProcess, x, 0);
            processTerminate = 0;
          }

          // struct processStateInfoMsgBuff terminatedProcessMsg;
          // int termination_result = msgrcv(Terminating_Process_MSGQ,
          //                                 &terminatedProcessMsg,
          //                                 sizeof(struct
          //                                 processStateInfoMsgBuff) -
          //                                 sizeof(long), 0, IPC_NOWAIT);
          //
          // if (termination_result != -1) {
          //   printf("TERMINATOR!\n");
          //   // Process has terminated
          //   struct processStateInfo terminatedProcess =
          //   terminatedProcessMsg.processState;
          //
          //   // Update process table
          //   ProcessTable[terminatedProcess.id - 1] = terminatedProcess;
          //
          //   // Log process completion
          //   printf("x: %d\n", x);
          //   // output(terminatedProcess, x, 0);
          //
          //   printf("process Table with salt\n");
          //
          //   // Decrement active process count
          //   currentNumberProcess--;
          //   processesCompleted++;
          //   printf("numberProcess: %d\t processesCompleted: %d\n",
          //   currentNumberProcess, processesCompleted);
          //
          //   // Reset running process
          //   runningProcess.pid = -1;
          //   printf("pq Head: %p,\tpid: %d\tpriority: %d\tid: %d\n", pq->head,
          //   runningProcess.pid, runningProcess.priority, runningProcess.id);
          // }
          // PART 3: SCHEDULE NEXT PROCESS
          if (pq->head != NULL &&
              (runningProcess.pid == -1 ||
               pq->head->process.priority < runningProcess.priority)) {

            printf("Scheduling!\n");
            // Preempt current running process if needed
            if (runningProcess.pid != -1) {
              kill(runningProcess.pid, SIGSTOP);
              output(ProcessTable[runningProcess.id - 1], x, waiting);
              insert_PHPF_priQ(pq, runningProcess);
            }

            // Get highest priority process
            runningProcess = extract_highestpri(pq);

            // Update process state to running
            output(ProcessTable[runningProcess.id - 1], x, running);

            // Start/resume the process
            kill(runningProcess.pid, SIGCONT);
          } else {
            printf("still with the same process!\n");
            printf("pq Head: %p,\tpid: %d\tpriority: %d\tid: %d\n", pq->head,
                   runningProcess.pid, runningProcess.priority,
                   runningProcess.id);
          }
        }

        // Clean up
        cleanup_priQ(pq);
        destroyClk(0);
        break;
      }
#pragma endregion
    }
  case RR:
#pragma region "Round Robin"

    while (numberFinishedProcesses < countProcesses) {

#pragma region "Recieving Process Data"

      if(!processWaitingFlag){

      // Check message queue for processes
      Gen_Sched_RCV_VAL =
          msgrcv(Gen_Sched_MSGQ, &RecievedProcess,
                 sizeof(struct processMsgBuff) - sizeof(long), 0, IPC_NOWAIT);

      // If process found, generate process then add it to queue
      if (Gen_Sched_RCV_VAL != -1 && Gen_Sched_RCV_VAL != 0) {
          processWaitingFlag = 1;
          processAllocated = memoryAllocate(
              RecievedProcess.process.memsize, RecievedProcess.process.id,
              &allocatedMemoryBlocks, &emptyMemoryBlocks);
      }
      }else{
        if(processAllocated){


        int PID = fork();
        if (PID == 0) {
          char strrunTime[6];
          char strid[6];
          char strarrivalTime[6];
          sprintf(strrunTime, "%d", RecievedProcess.process.runTime);
          sprintf(strid, "%d", RecievedProcess.process.id);
          sprintf(strarrivalTime, "%d", RecievedProcess.process.arrivalTime);
          char *processargs[] = {"./process.out", strrunTime, strid,
                                 strarrivalTime, NULL};
          execv("process.out", processargs);
        }
        kill(PID, SIGSTOP);

            // Finding the allocated memory block
            struct memBlock *tempMemoryBlock = allocatedMemoryBlocks;
            while (tempMemoryBlock->PID != RecievedProcess.process.id) {
              tempMemoryBlock = tempMemoryBlock->next;
            }

        // Initializing some variables for process in the process table
        ProcessTable[RecievedProcess.process.id - 1].arrivalTime =
            RecievedProcess.process.arrivalTime;
        ProcessTable[RecievedProcess.process.id - 1].runTime =
            RecievedProcess.process.runTime;
        ProcessTable[RecievedProcess.process.id - 1].remainingTime =
            RecievedProcess.process.runTime;
        ProcessTable[RecievedProcess.process.id - 1].id =
            RecievedProcess.process.id;

        ProcessTable[RecievedProcess.process.id - 1].memSize =
            RecievedProcess.process.memsize;
        ProcessTable[RecievedProcess.process.id - 1].offset =
            tempMemoryBlock->starts;

        // Adding process to queue
        RecievedProcess.process.pid = PID;
        insert_RR_priQ(pq, RecievedProcess.process);



            outputMEM(ProcessTable[RecievedProcess.process.id - 1], x,
                      ALLOCATE);

            processWaitingFlag = 0;
            processAllocated = 0;
      }else{
            processAllocated = memoryAllocate(
                RecievedProcess.process.memsize, RecievedProcess.process.id,
                &allocatedMemoryBlocks, &emptyMemoryBlocks);
      }
      }
#pragma endregion

#pragma region "Round Robin Implementation"
      // This variable is a place hold for the remaining time parameter that
      // will be recived form process
      x = getClk();

      // If current process is terminating, remove the current process and
      // switch state
      if (processTerminate == 1 && terminatedProcessId == runningProcess.id) {
        /*ProcessTable[current_process_info.id - 1] = current_process_info;*/
        // If there's something in queue, then set running process to the
        // extracted process from the queue
        if (pq->head != NULL) {
          runningProcess = extract_highestpri(pq);

          // NOT SURE WHAT ROUNDROBIN IS DOING HERE BUT CHANGE OF STATE OCCURS
          // SHOULD BE OUTPUTED
          current_process_info = ProcessTable[runningProcess.id - 1];
          output(current_process_info, x, running);

          kill(runningProcess.pid, SIGCONT);
        }

        // If there's nothing in queue, then set running pid to -1 so the
        // scheduler knows there is no current running process
        else {
          runningProcess.pid = -1;
        }

        // set flag back to 0
        processTerminate = 0;
      }

      // Instead, if time passed is the quantum, or if time is less than
      // quantum, or if there is no current running process then continue/start
      // the new process if there is a waiting process in the queue
      else if (pq->head != NULL &&
               (x >= (origin + quantum) || runningProcess.pid == -1)) {
        origin = x;
        // If runningProcess pid is not -1, that means there is a current
        // running process, so terminate current running process and put back
        // into queue
        if (runningProcess.pid != -1) {
          // stop current process
          kill(runningProcess.pid, SIGUSR1);
          // recieve process info
          Terminating_Process_RCV_VAL =
              msgrcv(Terminating_Process_MSGQ, &current_process_msg,
                     sizeof(struct processStateInfoMsgBuff) - sizeof(long), 0,
                     !IPC_NOWAIT);
          // store process info in process table
          printf("about to take info\n");

          int memsize=ProcessTable[runningProcess.id - 1].memSize;
          int offset=ProcessTable[runningProcess.id - 1].offset;
          printf("memsize here is %d\n",memsize);
          ProcessTable[runningProcess.id - 1] =
              current_process_msg.processState;
          ProcessTable[runningProcess.id - 1].memSize=memsize;
          ProcessTable[runningProcess.id - 1].offset=offset;

          // NOT SURE WHAT ROUNDROBIN IS DOING HERE BUT CHANGE OF STATE OCCURS
          // SHOULD BE OUTPUTED
          printf("about to output\n");
          output(current_process_msg.processState, x, waiting);

          // put process back into queue
          insert_RR_priQ(pq, runningProcess);
          // output the state of current running process
          //
        }
        runningProcess = extract_highestpri(pq);

        kill(runningProcess.pid, SIGCONT);
        current_process_info = ProcessTable[runningProcess.id - 1];

        // Outputs the data when process continues or starts
        output(current_process_info, x, running);
      }
    }
#pragma endregion
#pragma endregion
    break;
  }

  outputStats(ProcessTable, countProcesses, x);

  // upon termination release the clock resources

  destroyClk(0);

  raise(SIGINT);
}

#pragma region "Signal Handler Definitions"

void clearResources(int signum) {
  // TODO Clears all resources in case of interruption
  printf("Clearing scheduler resources...\n");
  msgctl(Terminating_Process_MSGQ, IPC_RMID, NULL);
  /*kill(SchedulerPID, SIGINT);*/
  // Incomplete
  struct memBlock *temp=emptyMemoryBlocks;
  while(temp!=NULL){
    emptyMemoryBlocks=temp;
    temp=emptyMemoryBlocks->next;
    free(emptyMemoryBlocks);
  }
  temp=allocatedMemoryBlocks;
  while(temp!=NULL){
    allocatedMemoryBlocks=temp;
    temp=allocatedMemoryBlocks->next;
    free(allocatedMemoryBlocks);
  }
  while (runningProcess.pid != -1) {
    runningProcess = extract_highestpri(pq);
    kill(runningProcess.pid, SIGINT);
  }

  exit(0);
}

void handler_SIGCHILD(int signal) {
  processTerminate = 1;
  proccessRunningSJF = 0;
  /*numberChildren -= 1;*/
  numberFinishedProcesses++;
  origin = x;

  printf("process terminating...\n");

  struct processStateInfoMsgBuff finishedProcessState;

  Terminating_Process_RCV_VAL = msgrcv(
      Terminating_Process_MSGQ, &finishedProcessState,
      sizeof(struct processStateInfoMsgBuff) - sizeof(long), 0, !IPC_NOWAIT);
  if (Terminating_Process_RCV_VAL == -1) {
    perror("Couldn't recieve final process state. ");
  };
  terminatedProcessId = finishedProcessState.processState.id;

      printf("about to deallocate\n");
  memoryDeallocate(ProcessTable[finishedProcessState.processState.id - 1].id,
                   &allocatedMemoryBlocks, &emptyMemoryBlocks);
      printf("deallocated\n");

  outputMEM(ProcessTable[finishedProcessState.processState.id - 1], x, FREE);
      printf("printed\n");

  struct memBlock* temp=allocatedMemoryBlocks;
  int number=1;
  while(temp!=NULL){
    number++;
    temp=temp->next;
  };

  printf("There is %d\n\n",number);

  output(finishedProcessState.processState, x, waiting);

  ProcessTable[finishedProcessState.processState.id - 1] =
      finishedProcessState.processState;
  return;
}
#pragma endregion
