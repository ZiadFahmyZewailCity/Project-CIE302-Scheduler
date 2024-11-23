#include "headers.h"

#pragma region "Static variables needed to clear resources"
enum schedulingAlgorithm alg;

struct PriorityQueue *pq;

struct processData runningProcess;

int Terminating_Process_MSGQ;

void clearResources(int signum);

#pragma endregion

#pragma region "SIGCHLD Handler things"
// Flag used to notify scheduler that the process it holds has been terminated
int processTerminate = 0;
int processTerminateSJF = 1;

// Number of children <<PROBABLY UNNECESSARY>>
/*int numberChildren = 0;*/

// When processes terminate they will notify the scheduler from here
void handler_SIGCHILD(int signal);
#pragma endregion

int main(int argc, char *argv[]) {
  signal(SIGINT, clearResources);
  initClk();
  int x = getClk();

  alg = atoi(argv[1]);
  unsigned int quantum = atoi(argv[2]);

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
  int Terminating_Process_RCV_VAL;
  Terminating_Process_Key = ftok("Terminating_Processes_KeyFile", 2);
  Terminating_Process_MSGQ = msgget(Terminating_Process_Key, IPC_CREAT | 0666);
  if (Terminating_Process_MSGQ == -1) {
    perror("Error in Creating Generator to Scheduler Message Queue");
    exit(-1);
  }
#pragma endregion

  struct processMsgBuff RecievedProcess;
  runningProcess.pid = -1;
  struct processFinalInfo Process_Info;
#pragma endregion

  PriorityQueue *pq = initialize_priQ();


    
    p_out = fopen("check.txt", "w");
    fprintf(p_out, "# At \ttime x \tprocess y \tstate arr w \ttotal z \tremain y \twait k\n");
    fclose(p_out);

  // TODO implement the scheduler :)
  switch (alg) {
  case SJF:

  int currentNumberProccess = 0;
    while(1)
{
    X = getClk();
    //Will add arrving messages to prioQUEUE
    Gen_Sched_RCV_VAL = msgrcv(Gen_Sched_MSGQ, &RecievedProcess,sizeof(struct processMsgBuff), 0, 0);
    if (Gen_Sched_RCV_VAL != -1 && Gen_Sched_RCV_VAL != 0)
    {
        int PID = fork();
        if (PID == 0)
        {
            
            char strrunTime[6];
            char strid[6];
            sprintf(strrunTime, "%d", RecievedProcess.process.runTime);
            sprintf(strid, "%d", RecievedProcess.process.id);
            char *processargs[] = {"./process.out", strrunTime, strid, NULL};
            execv("process.out", processargs);
                
        }
        if (PID == 1)
        {
            currentNumberProccess += 1;
            kill(SIGSTP,PID);
        }
    } 
    //will only send once the one process before has terminated 
    if(processTerminateSJF == 1 || currentNumberProccess = 0)
    {
        if (currentNumberProccess > 0)
        {
            currentNumberProccess -= 1;
            processTerminateSJF = 0;
            highestprio = extract_highestpri(pq);
            kill(SIGCONT,highestprio.pid);
        }
     }  
}

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
      //This variable is a place hold for the remaining time parameter that will be recived form process  
      int remainingTime = 5;
      x = getClk();
      // If time passed is the quantum or if time is less than quantum, then
      // continue/start process
      if (pq->head != NULL && x >= origin + quantum || x < quantum) {
        origin += 5;
        if (runningProcess.pid != -1) {
          kill(runningProcess.pid, SIGUSR1);

          //NOT SURE WHAT ROUNDROBIN IS DOING HERE BUT CHANGE OF STATE OCCURS SHOULD BE OUTPUTED
          output(runningProcess,remainingTime,x);

          insert_RR_priQ(pq, runningProcess);
        }
        runningProcess = extract_highestpri(pq);
        kill(runningProcess.pid, SIGCONT);

        //Outputs the data when process continues or starts
        output(runningProcess,remainingTime,x);
      }

      if (processTerminate == 1) {
        if (pq->head != NULL) {
          runningProcess = extract_highestpri(pq);

          //NOT SURE WHAT ROUNDROBIN IS DOING HERE BUT CHANGE OF STATE OCCURS SHOULD BE OUTPUTED
          output(runningProcess,remainingTime,x);

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

  // upon termination release the clock resources

  destroyClk(false);

}

#pragma region "Signal Handler Definitions"

void clearResources(int signum) {
  // TODO Clears all resources in case of interruption
  printf("Clearing scheduler resources...\n");
  struct msqid_ds temp;
  msgctl(Terminating_Process_MSGQ, IPC_RMID, &temp);
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
  processTerminateSJF = 1;
  /*numberChildren -= 1;*/
  return;
}

#pragma endregion
