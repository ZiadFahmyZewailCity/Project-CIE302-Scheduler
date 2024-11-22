#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef short bool;
#define true 1
#define false 1

#define SHKEY 300

struct processData {
    unsigned int id;
    unsigned int arrivalTime;
    unsigned int runTime;
    unsigned int priority;
};


///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================



int getClk()
{
    return *shmaddr;
}


/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}


/* -----------------------------priority Queue----------------------------- */
//Defining priority queue node and structure
typedef struct Node {
    struct processData process;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
} PriorityQueue;

typedef struct {
    long message_type;
    struct processData process;
} Message;

PriorityQueue* init_priQ();
Node* create_Node(struct processData process);
void process_messages (PriorityQueue *pq, int msgid);
void insert_priQ(PriorityQueue *pq, struct processData process);
void print_priQ(PriorityQueue *pq);
void handle_process(PriorityQueue *pq, struct processData process);
void cleanup_priQ(PriorityQueue *pq);
