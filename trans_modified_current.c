// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#define MAX_ACCOUNTS 100
// clientData structure definition
struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
};                        // end structure clientData

// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void listAccounts(FILE *fPtr);
void viewAccount(FILE *fPtr);
void transferFunds(FILE *fPtr);
void showSummary(FILE *fPtr);
void logTransaction(const char *action, const struct clientData *client, double amount, const char *note);
unsigned int readUnsignedInRange(const char *prompt, unsigned int min, unsigned int max);
double readNonNegativeDouble(const char *prompt);
void clearInputBuffer(void);

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice
    struct clientData blankClient = {0, "", "", 0.0};
    int i;

    // fopen opens the file for update; if it does not exist create it
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        if ((cfPtr = fopen("credit.dat", "wb+")) == NULL)
        {
            printf("%s: File could not be opened.\n", argv[0]);
            exit(-1);
        }

        for (i = 0; i < MAX_ACCOUNTS; ++i)
        {
            fwrite(&blankClient, sizeof(struct clientData), 1, cfPtr);
        }

        fflush(cfPtr);
        puts("Initialized new data file: credit.dat");
    }

    // enable user to specify action
    while (1)
    {
        choice = enterChoice();

        if (choice == 0 || choice == 9)
        {
            break;
        }

        switch (choice)
        {
        // print/export accounts file
        case 1:
            textFile(cfPtr);
            puts("accounts.txt generated.");
            puts("");
            break;
        // update account balance
        case 2:
            updateRecord(cfPtr);
            puts("");
            break;
        // add account
        case 3:
            newRecord(cfPtr);
            puts("");
            break;
        // delete account
        case 4:
            deleteRecord(cfPtr);
            puts("");
            break;
        // transfer funds
        case 5:
            transferFunds(cfPtr);
            puts("");
            break;
        // list accounts
        case 6:
            listAccounts(cfPtr);
            puts("");
            break;
        // view single account
        case 7:
            viewAccount(cfPtr);
            puts("");
            break;
        // show summary
        case 8:
            showSummary(cfPtr);
            puts("");
            break;
        // display if user does not select valid choice
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    }     // end while

    fclose(cfPtr); // fclose closes the file
} // end main

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    int result;     // used to test whether fread read any bytes
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } // end if
    else
    {
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        // copy all records from random-access file into text file
        while ((result = fread(&client, sizeof(struct clientData), 1, readPtr)) == 1)
        {
            // write single record to text file
            if (client.acctNum >= 1 && client.acctNum <= MAX_ACCOUNTS)
            {
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                        client.balance);
            } // end if
        }     // end while

        fclose(writePtr); // fclose closes the file
    }                     // end else
} // end function textFile

// update balance in existing record
void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;
    struct clientData client = {0, "", "", 0.0};

    account = readUnsignedInRange("Enter account to update ( 1 - 100 ): ", 1, MAX_ACCOUNTS);
    if (account == 0)
    {
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
        return;
    }

    printf("Current: %-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

    while (1)
    {
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        if (scanf("%lf", &transaction) == 1)
        {
            break;
        }

        puts("Invalid amount. Try again.");
        clearInputBuffer();
    }

    client.balance += transaction;

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    printf("Updated: %-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
    logTransaction("UPDATE", &client, transaction, "Balance adjusted");
}

// create and insert record
void newRecord(FILE *fPtr)
{
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum; // account number

    // obtain number of account to create
    accountNum = readUnsignedInRange("Enter new account number ( 1 - 100 ): ", 1, MAX_ACCOUNTS);
    if (accountNum == 0)
    {
        return;
    }

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account already exists
    if (client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
    } // end if
    else
    { // create record
        // user enters last name, first name and balance
        printf("%s", "Enter last name: ");
        while (scanf("%14s", client.lastName) != 1)
        {
            puts("Invalid last name. Try again.");
            clearInputBuffer();
            printf("%s", "Enter last name: ");
        }

        printf("%s", "Enter first name: ");
        while (scanf("%9s", client.firstName) != 1)
        {
            puts("Invalid first name. Try again.");
            clearInputBuffer();
            printf("%s", "Enter first name: ");
        }

        client.balance = readNonNegativeDouble("Enter opening balance: ");

        client.acctNum = accountNum;
        // move file pointer to correct record in file
        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        // insert record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
        logTransaction("ADD", &client, client.balance, "New account created");
    } // end else
} // end function newRecord

void deleteRecord(FILE *fPtr)
{
    unsigned int accountNum;
    struct clientData client = {0, "", "", 0.0};
    struct clientData blankClient = {0, "", "", 0.0};

    accountNum = readUnsignedInRange("Enter account number to delete ( 1 - 100 ): ", 1, MAX_ACCOUNTS);
    if (accountNum == 0)
    {
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account #%u does not exist.\n", accountNum);
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    printf("Account #%u deleted successfully.\n", accountNum);
    logTransaction("DELETE", &client, 0.0, "Account removed");
}

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    // display available options
    printf("%s", "\nEnter your choice\n"
                 "1 - print/export accounts.txt\n"
                 "2 - update account data\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - transfer funds\n"
                 "6 - list all active accounts\n"
                 "7 - view one account\n"
                 "8 - transaction summary\n"
                 "9 - end program\n");

    menuChoice = readUnsignedInRange("? ", 1, 9); // receive choice from user
    return menuChoice;
} // end function enterChoice

void listAccounts(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    int found = 0;

    rewind(fPtr);
    puts("");
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum >= 1 && client.acctNum <= MAX_ACCOUNTS)
        {
            found = 1;
            printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
        }
    }

    if (!found)
    {
        puts("No active accounts found.");
    }
}

