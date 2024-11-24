#include <signal.h>
#include <stdio.h> //if you don't use scanf/printf change this include
#include <stdlib.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300

#pragma region "Process data structure"
enum state { started = 1, waiting = 2, running = 3, finished = 4 };

struct processData {
  unsigned int id;
  unsigned int arrivalTime;
  unsigned int runTime;
  unsigned int priority;
  int pid;
  // enum state pstate;
};

enum schedulingAlgorithm { SJF = 1, PHPF, RR };

struct PCB {
  enum state pstate;
  unsigned int startTime;
  unsigned int remainingTime;
  unsigned int finishTime;
  // unsigned int waitingTime;
};

#pragma endregion

#pragma region "Inter Process Communication"
// Structure for Process message buffer, if any additions are need beyond the
// process data
struct processMsgBuff {
  long mtype;
  struct processData process;
};

struct processStateInfoMsgBuff {
  unsigned int msgType;
  unsigned int id;
  unsigned int startTime;
  unsigned int runTime;
  unsigned int remainingTime;
  unsigned int finishTime;
  enum state pstate;
};
#pragma endregion

#pragma region "Round Robin Algorithm"

#pragma endregion

#pragma region "Priority Queue"

// Defining priority queue node implementation as a linked list
typedef struct Node {
  struct processData process;
  struct Node *next;
} Node;

// Define the linked-list head for the priority queue
typedef struct {
  Node *head;
} PriorityQueue;

// Defining a constructor for the PQ
PriorityQueue *initialize_priQ() {
  PriorityQueue *pq = (PriorityQueue *)malloc(sizeof(PriorityQueue));
  pq->head = NULL;
  return pq;
}

// Defining a constructor for the Node
Node *create_Node(struct processData process) {
  Node *new_node = (Node *)malloc(sizeof(Node));
  new_node->process = process;
  new_node->next = NULL;
  return new_node;
}

// function for printing all the priority queue
void print_priQ(PriorityQueue *pq) {
  Node *current = pq->head;
  printf("PriorityQueue:\n");
  while (current != NULL) {
    printf("Process ID: %d, Priority: %d, RunTime: %d\n", current->process.id,
           current->process.priority, current->process.runTime);
    current = current->next;
  }
}

// Extracting highest priority
struct processData extract_highestpri(PriorityQueue *pq) {
  struct processData process;
  if (pq->head == NULL) {
    process.pid = -1;
    return process;
  }
  Node *temp = pq->head;
  process = temp->process;
  pq->head = pq->head->next;
  free(temp);
  return process;
}

// Cleaning up the PQ after excution
void cleanup_priQ(PriorityQueue *pq) {
  Node *current = pq->head;
  while (current != NULL) {
    Node *temp = current;
    current = current->next;
    free(temp);
  }
  free(pq);
}

// Inserting one node in the priority queue
void insert_PHPF_priQ(PriorityQueue *pq, struct processData process) {
  Node *new_node = create_Node(process);
  if (pq->head == NULL || pq->head->process.priority > process.priority) {
    new_node->next = pq->head;
    pq->head = new_node;
  } else {
    Node *current = pq->head;
    while (current->next != NULL &&
           current->next->process.priority <= process.priority) {
      current = current->next;
    }
    new_node->next = current->next;
    current->next = new_node;
  }
}

void insert_SJF_priQ(PriorityQueue *pq, struct processData process) {
  Node *new_node = create_Node(process);
  if (pq->head == NULL || pq->head->process.runTime > process.runTime) {
    new_node->next = pq->head;
    pq->head = new_node;
  } else {
    Node *current = pq->head;
    while (current->next != NULL &&
           current->next->process.runTime <= process.runTime) {
      current = current->next;
    }
    new_node->next = current->next;
    current->next = new_node;
  }
}

void insert_RR_priQ(PriorityQueue *pq, struct processData process) {
  Node *new_node = create_Node(process);
  if (pq->head == NULL) {
    new_node->next = pq->head;
    pq->head = new_node;
  } else {
    Node *current = pq->head;
    while (current->next != NULL) {
      current = current->next;
    }
    new_node->next = current->next;
    current->next = new_node;
  }
}

#pragma endregion

#pragma region Messagequeue
// PHPF process handeling
void handle_PHPF_process(PriorityQueue *pq, struct processData process) {
  pid_t pid = fork();
  if (pid == 0) {
    char strrunTime[6];
    sprintf(strrunTime, "%d", process.runTime);
    char *processargs[] = {"./process.out", strrunTime, NULL};
    execv("./process.out", processargs);
  } else if (pid > 0) {
    printf("Process%d (Priority: %d) forked with PID %d .\n", process.id,
           process.priority);
    insert_PHPF_priQ(pq, process);
  } else {
    perror("Fork Failed");
    exit(-1);
  }
}
#pragma endregion

