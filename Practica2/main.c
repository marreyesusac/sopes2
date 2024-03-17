#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#define MAX_ERRORS 1000 // Adjust if you expect more errors
#define MAX_OP_ERRORS 1000 

typedef struct {
    int line_no;
    char message[100]; 
} OpError;

typedef enum {
    DEPOSIT,
    WITHDRAWAL,
    TRANSFER
} OperationType;

typedef struct {
    OperationType operation;
    int account1;
    int account2; // Used only for transfers
    double amount;
} Transaction;

OpError op_errors[MAX_OP_ERRORS];
int op_error_index = 0;

typedef struct {
    int line_no;
    char *line;
    int thread_index;
} ThreadData;


typedef struct {
    int line_no;
    char message[100]; // Adjust size if needed
} Error;

Error load_errors[MAX_ERRORS];
int error_index = 0;

void display_menu() {
    printf("\nBanking System Menu\n");
    printf("1. Deposit\n");
    printf("2. Withdrawal\n");
    printf("3. Transaction\n");
    printf("4. Check Account\n");
    printf("5. Exit\n");
    printf("6. Load Users from CSV\n");
    printf("7. Load transactions from CSV\n");
    printf("Enter your choice: ");
}


typedef struct Account {
    int account_no;
    char name[100];  // Adjust size if needed
    double balance;
    pthread_mutex_t mutex; // Add the mutex 
} Account;

// Placeholder - your actual data storage mechanism 
Account accounts[1000]; // Adjust size as needed, consider dynamic allocation
int num_accounts = 0;
int num_transactions = 0;
int errors = 0; // Counter for total errors
int thread_loads[3] = {0, 0, 0};
int line_no = 0;
int total_withdrawals = 0;
int total_deposits = 0;
int total_transfers = 0;
int thread_index=0;
int total_errors_transactions = 0;
//--------------------------------------------------- Function to be executed by each thread
void *process_csv_line(void *arg) {
    char *line = (char *)arg;
    char *token;
    line_no++;

    // Tokenize the line
    token = strtok(line, ",");
    if (token == NULL) {
        printf("Error: Invalid CSV line format.\n");
        return NULL;
    }
    int account_no = atoi(token); 

    token = strtok(NULL, ",");
    if (token == NULL) {
        printf("Error: Invalid CSV line format.\n");
        return NULL;
    }
    char name[100]; // Adjust size if needed
    strcpy(name, token); 

    token = strtok(NULL, ","); 
    if (token == NULL) {
        printf("Error: Invalid CSV line format.\n");
        return NULL;
    }
    double balance = atof(token); 

     if (account_no <= 0) {
        printf("Error: Line %d - Account number must be positive.\n", line_no); 
        errors++;
        load_errors[error_index].line_no = line_no;
        sprintf(load_errors[error_index].message, "Error: Line %d - %s", line_no, "Account number must be positive.\n"); // Customize the message
        error_index++;
        return NULL;
    }
    if (balance < 0) {
        printf("Error: Line %d - Balance cannot be negative.\n", line_no); 
        errors++;
        load_errors[error_index].line_no = line_no;
        sprintf(load_errors[error_index].message, "Error: Line %d - %s", line_no, "Balance cannot be negative.\n"); // Customize the message
        error_index++;
        return NULL;
    }

    // Check for duplicate account numbers (requires data structure modification) 
    for (int i = 0; i < num_accounts; i++) {
        if (accounts[i].account_no == account_no) {
            printf("Error: Line %d - Duplicate account number.\n", line_no);
            errors++;
            load_errors[error_index].line_no = line_no;
            sprintf(load_errors[error_index].message, "Error: Line %d - %s", line_no, "Duplicate account number.\n"); // Customize the message
            error_index++;
            return NULL;
        }
    } 


    // Add account to storage (with synchronization later)
    accounts[num_accounts].account_no = account_no;
    strcpy(accounts[num_accounts].name, name);  
    accounts[num_accounts].balance = balance;
    pthread_mutex_init(&accounts[num_accounts].mutex, NULL);
    num_accounts++;

    thread_loads[thread_index]++; 

    
    free(line); // Free the dynamically allocated line copy
    return NULL;
}

