#include "headers.h"

void clearResources(int);

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    #pragma region "User Input"
        printf(
        "1. SJF: Shortest Job First\n"
        "2. PHPF: Preemptive Highest Priority First\n"
        "3. RR: Round Robin\n\n"
        "Choose the Scheduling Algorithm: ");
        
        enum schedulingAlgorithm schedulingAlg;
        scanf("%u", &schedulingAlg);
        
        while (schedulingAlg!=SJF&&schedulingAlg!=PHPF&&schedulingAlg!=RR)
        {
        printf(
        "Please select a valid option.\n"
        "1. SJF: Shortest Job First\n"
        "2. PHPF: Preemptive Highest Priority First\n"
        "3. RR: Round Robin\n\n"
        "Choose the Scheduling Algorithm: ");
        scanf("%u", &schedulingAlg);
        }
        if(schedulingAlg==RR){
            unsigned int quantum;
            printf("Type the Quantum length: ");
            scanf("%u", &quantum);
        }
    #pragma endregion 
    
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}

struct processData* load(char* inpFileName)
{
	int count_processes = 0;
	struct processData* p_arr_process;
	
	//Opening File and checking if its been successfully opened 
	FILE* p_file = fopen(inpFileName, "r");
	if (p_file == NULL)
	{
		perror("ERROR HAS OCCURED IN INPUT FILE OPENINING");
		fclose(p_file);
		return NULL;
	}


	//Buffer needed to store content of line (This is needed even we dont need the content while counting the number of processes)
	char buffer[256];
	
	//skipping past comments line
	fgets(buffer, sizeof(buffer), p_file);

	//itterating through the file to find number of processes
	while (fgets(buffer, sizeof(buffer), p_file) != NULL) { count_processes += 1;  }

	//once counting is complete we close the file to now read the file data 
	fclose(p_file);

	//Dyanmic allocation of array for processes
	p_arr_process = (struct processData*) malloc(count_processes * sizeof(struct processData));


	//Reopening input file
	p_file = fopen(inpFileName, "r");
	if (p_file == NULL)
	{
		perror("ERROR HAS OCCURED IN INPUT FILE OPENINING _2_");
		fclose(p_file);
		return NULL;
	}

	//skipping past comments line
	fgets(buffer, sizeof(buffer), p_file);

	//itterating through file to get data of each 
	for (int i = 0; i < count_processes; i++)
	{
		if (fscanf(p_file, "%d\t%d\t%d\t%d\t", &p_arr_process[i].id, &p_arr_process[i].arrivalTime, &p_arr_process[i].runTime, &p_arr_process[i].priority) != 4) 
		{
			printf("Issue reading from file");
		};
	}
		
	//Close File
	fclose(p_file);

	return p_arr_process;
}