// Process Data Loader
struct processData *load(char *inpFileName, int *count_processes) {
  *count_processes = 0;
  struct processData *p_arr_process;

  // Opening File and checking if its been successfully opened
  FILE *p_file = fopen(inpFileName, "r");
  if (p_file == NULL) {
    perror("ERROR HAS OCCURED IN INPUT FILE OPENINING");
    fclose(p_file);
    return NULL;
  }

  // Buffer needed to store content of line (This is needed even we dont need
  // the content while counting the number of processes)
  char buffer[256];

  // skipping past comments line
  fgets(buffer, sizeof(buffer), p_file);

  // itterating through the file to find number of processes
  while (fgets(buffer, sizeof(buffer), p_file) != NULL) {
    *count_processes += 1;
  }

  // once counting is complete we close the file to now read the file data
  fclose(p_file);

  // Dyanmic allocation of array for processes
  p_arr_process = (struct processData *)malloc(*count_processes *
                                               sizeof(struct processData));

  // Reopening input file
  p_file = fopen(inpFileName, "r");
  if (p_file == NULL) {
    perror("ERROR HAS OCCURED IN INPUT FILE OPENINING _2_");
    fclose(p_file);
    return NULL;
  }

  // skipping past comments line
  fgets(buffer, sizeof(buffer), p_file);

  // itterating through file to get data of each
  for (int i = 0; i < *count_processes; i++) {
    if (fscanf(p_file, "%d\t%d\t%d\t%d\t", &p_arr_process[i].id,
               &p_arr_process[i].arrivalTime, &p_arr_process[i].runTime,
               &p_arr_process[i].priority) != 4) {
      printf("Issue reading from file");
    };
  }

  // Close File
  fclose(p_file);

  return p_arr_process;
}

#pragma region outputFunction

FILE *p_out;

void output(struct processData inpProcessData, int remainingTime,
            int currentTime) {
  p_out = fopen("check.txt", "a");
  if (p_out == NULL) {
    perror("ERROR HAS OCCURRED IN OUTPUT FILE OPENING");
    return;
  }

  int wait_time =
      (currentTime - inpProcessData.arrivalTime) - inpProcessData.runTime;

  // This checks if the update is that the process is terminated to print the
  // extra parameters, number should be equal to the enum of terminated status
  if (inpProcessData.pstate == finished) {
    int turnAround = currentTime - inpProcessData.arrivalTime;
    int weightedTurnAround =
        (currentTime - inpProcessData.arrivalTime) / inpProcessData.runTime;

    fprintf(p_out,
            "At \ttime %d \tprocess %d \tfinished\t arr %d \ttotal %d \tremain "
            "%d \twait %d\n \tTA %d \tWTA %d",
            , currentTime, inpProcessData.id, inpProcessData.arrivalTime,
            remainingTime, remainingTime, wait_time, turnAround,
            weightedTurnAround);
  } else {
    switch (inpProcessData.pstate) {
    case started:
      fprintf(p_out,
              "At \ttime %d \tprocess %d \tstarted\t arr %d "
              "\ttotal %d \tremain %d \twait %d\n",
              currentTime, inpProcessData.id, inpProcessData.arrivalTime,
              remainingTime, remainingTime, wait_time);
      break;

    case running:
      fprintf(p_out,
              "At \ttime %d \tprocess %d \tresumed\t arr %d "
              "\ttotal %d \tremain %d \twait %d\n",
              currentTime, inpProcessData.id, inpProcessData.arrivalTime,
              remainingTime, remainingTime, wait_time);

      break;
    case waiting:
      fprintf(p_out,
              "At \ttime %d \tprocess %d \tstopped arr "
              "%d \ttotal %d \tremain %d \twait %d\n",
              currentTime, inpProcessData.id, inpProcessData.arrivalTime,
              remainingTime, remainingTime, wait_time);
      break;

    default:
      break;
    }

    fclose(p_out);
  }
}

#pragma endregion

#pragma region "Clock Stuff"
///==============================
// don't mess with this variable//
int *shmaddr; //
//===============================

int getClk() { return *shmaddr; }

/*
 * All process call this function at the beginning to establish communication
 * between them and the clock module. Again, remember that the clock is only
 * emulation!
 */
void initClk() {
  int shmid = shmget(SHKEY, 4, 0444);
  while ((int)shmid == -1) {
    // Make sure that the clock exists
    printf("Wait! The clock not initialized yet!\n");
    sleep(1);
    shmid = shmget(SHKEY, 4, 0444);
  }
  shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of
 * simulation. It terminates the whole system and releases resources.
 */

void destroyClk(bool terminateAll) {
  shmdt(shmaddr);
  if (terminateAll) {
    killpg(getpgrp(), SIGINT);
  }
}