void load_users_from_csv(const char *file_path) {
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    int num_threads = 3; 
    pthread_t threads[num_threads];

    // Read and process lines with threads
    int i = 0;
    while (getline(&line, &len, fp) != -1) {
        // Create a copy of the line for the thread
        char *line_copy = malloc(strlen(line) + 1); 
        if (line_copy == NULL) {
            printf("Memory allocation error.\n");
            // Handle memory errors more gracefully
            continue;  
        }
        strcpy(line_copy, line); 
        thread_index = i;
        if (pthread_create(&threads[i], NULL, process_csv_line, (void *)line_copy) != 0) {
            printf("Error creating thread.\n");
            // Handle thread creation errors more gracefully
        }

        i++;
        if (i == num_threads) { 
            // Wait for threads to finish before reading more lines
            for (int j = 0; j < num_threads; j++) {
                pthread_join(threads[j], NULL);
            }
            i = 0; 
        }
    }

    // Wait for any remaining threads
    for (int j = 0; j < i; j++) {
        pthread_join(threads[j], NULL);
    }

    fclose(fp);
    if (line) free(line); 
    printf("Loaded %d accounts from CSV.\n", num_accounts);
     time_t current_time;
    struct tm *time_info;
    char time_string[20]; 

    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "load_%Y_.log", time_info);

    FILE *report_file = fopen(time_string, "w");
    if (report_file == NULL) {
        printf("Error creating report file.\n");
        return;
    }

    fprintf(report_file, "------\nDate: %sUploaded Users:\n", time_string);
    fprintf(report_file, "Thread #1: %d\n", thread_loads[0]);
    fprintf(report_file, "Thread #2: %d\n", thread_loads[1]);
    fprintf(report_file, "Thread #3: %d\n", thread_loads[2]);
    fprintf(report_file, "Total: %d\n", num_accounts); 

    if (errors > 0) {
        fprintf(report_file, "Errors:\n"); // Add error list here
    }
    if (error_index > 0) { // Check if any errors were stored
        fprintf(report_file, "Errors:\n"); 
        for (int i = 0; i < error_index; i++) {
            fprintf(report_file, "%s\n", load_errors[i].message);
        }
    }

    fclose(report_file); 
    printf("Loaded %d accounts from CSV. Load report generated.\n", num_accounts);

}

int get_user_input() {
    int choice;

    if (scanf("%d", &choice) != 1 || choice < 1 || choice > 8) {
        printf("Invalid input. Please enter a number between 1 and 8.\n");
        fflush(stdin); // Clear input buffer
        return -1; // Indicate error
    }

    return choice;
}




//--------------------Funciones para cargar transacciones------------------------------------
Account *find_account(int account_no);
void perform_transaction(Transaction *transaction);

