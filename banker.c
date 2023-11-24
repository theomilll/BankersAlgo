#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CUSTOMERS 10
#define MAX_RESOURCES 5

int available[MAX_RESOURCES];
int alloc[MAX_CUSTOMERS][MAX_RESOURCES] = {0};
int max[MAX_CUSTOMERS][MAX_RESOURCES];
int need[MAX_CUSTOMERS][MAX_RESOURCES];
int numberOfResources;
int numberOfCustomers = MAX_CUSTOMERS;

int* parseCommandLineArguments(int argc, char *argv[], int *numberOfResources);
void readCustomerData(const char* filename);
void readCommandData(const char* filename, int numberOfResources, int available[], int alloc[][MAX_RESOURCES], int max[][MAX_RESOURCES], int need[][MAX_RESOURCES]);
bool isSafeState(int numberOfResources, int numberOfCustomers, int available[], int max[][MAX_RESOURCES], int alloc[][MAX_RESOURCES], int need[][MAX_RESOURCES]);
void processRequest(int customerNumber, int requestArray[], int numberOfResources, int availableResources[], int allocatedResources[][MAX_RESOURCES], int maxDemand[][MAX_RESOURCES], int currentNeed[][MAX_RESOURCES]);
void releaseResources(int customerNum, int release[], int numberOfResources, int available[], int alloc[][MAX_RESOURCES]);
void outputSystemState(int numberOfResources, int numberOfCustomers, int available[], int alloc[][MAX_RESOURCES], int max[][MAX_RESOURCES], int need[][MAX_RESOURCES]);

int* parseCommandLineArguments(int argc, char *argv[], int *numberOfResources) {
    *numberOfResources = argc - 1;
    int *resources = malloc(*numberOfResources * sizeof(int));
    for (int i = 1; i < argc; i++) {
        resources[i-1] = atoi(argv[i]);
    }
    return resources;
}

void readCustomerData(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s for reading\n", filename);
        return;
    }

    char line[100];
    int customerIndex = 0;
    while (fgets(line, sizeof(line), file) && customerIndex < MAX_CUSTOMERS) {
        for (int i = 0; i < MAX_RESOURCES; i++) {
            max[customerIndex][i] = 0;
        }

        int numParsed = sscanf(line, "%d,%d,%d,%d,%d",
            &max[customerIndex][0], &max[customerIndex][1],
            &max[customerIndex][2], &max[customerIndex][3],
            &max[customerIndex][4]);
        
        if (numParsed < 3) {
            fprintf(stderr, "Invalid format in customer data: %s\n", line);
        }
        customerIndex++;
    }

    fclose(file);
}



void readCommandData(const char* filename, int numberOfResources, int available[], int alloc[][MAX_RESOURCES], int max[][MAX_RESOURCES], int need[][MAX_RESOURCES]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s for reading\n", filename);
        return;
    }

    char line[100], commandStr[3];
    int customerNum, request[MAX_RESOURCES];
    
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%s", commandStr) != 1) {
            continue;
        }

        if (strcmp(commandStr, "*") == 0) {
            outputSystemState(numberOfResources, MAX_CUSTOMERS, available, alloc, max, need);
            continue;
        }

        if (sscanf(line, "%s %d %d %d %d", commandStr, &customerNum, &request[0], &request[1], &request[2]) != 5) {
            fprintf(stderr, "Invalid command format in line: %s\n", line);
            continue;
        }

        if (strcmp(commandStr, "RQ") == 0) {
            processRequest(customerNum, request, numberOfResources, available, alloc, max, need);
        } else if (strcmp(commandStr, "RL") == 0) {
            releaseResources(customerNum, request, numberOfResources, available, alloc);
        }
    }

    fclose(file);
}


