
// libraries

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

// -------------------------------------------------------------------------------------

// gloabl variables

int time = 0;

// -------------------------------------------------------------------------------------

// structures

typedef struct process {
    int processId;
    char processStatus[50];
    int serviceTime;
    int readyTime;
    int blockedTime;
    int localTime;
    int requests[50];
    int numRequests;
    struct process *next;
} PROCESS;


typedef struct {
	PROCESS *front;
	PROCESS *rear;
} QUEUE;

typedef struct {
    PROCESS *p;
} CPU;

typedef struct {
    PROCESS *p;
    int startTime;
} HDD;

typedef struct {
    int localTime;
    int readyTime;
} IDLE;

// -------------------------------------------------------------------------------------

// function definitions

void manage_hdd(HDD *hdd, QUEUE *blockedQueue, QUEUE *readyQueue);
int manage_cpu(CPU *cpu, QUEUE *readyQueue, QUEUE *blockedQueue);
void load_incoming_process(PROCESS **processesArray, int numProcesses, QUEUE *readyQueue);
void increment_local_time(CPU *cpu, IDLE *idle);
void increment_ready_time(QUEUE *readyQueue);
void increment_blocked_time(QUEUE *blockedQueue, HDD *hdd);
PROCESS *new_process(char processId, char *processStatus, int serviceTime, int readyTime, int blockedTime, int localTime, int requests[], int numRequests);
void create_process_table(char *input, PROCESS **processesArray, int numProcesses);
void display_process_table(PROCESS **processesArray, int numProcesses, IDLE *idle);
void enqueue(QUEUE *qp, PROCESS *np);
PROCESS *dequeue(QUEUE *qp);

// -------------------------------------------------------------------------------------

// main / testing

int main(int argc, char *argv[]) {
    char input[50];
    scanf("%s", input);
    int numProcesses = strlen(input);
    PROCESS *processesArray[numProcesses];
    create_process_table(input, processesArray, numProcesses);
    QUEUE readyQueue = {0};
    QUEUE blockedQueue = {0};
    CPU cpu = {0};
    HDD hdd = {0};
    IDLE idle = {0};
    int terminated = 0;
    while (1) {
        manage_hdd(&hdd, &blockedQueue, &readyQueue);
        load_incoming_process(processesArray, numProcesses, &readyQueue);
        terminated += manage_cpu(&cpu, &readyQueue, &blockedQueue);
        if (terminated == numProcesses) break;
        manage_hdd(&hdd, &blockedQueue, &readyQueue);
        increment_ready_time(&readyQueue);
        increment_blocked_time(&blockedQueue, &hdd);
        increment_local_time(&cpu, &idle);
        time += 50;
    }
    display_process_table(processesArray, numProcesses, &idle);
}

// -------------------------------------------------------------------------------------

// functions

/*
checks if process is using the HDD
    - if no process is using the HDD, give it to a process waiting for the HDD
    - if there is a process using the HDD, 
        - if the process is finished, remove it and if there is a process ready, give it HDD time
*/
void manage_hdd(HDD *hdd, QUEUE *blockedQueue, QUEUE *readyQueue) {
    // no process using hdd
    if (hdd->p == NULL) {
        // processes waiting for HDD
        if (blockedQueue->front != NULL) {
            hdd->p = dequeue(blockedQueue);
            hdd->startTime = time;
        }
    // process using hdd
    } else {
        // process done using hdd
        if (time - hdd->startTime == 1000) {
            enqueue(readyQueue, hdd->p);
            strcpy(hdd->p->processStatus, "ready");
            hdd->p = NULL;
            hdd->p = 0;
            // recrusive call to re-allocate hdd to process
            manage_hdd(hdd, blockedQueue, readyQueue);
        }
    }
}

/* 
checks if CPU is running a process
    - if CPU is NOT running a process check ifthere is a process ready to give it CPU time
    - if CPU is running a process
        - if the process is finished, remove it and if there is a process ready, give it CPU time
        - if the process needs I/O, put it in the blocked queue and if there is a process ready, give it CPU time
*/
int manage_cpu(CPU *cpu, QUEUE *readyQueue, QUEUE *blockedQueue) {
    // no running process
    if (cpu->p == NULL) {
        // non-empty ready queue
        if (readyQueue->front != NULL) {
            cpu->p = dequeue(readyQueue);
            strcpy(cpu->p->processStatus, "running");
        }
    // running process
    } else {
        // process done
        if (cpu->p->localTime == cpu->p->serviceTime) {
            strcpy(cpu->p->processStatus, "exited");
            cpu->p = NULL;
            // return 1 for success + recrusive call to re-allocate process to cpu
            return 1 + manage_cpu(cpu, readyQueue, blockedQueue);
        // process has scheduled requests
        } else if (cpu->p->numRequests > 0) {
            int i = 0;
            while (i < cpu->p->numRequests && cpu->p->requests[i] != cpu->p->localTime) { i++; }
            // process has scheduled I/O
            if (i != cpu->p->numRequests) {
                enqueue(blockedQueue, cpu->p);
                strcpy(cpu->p->processStatus, "blocked");
                cpu->p = NULL;
                // recrusive call to re-allocate process to cpu
                manage_cpu(cpu, readyQueue, blockedQueue);
            }
        }
    }
    return 0;
}

