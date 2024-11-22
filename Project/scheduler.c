#include "headers.h"

#pragma region Preprocessing

//Flag used to notify scheduler that the process it holds has been terminated
int processTerminate = 0;

//Number of children
int numberChildren = 0;

//When processes terminate they will notify the scheduler from here
void handler_SIGCHILD(int signal)
{
    processTerminate = 1;
    numberChildren -= 1;
    exit(0);
}

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

//Defining a constructor for the PQ
PriorityQueue* init_priQ(){
    PriorityQueue *pq = (PriorityQueue *)malloc(sizeof(PriorityQueue));
    pq->head = NULL;
    return pq;
}
//Defining a constructor for the Node 
Node* create_Node(struct processData process){
    Node *new_node= (Node*)malloc(sizeof(Node));
    new_node->process = process;
    new_node->next = NULL;
    return new_node;
}
//Processing messages to create processes
void process_messages (PriorityQueue *pq, int msgid){
    Message message;
    while(1){
        if (msgrcv(msgid, &message, sizeof(struct processData), 0, 0) ==-1){
            perror("Error recieving message");
            exit(EXIT_FAILURE);
        }
        if(message.process.id==0){
            printf("Termination signal received. Exiting...\n");
            break;
        }
        handle_process(pq, message.process);
        print_priQ(pq);
    }
}
//Defining an insertion function for the PQ
void insert_priQ(PriorityQueue *pq, struct processData process){
    Node *new_node = create_node(process);
    if (pq->head == NULL || pq->head->process.priority > process.priority){
        new_node->next = pq->head;
        pq->head = new_node;
    }
    else{
        Node *current = pq->head;
        while(current->next != NULL && current->next->process.priority <= process.priority){
            current = current->next;
        }
        new_node->next=current->next;
        current->next = new_node;
    }
}
//Printing a PQ attributes for reviewing updates in realtime
void print_priQ(PriorityQueue *pq){
    Node* current =pq ->head;
    printf("PriorityQueue:\n");
    while(current !=NULL){
        printf("Process ID: %d, Priority: %d, RunTime: %d\n",current->process.id, current->process.priority, current->process.runTime);
        current = current->next;
    }
}
//Handling the creation of a process
void handle_process(PriorityQueue *pq, struct processData process){
    pid_t pid=fork();

    if(pid==0){
        printf("Process %d (Priority: %d) is running .\n", process.id, process.priority);
        sleep(process.runTime);
        printf("Process %d (Priority: %d) finished .\n", process.id, process.priority);
        exit(0);
    }else if (pid>0){
        printf("Process%d (Priority: %d) forked with PID %d .\n", process.id, process.priority);
        insert_priQ(pq, process);
    }else{
        perror("Fork Failed");
        exit (EXIT_FAILURE);
    }
}
//Cleaning up the PQ after excution
void cleanup_priQ(PriorityQueue *pq){
    Node* current =pq->head;
    while (current != NULL){
        Node *temp =current;
        current =current->next;
        free(temp); 
    }
    free(pq);
}

#pragma endregion


#pragma region PriorityQueue
//Defining priority queue node implementation as a linked list
typedef struct Node {
    struct processData process;
    struct Node *next;
} Node;
//Define the linked-list head for the priority queue
typedef struct {
    Node *head;
} PriorityQueue;

//Defining a constructor for the PQ
PriorityQueue* initialize_priQ(){
    PriorityQueue *pq = (PriorityQueue *)malloc(sizeof(PriorityQueue));
    pq->head = NULL;
    return pq;
}
//Defining a constructor for the Node 
Node* create_Node(struct processData process){
    Node *new_node= (Node*)malloc(sizeof(Node));
    new_node->process = process;
    new_node->next = NULL;
    return new_node;
}
//function for printing all the priority queue
void print_priQ(PriorityQueue *pq){
    Node* current =pq ->head;
    printf("PriorityQueue:\n");
    while(current !=NULL){
        printf("Process ID: %d, Priority: %d, RunTime: %d\n",current->process.id, current->process.priority, current->process.runTime);
        current = current->next;
    }
}
//Cleaning up the PQ after excution
void cleanup_priQ(PriorityQueue *pq){
    Node* current =pq->head;
    while (current != NULL){
        Node *temp =current;
        current =current->next;
        free(temp); 
    }
    free(pq);
}
//Extracting highest priority
struct processData extract_highestpri (PriorityQueue *pq){
    if (pq->head == NULL){
        perror("Priority queue is empty");
        exit(EXIT_FAILURE);
    }
    Node *temp = pq->head;
    struct processData process = temp->process;
    pq->head = pq->head->next;
    free(temp);
    return process;
}
//Inserting one node in the priority queue
void insert_PHPF_priQ(PriorityQueue *pq, struct processData process){
    Node *new_node = create_node(process);
    if (pq->head == NULL || pq->head->process.priority > process.priority){
        new_node->next = pq->head;
        pq->head = new_node;
    }
    else{
        Node *current = pq->head;
        while(current->next != NULL && current->next->process.priority <= process.priority){
            current = current->next;
        }
        new_node->next=current->next;
        current->next = new_node;
    }
}
#pragma endregion
#pragma region Messagequeue

