#include "headers.h"

#pragma region "Static variables needed to clear resources"
enum schedulingAlgorithm alg;

struct PriorityQueue *pq;

struct processData runningProcess;

struct processStateInfoMsgBuff current_process_info;

int Terminating_Process_MSGQ;
int Terminating_Process_RCV_VAL;

int x;

void clearResources(int signum);

#pragma endregion

#pragma region "SIGCHLD Handler things"

// Flag used to notify scheduler that the process it holds has been terminated
int processTerminate = 0;

// Number of children <<PROBABLY UNNECESSARY>>
/*int numberChildren = 0;*/

// When processes terminate they will notify the scheduler from here
void handler_SIGCHILD(int signal);

#pragma endregion

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
  Terminating_Process_Key = ftok("Term_Proc_KeyFile", 1);
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
  p_out = fopen("check.txt", "w");
  fprintf(p_out, "# At \ttime x \tprocess y \tstate arr w \ttotal z \tremain y "
                 "\twait k\n");
  fclose(p_out);
#pragma endregion

  // TODO implement the scheduler :)
  switch (alg) {
  case SJF:

    break;
  case PHPF:

    break;
  case RR:
#pragma region "Round Robin"

    int origin = 0;
    while (1) {

          printf(" here\n");
#pragma region "Recieving Process Data"
      // Check message queue for processes
      Gen_Sched_RCV_VAL = msgrcv(Gen_Sched_MSGQ, &RecievedProcess,
                                 sizeof(struct processMsgBuff), 0, IPC_NOWAIT);

      // If process found, generate process then add it to queue
      if (Gen_Sched_RCV_VAL != -1 && Gen_Sched_RCV_VAL != 0) {
                     printf("recieved new proc\n\n");
        int PID = fork();
        if (PID == 0) {
          char strrunTime[6];
          char strid[6];
          sprintf(strrunTime, "%d", RecievedProcess.process.runTime);
          sprintf(strid, "%d", RecievedProcess.process.id);
          char *processargs[] = {"./process.out", strrunTime, strid, NULL};
          execv("process.out", processargs);
        };
        /*kill(PID, SIGSTOP);*/

        // Initializing some variables for process in the process table
        // Adding process to queue
        RecievedProcess.process.pid = PID;
        insert_RR_priQ(pq, RecievedProcess.process);
      }
#pragma endregion

#pragma region "Round Robin Implementation"
      // This variable is a place hold for the remaining time parameter that
      // will be recived form process
      x = getClk();

          printf("entered alg\n");
      // If current process is terminating, remove the current process and
      // switch state
      //sleep(1);
      if (processTerminate == 1) {
          printf("entered here\n");
        /*ProcessTable[current_process_info.id - 1] = current_process_info;*/
        // If there's something in queue, then set running process to the
        // extracted process from the queue
        if (pq->head != NULL) {
          runningProcess = extract_highestpri(pq);
          printf("im here\n");

          // NOT SURE WHAT ROUNDROBIN IS DOING HERE BUT CHANGE OF STATE OCCURS
          // SHOULD BE OUTPUTED
          //current_process_info = ProcessTable[runningProcess.id - 1];
          //current_process_info.pstate = running;
          //output(current_process_info, x);

          //kill(runningProcess.pid, SIGCONT);
        }

        // If there's nothing in queue, then set running pid to -1 so the
        // scheduler knows there is no current running process
        else {
          runningProcess.pid = -1;
          printf("im hereagaint\n");
        }

          printf("not terminated\n");
        // set flag back to 0
          printf("im here3[\n");
        processTerminate = 0;
      }

      // Instead, if time passed is the quantum, or if time is less than
      // quantum, or if there is no current running process then continue/start
      // the new process if there is a waiting process in the queue
      else if (pq->head != NULL &&
               (x >= (origin + quantum) || runningProcess.pid == -1)) {
                     printf("hereeeeeeeeeeee\n\n");
        origin = x;
        // If runningProcess pid is not -1, that means there is a current
        // running process, so terminate current running process and put back
        // into queue
        if (runningProcess.pid != -1) {
          // stop current process
          //kill(runningProcess.pid, SIGUSR1);
          
                     printf("signal ended\n\n");
          // recieve process info
          //Terminating_Process_RCV_VAL =
             // msgrcv(Terminating_Process_MSGQ, &current_process_info,
               //      sizeof(struct processStateInfoMsgBuff), 0, !IPC_NOWAIT);
           //          printf("recieved\n\n");
          // store process info in process table
          //ProcessTable[runningProcess.id - 1] = current_process_info;

          // NOT SURE WHAT ROUNDROBIN IS DOING HERE BUT CHANGE OF STATE OCCURS
          // SHOULD BE OUTPUTED
          //output(current_process_info, x);

          // put process back into queue
          insert_RR_priQ(pq, runningProcess);
          // output the state of current running process
          //
        }
        printf("process number %d is now stopping\n",runningProcess.id);
        print_priQ(pq);
        runningProcess = extract_highestpri(pq);
        //kill(runningProcess.pid, SIGCONT);
        printf("process number %d is now running\n",runningProcess.id);
        printf("time is %d \n",x);
        printf("origin is :%d\n",origin);

        //ProcessTable[runningProcess.id - 1].pstate = running;

        kill(runningProcess.pid, SIGCONT);
        //current_process_info = ProcessTable[runningProcess.id - 1];

        // Outputs the data when process continues or starts
        //output(current_process_info, x);
      }
      sleep(1);

#pragma endregion
    }

#pragma endregion
    break;
  }

  // upon termination release the clock resources

  destroyClk(false);
}

#pragma region "Signal Handler Definitions"

void clearResources(int signum) {
  // TODO Clears all resources in case of interruption
  printf("Clearing scheduler resources...\n");
  struct msqid_ds temp;
  msgctl(Terminating_Process_MSGQ, IPC_RMID, NULL);
  /*kill(SchedulerPID, SIGINT);*/
  // Incomplete
  while (runningProcess.pid != -1) {
    kill(runningProcess.pid, SIGINT);
    runningProcess = extract_highestpri(pq);
  }

  exit(0);
}

void handler_SIGCHILD(int signal) {
  processTerminate = 1;
  /*numberChildren -= 1;*/
  struct processStateInfoMsgBuff temp;
  Terminating_Process_RCV_VAL =
      msgrcv(Terminating_Process_MSGQ, &temp,
             sizeof(struct processStateInfoMsgBuff), 2, !IPC_NOWAIT);

  printf("recieved\n\n");

  //printf("Terminated this one process, the remaining time is %d\n",temp.remainingTime);
  //output(temp, x);
  //ProcessTable[temp.id - 1] = temp;
  //printf("Terminated this one process, the remaining time is %d\n",temp.remainingTime);
  return;
}

#pragma endregion
