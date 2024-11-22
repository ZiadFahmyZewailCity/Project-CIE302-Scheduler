#include "headers.h"

#pragma region Preprocessing

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


int main(int argc, char * argv[])
{
    initClk();
    
    //TODO implement the scheduler :)
    //upon termination release the clock resources
    PriorityQueue *pq = init_priQ();
    key_t key =ftok("scheduler", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if(msgid ==-1){
        perror("Failed to create message queue");
        exit(EXIT_FAILURE);
    }
    process_messages(pq, msgid);
    while(wait(NULL)>0);
    cleanup_priQ(pq);
    
    
    destroyClk(true);
}