//PHPF process handeling
void handle_PHPF_process(PriorityQueue *pq, struct processData process){
    pid_t pid=fork();
    if(pid==0){
        char strrunTime[6];
        sprintf(strrunTime, "%d", process.runTime);
        char *processargs[]={"./process.out",strrunTime, NULL};
        execv("./process.out",processargs);
    }else if (pid>0){
        printf("Process%d (Priority: %d) forked with PID %d .\n", process.id, process.priority);
        insert_PHPF_priQ(pq, process);
    }else{
        perror("Fork Failed");
        exit (EXIT_FAILURE);
    }
}

//processing each incomming messages holding a process struct to create
void process_messages (PriorityQueue *pq, int msgid){
    Message message;
    while(1){
        if (msgrcv(msgid, &message, sizeof(struct processData), 0, 0) ==-1){
            perror("Error recieving message");
            exit(EXIT_FAILURE);
        }
        if(message.process.id==0){
            printf("Termination signal received. Exiting...\n");
            break;
        }
        handle_process(pq, message.process);
        print_priQ(pq);
    }
}
#pragma endregion
#pragma region Preeptime HPF Algorithm
//Preemptive HPF scheduling algorithm
void PHPF_schedule(PriorityQueue *pq, int msgid, enum schedulingAlgorithm alg, unsigned int quantum){
    struct processData running_process = {0};
    int running_pid =-1;

    while(1){
        Message msg;
        if (msgrcv(msgid, &msg, sizeof(struct processData),0,IPC_NOWAIT != -1)){
            if(msg.process.id == 0){
                printf("Termination signal recieved. Existing...\n");
                if(running_pid>0) kill(running_pid, SIGKILL);
                break;
            } 
            insert_priority_queue(pq, msg.process);
            print_priQ(pq);
            if(running_pid >0 && msg.process.priority<running_process.priority){
                printf("Preempting process %d for process %d .\n", running_process.id,msg.process.id);
                kill(running_pid, SIGSTOP);
                running_process.runTime -= (getClk()- running_process.arrivalTime);
                insert_priQ(pq, running_process);
                running_pid = -1;
            }
        }
        if (running_pid == -1 && pq->head != NULL){
            running_process = extract_highestpri(pq);
            running_pid = fork();
            if(running_pid ==0){
                printf("Process %d (Priority: %d) is running .\n", running_process.id,running_process.priority);
                sleep(running_process.runTime);
                printf("Process %d (Priority: %d) finished .\n", running_process.id, running_process.priority);
                exit(0);
            }else if(running_pid>0){
                printf("Process %d (Priority: %d) forked with PID .\n", running_process.id, running_process.priority, running_pid);
            }else{
                perror("Fork failed");
                exit(EXIT_FAILURE);
            }
        }
       usleep(100000);
    }
}
#pragma endregion


int main(int argc, char * argv[])
{
    enum schedulingAlgorithm alg =atoi(argv[1]);
    unsigned int quantum = atoi(argv[2]);

    //Initialize message queue
    key_t key =ftok("Gen_Sched_KeyFile",1);
    int msgid =msgget(key, 0666);
    if(msgid == -1){
        perror("Failed to access message queue");
        exit(EXIT_FAILURE);
    }

    PriorityQueue *pq =init_priQ();

    initClk();



    Gen_Sched_MSGQ
  switch (alg) {
  case SJF:

    int returnValue;
    struct processData p,highestprio;  
    pid_t pid;
    while(1)
    {
        //Will add arrving messages to prioQUEUE
        returnValue = msgrcv(msgrevGen_Sched_MSGQ,&p,sizeof(p),0,!IPC_NOWAIT);
        if (returnValue = -1){}
        else
        {
            pid = fork();
            if (pid == 0)
            {
                int execv("./process.out",processargs);
                
            }
            if (pid == 1)
            {   
                kill(p.PID,SIGTOP);
                insert_PHPF_priQ(pq,p);   
            }
        }

        //will only send once the one process before has terminated 
        if(flag == 1)
        {
            highestprio = extract_highestpri(pq);
            kill(SIGCONT,highestprio.pid)
            flag = 0;
        }   
 
    }
    break;
  case PHPF:
    break;
  case RR:
    break;    
    destroyClk(true);
}