#include "headers.h"
#include <stdio.h>

#define ID argv[1];
#define PRIORITY argv[2];
#define RUN_TIME argv[3];
#define ARRIVAL argv[4];


/* Modify this file as needed*/
int remainingtime;


int main(int agrc, char * argv[])
{
    size_t pid = fork();

    if(pid == 0)
        execlp("./clk.out", "./clk.out");
    

    initClk();
    
    int x = getClk();
    remainingtime = atoi(argv[1]);
    // it needs to get the remaining time from somewhere
    while (remainingtime > 0)
    {
        if (getClk() != x){
            remainingtime--;
            x = getClk();
            printf("%d\n", remainingtime);
        }
    }
    destroyClk(false);

    

    return 0;
}
