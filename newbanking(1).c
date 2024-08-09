#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_ACCOUNTS 3
#define NUM_THREADS 10
#define NAME_LENGTH 200

typedef struct {
    int id;
    char name[NAME_LENGTH];
    int balance;
    int deposit_count;  // Counter for deposits
    int withdraw_count; // Counter for withdrawals
    int total_deposited; // Total amount deposited
    int total_withdrawn; // Total amount withdrawn
    pthread_mutex_t mutex;
} Account;

Account accounts[NUM_ACCOUNTS];
int total_deposited = 0; // Global variable to keep track of total deposited amount
int total_withdrawn = 0; // Global variable to keep track of total withdrawn amount
pthread_mutex_t total_deposited_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for accessing total_deposited
pthread_mutex_t total_withdrawn_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for accessing total_withdrawn

// Initialize accounts with default values, names, and counters
void initialize_accounts() {
    const char* names[NUM_ACCOUNTS] = {"DEEPAK", "RAMESH", "SURESH"};

    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].id = i;
        strncpy(accounts[i].name, names[i], NAME_LENGTH - 1);
        accounts[i].name[NAME_LENGTH - 1] = '\0'; // Ensure null-termination
        accounts[i].balance = 1000; // Initial balance
        accounts[i].deposit_count = 0;  // Initialize deposit counter
        accounts[i].withdraw_count = 0; // Initialize withdrawal counter
        accounts[i].total_deposited = 0; // Initialize total deposited amount
        accounts[i].total_withdrawn = 0; // Initialize total withdrawn amount
        pthread_mutex_init(&accounts[i].mutex, NULL);
    }
}

// Deposit function with mutex locking
void deposit(Account *account, int amount) {
    pthread_mutex_lock(&account->mutex);
    account->balance += amount;
    account->deposit_count++; // Increment deposit counter
    account->total_deposited += amount; // Update total deposited amount
    printf("Deposited %d Rs to account %d (%s). New balance: %d Rs\n", amount, account->id, account->name, account->balance);

    // Update the global total deposited amount
    pthread_mutex_lock(&total_deposited_mutex);
    total_deposited += amount;
    pthread_mutex_unlock(&total_deposited_mutex);

    pthread_mutex_unlock(&account->mutex);
}

// Withdraw function with mutex locking
void withdraw(Account *account, int amount) {
    pthread_mutex_lock(&account->mutex);
    if (account->balance >= amount) {
        account->balance -= amount;
        account->withdraw_count++; // Increment withdrawal counter
        account->total_withdrawn += amount; // Update total withdrawn amount
        printf("Withdrew %d Rs from account %d (%s). New balance: %d Rs\n", amount, account->id, account->name, account->balance);

        // Update the global total withdrawn amount
        pthread_mutex_lock(&total_withdrawn_mutex);
        total_withdrawn += amount;
        pthread_mutex_unlock(&total_withdrawn_mutex);
    } else {
        printf("Insufficient funds in account %d (%s)\n", account->id, account->name);
    }
    pthread_mutex_unlock(&account->mutex);
}

// Thread function to perform random transactions
void* transaction(void *arg) {
    int account_id = *(int*)arg;
    free(arg);

    // Randomly deposit or withdraw
    int action = rand() % 2; // 0 for deposit, 1 for withdraw
    int amount = (rand() % 100) + 1; // Random amount between 1 and 100

    if (action == 0) {
        deposit(&accounts[account_id], amount);
    } else {
        withdraw(&accounts[account_id], amount);
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];

    initialize_accounts();

    // Seed random number generator
    srand(time(NULL));

    for (int i = 0; i < NUM_THREADS; i++) {
        int *account_id = malloc(sizeof(int));
        if (account_id == NULL) {
            perror("Failed to allocate memory");
            return 1;
        }
        *account_id = rand() % NUM_ACCOUNTS; // Randomly select an account
        if (pthread_create(&threads[i], NULL, transaction, account_id) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Failed to join thread");
            return 1;
        }
    }

    // Print total deposited and withdrawn amounts
    printf("Total amount deposited: %d Rs\n", total_deposited);
    printf("Total amount withdrawn: %d Rs\n", total_withdrawn);

    // Print deposit and withdrawal counts and totals for each account
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d (%s) - Deposits: %d, Total Deposited: %d Rs, Withdrawals: %d, Total Withdrawn: %d Rs\n", 
               accounts[i].id, accounts[i].name, 
               accounts[i].deposit_count, accounts[i].total_deposited, 
               accounts[i].withdraw_count, accounts[i].total_withdrawn);
    }

    // Cleanup
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].mutex);
    }
    pthread_mutex_destroy(&total_deposited_mutex);
    pthread_mutex_destroy(&total_withdrawn_mutex);

    return 0;
}













