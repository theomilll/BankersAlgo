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

// Function declarations
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
    while (fgets(line, sizeof(line), file)) {
        printf("Customer Data: %s", line);
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
            continue; // Invalid line format
        }

        if (strcmp(commandStr, "*") == 0) {
            // Handle the '*' command here, if necessary
            outputSystemState(numberOfResources, MAX_CUSTOMERS, available, alloc, max, need);
            continue;
        }

        if (sscanf(line, "%s %d %d %d %d", commandStr, &customerNum, &request[0], &request[1], &request[2]) != 5) {
            fprintf(stderr, "Invalid command format in line: %s\n", line);
            continue; // Invalid command format
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
    // Check for negative or excessive resource requests
    for (int i = 0; i < numberOfResources; i++) {
        if (request[i] < 0 || request[i] > need[customerNum][i]) {
            printf("Invalid or unsatisfiable request from customer %d\n", customerNum);
            return;
        }
    }

    // Tentatively allocate requested resources
    for (int i = 0; i < numberOfResources; i++) {
        available[i] -= request[i];
        alloc[customerNum][i] += request[i];
        need[customerNum][i] -= request[i];
    }

    // Check if the new state is safe
    if (!isSafeState(numberOfResources, MAX_CUSTOMERS, available, max, alloc, need)) {
        // Roll back the allocation if not safe
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
    for (int i = 0; i < numberOfResources; i++) {
        if (release[i] > alloc[customerNum][i]) {
            printf("Release request exceeds the current allocation for customer %d\n", customerNum);
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

int main(int argc, char *argv[]) {
    // Example of initial state for available resources
    for (int i = 0; i < MAX_RESOURCES; i++) {
        available[i] = 10; // Adjust as per your project requirements
    }

    // Initialize alloc array to 0 (already done in global declaration)

    // Read initial customer data to set up the max array
    readCustomerData("customer.txt");

    // Initialize need array based on max and alloc
    for (int i = 0; i < numberOfCustomers; i++) {
        for (int j = 0; j < numberOfResources; j++) {
            need[i][j] = max[i][j] - alloc[i][j];
        }
    }
    readCommandData("commands.txt", numberOfResources, available, alloc, max, need);

    return 0;
}

