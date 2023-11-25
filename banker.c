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
int actualNumberOfCustomers = 0;


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
    actualNumberOfCustomers = 0;
    while (fgets(line, sizeof(line), file) && actualNumberOfCustomers < MAX_CUSTOMERS) {
        for (int i = 0; i < MAX_RESOURCES; i++) {
            max[actualNumberOfCustomers][i] = 0;
        }

        int numParsed = sscanf(line, "%d,%d,%d,%d,%d",
            &max[actualNumberOfCustomers][0], &max[actualNumberOfCustomers][1],
            &max[actualNumberOfCustomers][2], &max[actualNumberOfCustomers][3],
            &max[actualNumberOfCustomers][4]);
        
        if (numParsed < 3) {
            fprintf(stderr, "Invalid format in customer data: %s\n", line);
        } else {
            actualNumberOfCustomers++;
        }
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
    FILE *file = fopen("result.txt", "a"); // Append mode

    for (int i = 0; i < numberOfResources; i++) {
        if (request[i] < 0 || request[i] > need[customerNum][i] || request[i] > available[i]) {
            fprintf(file, "The customer %d request cannot be granted.\n", customerNum);
            fclose(file);
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
        fprintf(file, "The request from customer %d cannot be granted as it leads to an unsafe state.\n", customerNum);
    } else {
        fprintf(file, "Allocate to customer %d the resources", customerNum);
        for (int i = 0; i < numberOfResources; i++) {
            fprintf(file, " %d", request[i]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}


void releaseResources(int customerNum, int release[], int numberOfResources, int available[], int alloc[][MAX_RESOURCES]) {
    FILE *file = fopen("result.txt", "a"); // Append mode

    for (int i = 0; i < numberOfResources; i++) {
        if (release[i] < 0 || release[i] > alloc[customerNum][i]) {
            fprintf(file, "Release request from customer %d cannot be granted.\n", customerNum);
            fclose(file);
            return;
        }
    }

    fprintf(file, "Release from customer %d the resources", customerNum);
    for (int i = 0; i < numberOfResources; i++) {
        fprintf(file, " %d", release[i]);
        available[i] += release[i];
        alloc[customerNum][i] -= release[i];
    }
    fprintf(file, "\n");

    fclose(file);
}

void outputSystemState(int numberOfResources, int actualNumberOfCustomers, int available[], int alloc[][MAX_RESOURCES], int max[][MAX_RESOURCES], int need[][MAX_RESOURCES]) {
    FILE *fp = fopen("result.txt", "a"); // Open in append mode
    if (fp == NULL) {
        fprintf(stderr, "Error opening file result.txt\n");
        return;
    }

    fprintf(fp, "\nCurrent System State:\n");

    fprintf(fp, "AVAILABLE RESOURCES: ");
    for (int i = 0; i < numberOfResources; i++) {
        fprintf(fp, "%d ", available[i]);
    }
    fprintf(fp, "\n\n");

    fprintf(fp, "CUSTOMER | MAXIMUM DEMAND | CURRENT ALLOCATION | CURRENT NEED\n");
    for (int i = 0; i < actualNumberOfCustomers; i++) {
        fprintf(fp, "   %d    |    ", i);
        for (int j = 0; j < numberOfResources; j++) {
            fprintf(fp, "%d ", max[i][j]);
        }
        fprintf(fp, "   |    ");
        for (int j = 0; j < numberOfResources; j++) {
            fprintf(fp, "%d ", alloc[i][j]);
        }
        fprintf(fp, "   |    ");
        for (int j = 0; j < numberOfResources; j++) {
            fprintf(fp, "%d ", need[i][j]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

void calculateNeedArray() {
    for (int i = 0; i < numberOfCustomers; i++) {
        for (int j = 0; j < numberOfResources; j++) {
            need[i][j] = max[i][j] - alloc[i][j];
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Error: Insufficient command line arguments provided.\n");
        return 1;
    }

    int *resourceArray = parseCommandLineArguments(argc, argv, &numberOfResources);

    for (int i = 0; i < numberOfResources; i++) {
        available[i] = resourceArray[i];
    }

    readCustomerData("customer.txt");
    calculateNeedArray();
    readCommandData("commands.txt", numberOfResources, available, alloc, max, need);

    free(resourceArray);
    return 0;
}