bool isSafeState(int numberOfResources, int numberOfCustomers, int available[], int max[][MAX_RESOURCES], int alloc[][MAX_RESOURCES], int need[][MAX_RESOURCES]) {
    int work[numberOfResources];
    for (int i = 0; i < numberOfResources; i++) {
        work[i] = available[i];
    }

    bool finish[numberOfCustomers];
    for (int i = 0; i < numberOfCustomers; i++) {
        finish[i] = false;
    }

    bool found;
    do {
        found = false;
        for (int i = 0; i < numberOfCustomers; i++) {
            if (!finish[i]) {
                bool canSatisfy = true;
                for (int j = 0; j < numberOfResources; j++) {
                    if (need[i][j] > work[j]) {
                        canSatisfy = false;
                        break;
                    }
                }

                if (canSatisfy) {
                    for (int j = 0; j < numberOfResources; j++) {
                        work[j] += alloc[i][j];
                    }
                    finish[i] = true;
                    found = true;
                }
            }
        }
    } while (found);

    for (int i = 0; i < numberOfCustomers; i++) {
        if (!finish[i]) {
            return false;
        }
    }

    return true;
}

void processRequest(int customerNum, int request[], int numberOfResources, int available[], int alloc[][MAX_RESOURCES], int max[][MAX_RESOURCES], int need[][MAX_RESOURCES]) {
    for (int i = 0; i < numberOfResources; i++) {
        if (request[i] < 0 || request[i] > need[customerNum][i] || request[i] > available[i]) {
            printf("Request cannot be granted for customer %d\n", customerNum);
            return;
        }
    }

    for (int i = 0; i < numberOfResources; i++) {
        available[i] -= request[i];
        alloc[customerNum][i] += request[i];
        need[customerNum][i] -= request[i];
    }

    if (!isSafeState(numberOfResources, MAX_CUSTOMERS, available, max, alloc, need)) {
        for (int i = 0; i < numberOfResources; i++) {
            available[i] += request[i];
            alloc[customerNum][i] -= request[i];
            need[customerNum][i] += request[i];
        }
        printf("Cannot grant request for customer %d as it leads to an unsafe state\n", customerNum);
    } else {
        printf("Request from customer %d has been granted\n", customerNum);
    }
}

void releaseResources(int customerNum, int release[], int numberOfResources, int available[], int alloc[][MAX_RESOURCES]) {
    for (int i = 0; i < numberOfResources; i++) {
        if (release[i] < 0 || release[i] > alloc[customerNum][i]) {
            printf("Release request exceeds allocation for customer %d\n", customerNum);
            return;
        }
    }

    for (int i = 0; i < numberOfResources; i++) {
        available[i] += release[i];
        alloc[customerNum][i] -= release[i];
    }

    printf("Resources released from customer %d\n", customerNum);
}


void outputSystemState(int numberOfResources, int numberOfCustomers, int available[], int alloc[][MAX_RESOURCES], int max[][MAX_RESOURCES], int need[][MAX_RESOURCES]) {
    printf("\nCurrent System State:\n");

    printf("AVAILABLE RESOURCES: ");
    for (int i = 0; i < numberOfResources; i++) {
        printf("%d ", available[i]);
    }
    printf("\n\n");

    printf("CUSTOMER | MAXIMUM DEMAND | CURRENT ALLOCATION | CURRENT NEED\n");
    for (int i = 0; i < numberOfCustomers; i++) {
        printf("   %d    |    ", i);
        for (int j = 0; j < numberOfResources; j++) {
            printf("%d ", max[i][j]);
        }
        printf("   |    ");
        for (int j = 0; j < numberOfResources; j++) {
            printf("%d ", alloc[i][j]);
        }
        printf("   |    ");
        for (int j = 0; j < numberOfResources; j++) {
            printf("%d ", need[i][j]);
        }
        printf("\n");
    }
}


void calculateNeedArray() {
    for (int i = 0; i < numberOfCustomers; i++) {
        for (int j = 0; j < numberOfResources; j++) {
            need[i][j] = max[i][j] - alloc[i][j];
        }
    }
}



int main(int argc, char *argv[]) {
    int *resourceArray = parseCommandLineArguments(argc, argv, &numberOfResources);
    for (int i = 0; i < numberOfResources; i++) {
        available[i] = resourceArray[i];
    }
    free(resourceArray);

    readCustomerData("customer.txt");

    for (int i = 0; i < numberOfCustomers; i++) {
        for (int j = 0; j < numberOfResources; j++) {
            need[i][j] = max[i][j] - alloc[i][j];
        }
    }

    readCommandData("commands.txt", numberOfResources, available, alloc, max, need);

    outputSystemState(numberOfResources, numberOfCustomers, available, alloc, max, need);

    return 0;
}

