#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CUSTOMERS 10
#define MAX_RESOURCES 5

// Function declarations
int* parseCommandLineArguments(int argc, char *argv[], int *numberOfResources);
void readCustomerData(const char* filename);
void readCommandData(const char* filename, int numberOfResources, int available[], int alloc[][MAX_RESOURCES], int max[][MAX_RESOURCES], int need[][MAX_RESOURCES]);
bool isSafeState(int numberOfResources, int numberOfCustomers, int available[], int max[][MAX_RESOURCES], int alloc[][MAX_RESOURCES], int need[][MAX_RESOURCES]);
void processRequest(int customerNumber, int requestArray[], int numberOfResources, int availableResources[], int allocatedResources[][MAX_RESOURCES], int maxDemand[][MAX_RESOURCES], int currentNeed[][MAX_RESOURCES]);
void releaseResources(int customerNum, int release[], int numberOfResources, int available[], int alloc[][MAX_RESOURCES]);
void outputSystemState(int numberOfResources, int numberOfCustomers, int available[], int alloc[][MAX_RESOURCES], int max[][MAX_RESOURCES], int need[][MAX_RESOURCES]);

void runTestCases() {
    int numberOfResources = 3;
    int numberOfCustomers = 5;
    int available[3] = {10, 10, 10};

    int max[5][MAX_RESOURCES] = {{7, 5, 3, 0, 0}, {3, 2, 2, 0, 0}, {9, 0, 2, 0, 0}, {2, 2, 2, 0, 0}, {4, 3, 3, 0, 0}};
    int alloc[5][MAX_RESOURCES] = {{0, 1, 0, 0, 0}, {2, 0, 0, 0, 0}, {3, 0, 2, 0, 0}, {2, 1, 1, 0, 0}, {0, 0, 2, 0, 0}};
    int need[5][MAX_RESOURCES];

    for (int i = 0; i < numberOfCustomers; i++) {
        for (int j = 0; j < numberOfResources; j++) {
            need[i][j] = max[i][j] - alloc[i][j];
        }
        for (int j = numberOfResources; j < MAX_RESOURCES; j++) {
            need[i][j] = 0;
        }
    }

    int request1[3] = {1, 0, 2};
    printf("Running Test 1: Safe Request\n");
    processRequest(0, request1, numberOfResources, available, alloc, max, need);
    outputSystemState(numberOfResources, numberOfCustomers, available, alloc, max, need);
}

int main(int argc, char *argv[]) {
    int available[MAX_RESOURCES];
    int alloc[MAX_CUSTOMERS][MAX_RESOURCES];
    int max[MAX_CUSTOMERS][MAX_RESOURCES];
    int need[MAX_CUSTOMERS][MAX_RESOURCES];
    int numberOfCustomers = MAX_CUSTOMERS;

    runTestCases();

    int numberOfResources;
    int *resources = parseCommandLineArguments(argc, argv, &numberOfResources);

    readCustomerData("customer.txt");
    readCommandData("commands.txt", numberOfResources, available, alloc, max, need);

    free(resources);
    return 0;
}

int* parseCommandLineArguments(int argc, char *argv[], int *numberOfResources) {
    // Basic implementation
    *numberOfResources = argc - 1;
    int *resources = malloc(*numberOfResources * sizeof(int));
    for (int i = 1; i < argc; i++) {
        resources[i-1] = atoi(argv[i]);
    }
    return resources;
}

// Existing function definitions...