/*
checks if a process has arrived
    - if a process has arrived it is put into the ready queue
*/
void load_incoming_process(PROCESS **processesArray, int numProcesses, QUEUE *readyQueue) {
    int temp = 100 * numProcesses;
    // process incoming
    if (time % 100 == 0 && time < temp) {
        temp = time / 100;
        enqueue(readyQueue, *(processesArray + temp));
        strcpy((*(processesArray + temp))->processStatus, "ready");
    }
}

/*
increments the total local time of the process that currently has CPU time
*/
void increment_local_time(CPU *cpu, IDLE *idle) {
    // no running process
    if (cpu->p == NULL) {
        idle->localTime += 50;
    // running process
    } else {
        idle->readyTime += 50;
        cpu->p->localTime += 50;
    }
}

/*
increments the total ready time of all the processes currently in the ready queue
*/
void increment_ready_time(QUEUE *readyQueue) {
    PROCESS *p =readyQueue->front;
    // increment ready time of process in the blocked queue
    while (p) {
        p->readyTime += 50;
        p = p->next;
    }
}

/*
increments the total blocked time of all the processes currently in the blocked queue
increments the total blocked time of the process currently using the HDD
*/
void increment_blocked_time(QUEUE *blockedQueue, HDD *hdd) {
    PROCESS *p = blockedQueue->front;
    // increment blocked time of process in the blocked queue
    while (p) {
        p->blockedTime += 50;
        p = p->next;
    }
    // increment blocked tim of process using the HDD
    if (hdd->p != NULL) {
        hdd->p->blockedTime += 50;
    }
}

/*
creates and returns a new process
*/
PROCESS *new_process(char processId, char *processStatus, int serviceTime, int readyTime, int blockedTime, int localTime, int requests[], int numRequests) {
    PROCESS *p = (PROCESS *) malloc(sizeof(PROCESS));
    p->processId = processId;
    strcpy(p->processStatus, processStatus);
    p->serviceTime = serviceTime;
    p->readyTime = readyTime;
    p->blockedTime = blockedTime;
    p->localTime = localTime;
    for (int i=0; i<numRequests; i++) p->requests[i] = requests[i];
    p->numRequests = numRequests;
    return p;
}

/*
creates a process table that holds every process and their details
*/
void create_process_table(char *input, PROCESS **processesArray, int numProcesses) {
    int requests[3] = {50, 100, 150};
    for (int i = 0; i < numProcesses; i++) {
        if (*(input + i) == 'C') *(processesArray + i) = new_process(i + 1, "new", 1000, 0, 0, 0, requests, 0);
        else if (*(input + i) == 'S') *(processesArray + i) = new_process(i + 1, "new", 200, 0, 0, 0, requests, 0);
        else if (*(input + i) == 'I') *(processesArray + i) = new_process(i + 1, "new", 200, 0, 0, 0, requests, 3);
    }
}

/*
prints the details of the process table
*/
void display_process_table(PROCESS **processesArray, int numProcesses, IDLE *idle) {
    // display idle process
    int num = 0;
    printf("%d %d %d\n", num, idle->localTime, idle->readyTime);
    // display other processes
    for (int i = 0; i < numProcesses; i++) printf("%d %d %d %d\n", (*(processesArray + i))->processId, (*(processesArray + i))->localTime, (*(processesArray + i))->readyTime, (*(processesArray + i))->blockedTime);
}

/*
adds a process to the end of a queue
*/
void enqueue(QUEUE *qp, PROCESS *pp) {
    // empty queue
    if (qp->front == NULL) qp->front = pp;
    // non-empty queue
	else qp->rear->next = pp;
	qp->rear = pp;
	pp->next = NULL;
} 

/*
removes the first process in the queue
*/
PROCESS *dequeue(QUEUE *qp) {
	PROCESS *pp = NULL;
    // non-empty queue
	if (qp->front) {
		pp = qp->front;
		qp->front = pp->next;
		if (qp->front == qp->rear) {
            qp->rear = NULL;
		}
	}
	return pp;
}