void *process_transaction_line(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    char *line = data->line;
    char *token;
    int line_no = data->line_no;
    int errors = 0; 

    Transaction transaction;

    // 1. Parse the operation type
    token = strtok(line, ",");
    if (token == NULL) {
        printf("Error: Line %d - Invalid CSV format.\n", line_no);
        errors++;
        sprintf(op_errors[op_error_index].message, "Error: Line %d - %s", line_no, "Invalid CSV format.\n"); // Customize the message
        op_error_index++;
        return NULL;
    }
    int op_code = atoi(token);
    if (op_code < 1 || op_code > 3) {
        printf("Error: Line %d - Invalid operation code.\n", line_no); 
        errors++;
        sprintf(op_errors[op_error_index].message, "Error: Line %d - %s", line_no, "Invalid operation code.\n"); // Customize the message
        op_error_index++;
        return NULL;
    }
    if (op_code == 1) {
        transaction.operation = DEPOSIT;
    } else if (op_code == 2) {
        transaction.operation = WITHDRAWAL;
    } else if (op_code == 3) {
        transaction.operation = TRANSFER;
    }

    // 2. Parse account1
    token = strtok(NULL, ","); 
    if (token == NULL) {
        printf("Error: Line %d - Invalid CSV format.\n", line_no);
        errors++; 
        sprintf(op_errors[op_error_index].message, "Error: Line %d - %s", line_no, "Invalid CSV format.\n"); // Customize the message
        op_error_index++;
        return NULL;
    }
    transaction.account1 = atoi(token); 

    Account *acc1 = find_account(transaction.account1);
    if (acc1 == NULL) {
        printf("Error: Line %d - Account %d does not exist.\n", line_no, transaction.account1);
        errors++;
        sprintf(op_errors[op_error_index].message, "Error: Line %d - %s", line_no, "Account does not exist.\n"); // Customize the message
        op_error_index++;
        return NULL; 
    }

    // 3. Parse account2 (only for transfer)
    if (transaction.operation == TRANSFER) {
        token = strtok(NULL, ","); 
        if (token == NULL) {
            printf("Error: Line %d - Invalid CSV format.\n", line_no);
            
            errors++; 
            sprintf(op_errors[op_error_index].message, "Error: Line %d - %s", line_no, "Invalid CSV format.\n"); // Customize the message
            op_error_index++;
            return NULL;
        }
        transaction.account2 = atoi(token); 

        Account *acc2 = find_account(transaction.account2);
        if (acc1 == NULL) {
            printf("Error: Line %d - Account %d does not exist.\n", line_no, transaction.account1);
            errors++;
            return NULL; 
        }
    }

    // 4. Parse amount 
    token = strtok(NULL, ","); 
    if (token == NULL) {
        printf("Error: Line %d - Invalid CSV format.\n", line_no);
        errors++; 
        return NULL;
    }
    transaction.amount = atof(token); 
    if (transaction.amount <= 0 && transaction.operation == TRANSFER) {

        errors++;
        printf("Error: Line %d - Invalid amount.\n", line_no);
        sprintf(op_errors[op_error_index].message, "Error: Line %d - %s", line_no, "Invalid amount.\n"); // Customize the message
        op_error_index++;
        return NULL;
    }

    // 5. Additional Checks (Balance for withdrawal/transfer)
    if (transaction.operation == WITHDRAWAL || transaction.operation == TRANSFER) {
        Account *acc1 = find_account(transaction.account1);
        if (acc1 == NULL) {
            printf("Error: Line %d - Account %d does not exist.\n", line_no, transaction.account1);
            errors++;
            sprintf(op_errors[op_error_index].message, "Error: Line %d - %s", line_no, "Account does not exist.\n"); // Customize the message
            op_error_index++;
            return NULL;           }
        if (acc1->balance < transaction.amount) {
            printf("Error: Line %d - Insufficient balance in account %d.\n", line_no, transaction.account1);
            sprintf(op_errors[op_error_index].message, "Error: Line %d - %s", line_no, "Insufficient balance.\n"); // Customize the message
            op_error_index++;
            errors++;
            return NULL;
            }
    }

    // 6. Perform Transaction with Synchronization
    if (errors == 0) { 
        num_transactions++;        
        perform_transaction(&transaction); 
    }
    else {
        total_errors_transactions += errors; 
    }

    free(line); 
    free(data); 
    return NULL;
}




