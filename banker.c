#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>

int *available; // Array to track available amount of each resource
int **maximum; // Maximum demand of each customer
int **allocation; // Amount currently allocated to each customer
int **need; // Remaining needs of each customer
int NUMBER_OF_RESOURCES;
int NUMBER_OF_CUSTOMERS;

FILE *output_file;

void initialize_system_state(char **argv);
void read_customer_file(const char *filename);
bool is_request_valid(int customer_id, int request[]);
bool is_enough_resources(int request[]);
bool is_release_valid(int customer_id, int release[]);
bool is_safe_state();
void process_request(int customer_id, int request[]);
void process_command(const char *command);
void process_release(int customer_id, int release[]);
void read_commands_file(const char *filename);
bool is_number(const char *str);
int count_customers(const char *filename);
void print_system_state();

int count_customers(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open customer.txt for counting");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        count++;
    }

    fclose(file);
    return count;
}

void initialize_system_state(char **argv) {
    // Allocate memory for available resources
    available = (int *)malloc(NUMBER_OF_RESOURCES * sizeof(int));
    if (available == NULL) {
        perror("Failed to allocate memory for available resources");
        exit(EXIT_FAILURE);
    }

    // Initialize available resources from command line arguments
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] = atoi(argv[i + 1]);
    }

    // Allocate and initialize maximum, allocation, and need matrices
    maximum = (int **)malloc(NUMBER_OF_CUSTOMERS * sizeof(int *));
    allocation = (int **)malloc(NUMBER_OF_CUSTOMERS * sizeof(int *));
    need = (int **)malloc(NUMBER_OF_CUSTOMERS * sizeof(int *));
    if (maximum == NULL || allocation == NULL || need == NULL) {
        perror("Failed to allocate memory for matrices");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        maximum[i] = (int *)malloc(NUMBER_OF_RESOURCES * sizeof(int));
        allocation[i] = (int *)malloc(NUMBER_OF_RESOURCES * sizeof(int));
        need[i] = (int *)malloc(NUMBER_OF_RESOURCES * sizeof(int));
        if (maximum[i] == NULL || allocation[i] == NULL || need[i] == NULL) {
            perror("Failed to allocate memory for matrix rows");
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            maximum[i][j] = 0;
            allocation[i][j] = 0;
            need[i][j] = 0;
        }
    }
    printf("Available resources: ");
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        printf("%d ", available[i]);
    }
    printf("\n");
}

bool is_number(const char *str) {
    char *endptr;
    long val = strtol(str, &endptr, 10);
    return endptr != str && *endptr == '\0' && val <= INT_MAX && val >= INT_MIN;
}

void read_customer_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening customer.txt: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (int customer_id = 0; customer_id < NUMBER_OF_CUSTOMERS; customer_id++) {
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            if (fscanf(file, "%d,", &maximum[customer_id][i]) != 1) {
                fprintf(stderr, "Error reading customer.txt at line %d, resource %d\n", customer_id + 1, i + 1);
                exit(EXIT_FAILURE);
            }
            allocation[customer_id][i] = 0; // Initialize allocation to 0
            need[customer_id][i] = maximum[customer_id][i]; // Need is initially equal to maximum
        }
    }
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
    printf("Customer %d maximum demand: ", i);
    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
        printf("%d ", maximum[i][j]);
    }
    printf("\n");
    }

    fclose(file);
}

bool is_request_valid(int customer_id, int request[]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (request[i] > need[customer_id][i]) {
            return false;
        }
    }
    return true;
}

bool is_enough_resources(int request[]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (request[i] > available[i]) {
            return false;
        }
    }
    return true;
}

bool is_release_valid(int customer_id, int release[]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (release[i] > allocation[customer_id][i]) {
            return false;
        }
    }
    return true;
}

bool is_safe_state() {
    int work[NUMBER_OF_RESOURCES];
    bool finish[NUMBER_OF_CUSTOMERS];
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        work[i] = available[i];
    }
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        finish[i] = false;
    }

    while (true) {
        bool found = false;
        for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
            if (!finish[i]) {
                bool can_allocate = true;
                for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                    if (need[i][j] > work[j]) {
                        can_allocate = false;
                        break;
                    }
                }

                if (can_allocate) {
                    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                        work[j] += allocation[i][j];
                    }
                    finish[i] = true;
                    found = true;
                }
            }
        }

        if (!found) {
            break;
        }
    }

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        if (!finish[i]) {
            return false;
        }
    }
    return true;
}