void transferFunds(FILE *fPtr)
{
    unsigned int fromAccount;
    unsigned int toAccount;
    double amount;
    struct clientData fromClient = {0, "", "", 0.0};
    struct clientData toClient = {0, "", "", 0.0};

    fromAccount = readUnsignedInRange("Transfer from account ( 1 - 100 ): ", 1, MAX_ACCOUNTS);
    toAccount = readUnsignedInRange("Transfer to account ( 1 - 100 ): ", 1, MAX_ACCOUNTS);
    if (fromAccount == 0 || toAccount == 0)
    {
        return;
    }

    if (fromAccount == toAccount)
    {
        puts("Source and destination accounts must be different.");
        return;
    }

    fseek(fPtr, (fromAccount - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&fromClient, sizeof(struct clientData), 1, fPtr);

    fseek(fPtr, (toAccount - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&toClient, sizeof(struct clientData), 1, fPtr);

    if (fromClient.acctNum == 0)
    {
        printf("Account #%u has no information.\n", fromAccount);
        return;
    }

    if (toClient.acctNum == 0)
    {
        printf("Account #%u has no information.\n", toAccount);
        return;
    }

    amount = readNonNegativeDouble("Enter transfer amount: ");
    if (amount < 0.0)
    {
        return;
    }

    if (amount > fromClient.balance)
    {
        puts("Insufficient balance for transfer.");
        return;
    }

    fromClient.balance -= amount;
    toClient.balance += amount;

    fseek(fPtr, (fromAccount - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&fromClient, sizeof(struct clientData), 1, fPtr);

    fseek(fPtr, (toAccount - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&toClient, sizeof(struct clientData), 1, fPtr);

    puts("Transfer completed successfully.");
    logTransaction("TRANSFER_OUT", &fromClient, amount, "Transferred to another account");
    logTransaction("TRANSFER_IN", &toClient, amount, "Received from another account");
}

void viewAccount(FILE *fPtr)
{
    unsigned int account;
    struct clientData client = {0, "", "", 0.0};

    account = readUnsignedInRange("Enter account to view ( 1 - 100 ): ", 1, MAX_ACCOUNTS);
    if (account == 0)
    {
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
        return;
    }

    printf("\nAccount Details\n");
    printf("Account No : %u\n", client.acctNum);
    printf("Last Name  : %s\n", client.lastName);
    printf("First Name : %s\n", client.firstName);
    printf("Balance    : %.2f\n", client.balance);
}

void showSummary(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    int activeCount = 0;
    double totalBalance = 0.0;
    double minBalance = 0.0;
    double maxBalance = 0.0;

    rewind(fPtr);

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum >= 1 && client.acctNum <= MAX_ACCOUNTS)
        {
            totalBalance += client.balance;

            if (activeCount == 0)
            {
                minBalance = client.balance;
                maxBalance = client.balance;
            }
            else
            {
                if (client.balance < minBalance)
                {
                    minBalance = client.balance;
                }
                if (client.balance > maxBalance)
                {
                    maxBalance = client.balance;
                }
            }

            activeCount++;
        }
    }

    printf("\nTransaction Summary\n");
    printf("Active Accounts : %d\n", activeCount);
    printf("Total Balance   : %.2f\n", totalBalance);

    if (activeCount > 0)
    {
        printf("Average Balance : %.2f\n", totalBalance / activeCount);
        printf("Minimum Balance : %.2f\n", minBalance);
        printf("Maximum Balance : %.2f\n", maxBalance);
    }
    else
    {
        puts("No active accounts found.");
    }
}

void logTransaction(const char *action, const struct clientData *client, double amount, const char *note)
{
    FILE *logPtr;

    logPtr = fopen("transactions.log", "a");
    if (logPtr == NULL)
    {
        return;
    }

    fprintf(logPtr,
            "ACTION=%s ACCT=%u NAME=%s,%s AMOUNT=%.2f BALANCE=%.2f NOTE=%s\n",
            action,
            client->acctNum,
            client->lastName,
            client->firstName,
            amount,
            client->balance,
            note);

    fclose(logPtr);
}

unsigned int readUnsignedInRange(const char *prompt, unsigned int min, unsigned int max)
{
    unsigned int value;
    int scanResult;

    while (1)
    {
        printf("%s", prompt);

        scanResult = scanf("%u", &value);

        if (scanResult == EOF)
        {
            puts("");
            return 0;
        }

        if (scanResult == 1 && value >= min && value <= max)
        {
            return value;
        }

        puts("Invalid input. Please enter a valid number in range.");
        clearInputBuffer();
    }
}

double readNonNegativeDouble(const char *prompt)
{
    double value;
    int scanResult;

    while (1)
    {
        printf("%s", prompt);

        scanResult = scanf("%lf", &value);

        if (scanResult == EOF)
        {
            puts("");
            return -1.0;
        }

        if (scanResult == 1 && value >= 0.0)
        {
            return value;
        }

        puts("Invalid amount. Please enter a non-negative number.");
        clearInputBuffer();
    }
}

void clearInputBuffer(void)
{
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF)
    {
    }
}