void load_transactions_from_csv(const char *file_path) {
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    int num_threads = 4; 
    pthread_t threads[num_threads]; 

    int i = 0;
    int line_no = 1; 
    while (getline(&line, &len, fp) != -1) {
        char *line_copy = malloc(strlen(line) + 1); 
        if (line_copy == NULL) {
            printf("Memory allocation error.\n");
            continue;
        }
        strcpy(line_copy, line); 

        ThreadData *data = malloc(sizeof(ThreadData));
        if (data == NULL) {
            printf("Memory allocation error.\n");
            free(line_copy);
            continue;
        }
        data->line = line_copy;
        data->line_no = line_no;

        if (pthread_create(&threads[i], NULL, process_transaction_line, (void *)data) != 0) {
            printf("Error creating thread.\n");
            free(line_copy);
            free(data);
            continue;
        }

        i++;
        line_no++;
        if (i == num_threads) { 
            for (int j = 0; j < num_threads; j++) {
                pthread_join(threads[j], NULL);
            }
            i = 0; 
        }
    }

    // Wait for any remaining threads
    for (int j = 0; j < i; j++) {
        pthread_join(threads[j], NULL);
    }
    
    fclose(fp);
    if (line) free(line); 
    printf("Loaded %d transactions from CSV.\n", num_transactions);
    // Generate Operations Report (Basic)
    time_t current_time;
    struct tm *time_info;
    char time_string[20]; 

    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "operaciones_%Y.log", time_info);

    FILE *report_file2 = fopen("operaciones.log", "w");
    if (report_file2 == NULL) {
        printf("Error creating report file.\n");
        return;
    }

    fprintf(report_file2, "Total errors: %d\n", total_errors_transactions);
    fprintf(report_file2, "Operations per thread: \n");
    fprintf(report_file2, "Thread #1: %d\n", thread_loads[0]);
    fprintf(report_file2, "Thread #2: %d\n", thread_loads[1]);
    fprintf(report_file2, "Thread #3: %d\n", thread_loads[2]);
    //fprintf(report_file2, "Thread #4: %d\n", thread_loads[3]);
    fprintf(report_file2, "Total: %d\n", num_transactions);
    fprintf(report_file2, "Withdrawals: %d\n", total_withdrawals);
    fprintf(report_file2, "Deposits: %d\n", total_deposits);
    fprintf(report_file2, "Transfers: %d\n", total_transfers);
    fprintf(report_file2, "Errors:\n"); 
    for (int i = 0; i < op_error_index; i++) {
        fprintf(report_file2, "%s\n", op_errors[i].message);
    }
    

    fclose(report_file2); 
}


Account *find_account(int account_no) {
    for (int i = 0; i < num_accounts; i++) {
        if (accounts[i].account_no == account_no) {
            return &accounts[i]; // Return a pointer to the found account
        }
    }
    return NULL; // Account not found
}

void perform_transaction(Transaction *transaction) {
    Account *acc1, *acc2;
    
    // 1. Find accounts (with synchronization if needed)
    acc1 = find_account(transaction->account1);
    if (acc1 == NULL) {

        // Handle error: Account 1 not found
        return; 
    }

    if (transaction->operation == TRANSFER) {
        acc2 = find_account(transaction->account2); 
        if (acc2 == NULL) {
            // Handle error: Account 2 not found
            return; 
        }
    }

    // 2. Synchronization: Protect shared account data
    //    - Use mutexes or semaphores. Example using a mutex for each account: 
    pthread_mutex_lock(&acc1->mutex); 
    if (transaction->operation == TRANSFER) {
        pthread_mutex_lock(&acc2->mutex); 
    }

    // 3. Perform the Transaction Logic
    switch (transaction->operation) {
        case DEPOSIT:
            acc1->balance += transaction->amount;
            total_deposits++;
            break;
        case WITHDRAWAL:
            if (acc1->balance >= transaction->amount) {
                acc1->balance -= transaction->amount;
                total_withdrawals++;
            } else {
                // Handle error: Insufficient balance
            }
            break;
        case TRANSFER:
            if (acc1->balance >= transaction->amount) {
                acc1->balance -= transaction->amount;
                acc2->balance += transaction->amount;
                total_transfers++;
            } else {
                // Handle error: Insufficient balance 
            }
            break;
    }

    // 4. Release Synchronization
    if (transaction->operation == TRANSFER) {
        pthread_mutex_unlock(&acc2->mutex); 
    }
    pthread_mutex_unlock(&acc1->mutex); 
}