bool isSafeState(int numberOfResources, int numberOfCustomers, int available[], int max[][MAX_RESOURCES], int alloc[][MAX_RESOURCES], int need[][MAX_RESOURCES]) {
    // Work vector to store the available amount of each resource
    int work[numberOfResources];
    for (int i = 0; i < numberOfResources; i++) {
        work[i] = available[i];
    }

    // Finish vector to indicate if a customer's maximum demand can be satisfied
    bool finish[numberOfCustomers];
    for (int i = 0; i < numberOfCustomers; i++) {
        finish[i] = false;
    }

    // Loop to find a customer whose needs can be satisfied with the available resources
    bool found;
    do {
        found = false;
        for (int i = 0; i < numberOfCustomers; i++) {
            if (!finish[i]) {
                // Check if resources can satisfy this customer's need
                bool canSatisfy = true;
                for (int j = 0; j < numberOfResources; j++) {
                    if (need[i][j] > work[j]) {
                        canSatisfy = false;
                        break;
                    }
                }

                // If all needs can be satisfied, update work and finish for this customer
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

    // Check if all customers are finished
    for (int i = 0; i < numberOfCustomers; i++) {
        if (!finish[i]) {
            return false; // Not in a safe state
        }
    }

    return true; // Safe state
}


void processRequest(int customerNum, int request[], int numberOfResources, int available[], int alloc[][MAX_RESOURCES], int max[][MAX_RESOURCES], int need[][MAX_RESOURCES]) {
    // First, check if the request exceeds the customer's maximum demand
    for (int i = 0; i < numberOfResources; i++) {
        if (request[i] > need[customerNum][i]) {
            printf("The customer %d request exceeds its maximum demand\n", customerNum);
            return;
        }
    }

    // Next, check if the request can be satisfied with available resources
    for (int i = 0; i < numberOfResources; i++) {
        if (request[i] > available[i]) {
            printf("The resources are not available for customer %d\n", customerNum);
            return;
        }
    }

    // Tentatively allocate requested resources (assuming a safe state)
    for (int i = 0; i < numberOfResources; i++) {
        available[i] -= request[i];
        alloc[customerNum][i] += request[i];
        need[customerNum][i] -= request[i];
    }

    // Check if the new state is safe
    if (!isSafeState(numberOfResources, MAX_CUSTOMERS, available, max, alloc, need)) {
        // If not safe, roll back the allocation
        for (int i = 0; i < numberOfResources; i++) {
            available[i] += request[i];
            alloc[customerNum][i] -= request[i];
            need[customerNum][i] += request[i];
        }
        printf("The request from customer %d leaves the system in an unsafe state\n", customerNum);
    } else {
        printf("Request from customer %d has been granted\n", customerNum);
    }
}


void releaseResources(int customerNum, int release[], int numberOfResources, int available[], int alloc[][MAX_RESOURCES]) {
    // First, check if the release request exceeds the customer's current allocation
    for (int i = 0; i < numberOfResources; i++) {
        if (release[i] > alloc[customerNum][i]) {
            printf("Release request exceeds the current allocation for customer %d\n", customerNum);
            return;
        }
    }

    // Release the resources and update the available and allocation matrices
    for (int i = 0; i < numberOfResources; i++) {
        available[i] += release[i];
        alloc[customerNum][i] -= release[i];
    }

    printf("Resources released from customer %d\n", customerNum);
}


void outputSystemState(int numberOfResources, int numberOfCustomers, int available[], int alloc[][MAX_RESOURCES], int max[][MAX_RESOURCES], int need[][MAX_RESOURCES]) {
    printf("Current System State:\n");
    printf("AVAILABLE RESOURCES:\n");
    for (int i = 0; i < numberOfResources; i++) {
        printf("Resource %d: %d\n", i, available[i]);
    }

    printf("\nMAXIMUM DEMAND:\n");
    for (int i = 0; i < numberOfCustomers; i++) {
        printf("Customer %d: ", i);
        for (int j = 0; j < numberOfResources; j++) {
            printf("%d ", max[i][j]);
        }
        printf("\n");
    }

    printf("\nCURRENT ALLOCATION:\n");
    for (int i = 0; i < numberOfCustomers; i++) {
        printf("Customer %d: ", i);
        for (int j = 0; j < numberOfResources; j++) {
            printf("%d ", alloc[i][j]);
        }
        printf("\n");
    }

    printf("\nCURRENT NEED:\n");
    for (int i = 0; i < numberOfCustomers; i++) {
        printf("Customer %d: ", i);
        for (int j = 0; j < numberOfResources; j++) {
            printf("%d ", need[i][j]);
        }
        printf("\n");
    }
}

void readCustomerData(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s for reading\n", filename);
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        printf("Customer Data: %s", line); // Replace this with actual processing
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
    while (fgets(line, sizeof(line), file)) {
        int customerNum, request[MAX_RESOURCES];
        
        if (sscanf(line, "%s %d", commandStr, &customerNum) == 2) {
            if (strcmp(commandStr, "RQ") == 0) {
                if (sscanf(line, "RQ %d %d %d %d", &customerNum, &request[0], &request[1], &request[2]) == 4) {
                    processRequest(customerNum, request, numberOfResources, available, alloc, max, need);
                }
            } else if (strcmp(commandStr, "RL") == 0) {
                if (sscanf(line, "RL %d %d %d %d", &customerNum, &request[0], &request[1], &request[2]) == 4) {
                    releaseResources(customerNum, request, numberOfResources, available, alloc);
                }
            }
        }
        
        outputSystemState(numberOfResources, MAX_CUSTOMERS, available, alloc, max, need);
    }

    fclose(file);
}