void process_request(int customer_id, int request[]) {
    if (!is_request_valid(customer_id, request)) {
        fprintf(output_file, "The customer %d request was denied because it exceeds its maximum need\n", customer_id);
        return;
    }

    if (!is_enough_resources(request)) {
        fprintf(output_file, "The resources are not enough for customer %d request\n", customer_id);
        return;
    }

    // Tentatively allocate resources
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] -= request[i];
        allocation[customer_id][i] += request[i];
        need[customer_id][i] -= request[i];
    }

    if (!is_safe_state()) {
        // Rollback allocation if it leads to an unsafe state
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            available[i] += request[i];
            allocation[customer_id][i] -= request[i];
            need[customer_id][i] += request[i];
        }
        fprintf(output_file, "The customer %d request was denied because it results in an unsafe state\n", customer_id);
    } else {
        fprintf(output_file, "Allocate to customer %d the resources", customer_id);
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            fprintf(output_file, " %d", request[i]);
        }
        fprintf(output_file, "\n");
    }
}

void process_command(const char *command) {
    char cmd_type[3];
    int customer_id;
    int resources[NUMBER_OF_RESOURCES];

    // Initialize resources array to zero
    memset(resources, 0, sizeof(resources));

    if (strcmp(command, "*\n") == 0) {  // Check for '*' command
        print_system_state();
        return;
    }

    if (sscanf(command, "%2s %d", cmd_type, &customer_id) == 2) {
        // Validate customer_id
        if (customer_id < 0 || customer_id >= NUMBER_OF_CUSTOMERS) {
            fprintf(stderr, "Invalid customer ID: %d\n", customer_id);
            return;
        }

        // Parse resource amounts (either request or release)
        char *token = strtok((char *)command + 4, " ");
        for (int i = 0; token != NULL && i < NUMBER_OF_RESOURCES; i++) {
            resources[i] = atoi(token);
            token = strtok(NULL, " ");
        }

        // Process based on command type
        if (strcmp(cmd_type, "RQ") == 0) {
            process_request(customer_id, resources);
        } else if (strcmp(cmd_type, "RL") == 0) {
            process_release(customer_id, resources);
        }
    } else {
        fprintf(stderr, "Invalid command format: %s\n", command);
    }
}

void process_release(int customer_id, int release[]) {
    if (!is_release_valid(customer_id, release)) {
        printf("Release from customer %d was denied because it exceeds its current allocation\n", customer_id);
        return;
    }

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        allocation[customer_id][i] -= release[i];
        available[i] += release[i];
        need[customer_id][i] += release[i];
    }

    printf("Release from customer %d the resources", customer_id);
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        printf(" %d", release[i]);
    }
    printf("\n");
}

void read_commands_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open commands.txt");
        exit(EXIT_FAILURE);
    }

    char command[256];
    while (fgets(command, sizeof(command), file) != NULL) {
        process_command(command);
    }

    fclose(file);
}

void print_system_state() {
    // Header
    fprintf(output_file, "MAXIMUM | ALLOCATION | NEED\n");

    // Printing Maximum, Allocation, and Need side by side
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        // Maximum
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            fprintf(output_file, "%d ", maximum[i][j]);
        }
        fprintf(output_file, "| ");

        // Allocation
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            fprintf(output_file, "%d ", allocation[i][j]);
        }
        fprintf(output_file, "| ");

        // Need
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            fprintf(output_file, "%d ", need[i][j]);
        }
        fprintf(output_file, "\n");
    }

    // Print Available Resources
    fprintf(output_file, "AVAILABLE ");
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        fprintf(output_file, "%d ", available[i]);
    }
    fprintf(output_file, "\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <resource1> <resource2> ... <resourceN>\n", argv[0]);
        return -1;
    }

    // Validate command-line arguments
    for (int i = 1; i < argc; i++) {
        if (!is_number(argv[i])) {
            printf("Incompatibility between command line arguments\n");
            return -1;
        }
    }

    NUMBER_OF_RESOURCES = argc - 1;

    // Check if customer.txt can be accessed
    if (access("customer.txt", F_OK) == -1) {
        printf("Fail to read customer.txt\n");
        return -1;
    }

    // Determine the number of customers from customer.txt
    NUMBER_OF_CUSTOMERS = count_customers("customer.txt");

    // Initialize system state
    initialize_system_state(argv);

    // Open the output file
    output_file = fopen("result.txt", "w");
    if (output_file == NULL) {
        perror("Failed to open result.txt");
        return -1;
    }

    // Read customer file to populate data structures
    read_customer_file("customer.txt");

    // Check if commands.txt can be accessed
    if (access("commands.txt", F_OK) == -1) {
        printf("Fail to read commands.txt\n");
        fclose(output_file);
        return -1;
    }

    // Read and process commands
    read_commands_file("commands.txt");

    // Close the output file
    fclose(output_file);

    // Free dynamically allocated memory
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        free(maximum[i]);
        free(allocation[i]);
        free(need[i]);
    }
    free(maximum);
    free(allocation);
    free(need);
    free(available);

    fclose(output_file);
    return 0;
}