// ----------------------------Funciones para el menu-------------------------------------




// Placeholder functions for now, implement later
void handle_deposit() {
    printf("insert the account number: ");
    int account_number;
    scanf("%d", &account_number);
    printf("insert the amount to deposit: ");
    double amount;
    scanf("%lf", &amount);
    Account *acc = find_account(account_number);
    if (acc == NULL) {
        printf("Account not found.\n");
        return;
    }
    pthread_mutex_lock(&acc->mutex);
    acc->balance += amount;
    pthread_mutex_unlock(&acc->mutex);
    printf("Deposit successful.\n");
}

void handle_withdrawal() {
    printf("Insert the account number: ");
    int account_number;
    scanf("%d", &account_number);
    printf("Insert the amount to withdraw: ");
    double amount;
    scanf("%lf", &amount);
    Account *acc = find_account(account_number);
    if (acc == NULL) {
        printf("Account not found.\n");
        return;
    }
    if (acc->balance < amount) {
        printf("Insufficient balance.\n");
        return;
    }
    pthread_mutex_lock(&acc->mutex);
    acc->balance -= amount;
    pthread_mutex_unlock(&acc->mutex);
    printf("Withdrawal successful.\n");
}

void handle_transaction() {
    printf("Insert the account number: ");
    int account_number;
    scanf("%d", &account_number);
    printf("Insert the account number to transfer:");
    int account_number2;
    scanf("%d", &account_number2);
    printf("Insert the amount to transfer: ");
    double amount;
    scanf("%lf", &amount);
    Account *acc = find_account(account_number);
    Account *acc2 = find_account(account_number2);
    if (acc == NULL) {
        printf("Account not found.\n");
        return;
    }
    if (acc2 == NULL) {
        printf("Account not found.\n");
        return;
    }
    if (acc->balance < amount) {
        printf("Insufficient balance.\n");
        return;
    }
    pthread_mutex_lock(&acc->mutex);
    pthread_mutex_lock(&acc2->mutex);
    acc->balance -= amount;
    acc2->balance += amount;
    pthread_mutex_unlock(&acc->mutex);
    pthread_mutex_unlock(&acc2->mutex);
    printf("Transaction successful.\n");
    
}

void handle_check_account() {
    printf("Insert the account number: ");
    int account_number;
    scanf("%d", &account_number);
    Account *acc = find_account(account_number);
    if (acc == NULL) {
        printf("Account not found.\n");
        return;
    }
    printf("Account number: %d\n", acc->account_no);
    printf("Name: %s\n", acc->name);
    printf("Balance: %.2lf\n", acc->balance);
    return;
}

void accounts_report() {
    printf("Accounts Report\n");
    for (int i = 0; i < num_accounts; i++) {
        printf("Account number: %d\n", accounts[i].account_no);
        printf("Name: %s\n", accounts[i].name);
        printf("Balance: %.2lf\n", accounts[i].balance);
        printf("------\n");
    }
    return;
}

int main() {
    int choice;

    while (1) { // Main menu loop
        display_menu();
        choice = get_user_input();

        if (choice == -1) {
            continue; // Skip to the next iteration if input was invalid
        }

        switch (choice) {
            case 1: handle_deposit();   break;
            case 2: handle_withdrawal(); break;
            case 3: handle_transaction(); break;
            case 4: handle_check_account(); break;
            case 5: exit(0); // Exit the program
            case 6: load_users_from_csv("users.csv"); break;
            case 7: load_transactions_from_csv("transactions.csv");break;
            case 8: accounts_report(); break;
            default: printf("Invalid choice.\n");
        }
    }

    return 0;
}

   
   