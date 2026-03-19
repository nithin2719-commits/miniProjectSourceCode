#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ACCOUNTS 100
#define MIN_BALANCE 0.0

struct client_data
{
    unsigned int acct_num;
    char last_name[15];
    char first_name[10];
    double balance;
};

unsigned int enter_choice(void);
void export_accounts_file(FILE *data_file);
void update_record(FILE *data_file);
void add_record(FILE *data_file);
void delete_record(FILE *data_file);
void transfer_funds(FILE *data_file);
void list_accounts(FILE *data_file);
void view_account(FILE *data_file);
void show_summary(FILE *data_file);
void list_accounts_sorted_by_balance(FILE *data_file);

unsigned int read_unsigned_in_range(const char *prompt, unsigned int min, unsigned int max);
double read_double_value(const char *prompt);
double read_non_negative_double(const char *prompt);
void clear_input_buffer(void);

long account_offset(unsigned int account_num);
void ensure_data_file_initialized(FILE *data_file);
void sanitize_data_file(FILE *data_file);
int read_account_slot(FILE *data_file, unsigned int account_num, struct client_data *client);
int write_account_slot(FILE *data_file, unsigned int account_num, const struct client_data *client);
int is_valid_account_record(unsigned int account_num, const struct client_data *client);
void blank_account(struct client_data *client);

void log_transaction(const char *action, const struct client_data *client, double amount, const char *note);

int compare_balance_asc(const void *left, const void *right);

int main(int argc, char *argv[])
{
    FILE *data_file;
    unsigned int choice;

    if ((data_file = fopen("credit.dat", "rb+")) == NULL)
    {
        if ((data_file = fopen("credit.dat", "wb+")) == NULL)
        {
            printf("%s: File could not be opened.\n", argv[0]);
            return -1;
        }
    }

    ensure_data_file_initialized(data_file);
    sanitize_data_file(data_file);

    while (1)
    {
        choice = enter_choice();

        if (choice == 0 || choice == 10)
        {
            break;
        }

        switch (choice)
        {
        case 1:
            export_accounts_file(data_file);
            puts("accounts.txt generated.");
            break;
        case 2:
            update_record(data_file);
            break;
        case 3:
            add_record(data_file);
            break;
        case 4:
            delete_record(data_file);
            break;
        case 5:
            transfer_funds(data_file);
            break;
        case 6:
            list_accounts(data_file);
            break;
        case 7:
            view_account(data_file);
            break;
        case 8:
            show_summary(data_file);
            break;
        case 9:
            list_accounts_sorted_by_balance(data_file);
            break;
        default:
            puts("Incorrect choice.");
            break;
        }

        puts("");
    }

    fclose(data_file);
    return 0;
}

unsigned int enter_choice(void)
{
    printf("%s", "\nEnter your choice\n"
                 "1 - print/export accounts.txt\n"
                 "2 - update account data\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - transfer funds\n"
                 "6 - list all active accounts\n"
                 "7 - view one account\n"
                 "8 - transaction summary\n"
                 "9 - list accounts sorted by balance\n"
                 "10 - end program\n");

    return read_unsigned_in_range("? ", 1, 10);
}

void export_accounts_file(FILE *data_file)
{
    FILE *write_file;
    struct client_data client;
    unsigned int account_num;

    if ((write_file = fopen("accounts.txt", "w")) == NULL)
    {
        puts("accounts.txt could not be opened.");
        return;
    }

    fprintf(write_file, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    for (account_num = 1; account_num <= MAX_ACCOUNTS; ++account_num)
    {
        if (read_account_slot(data_file, account_num, &client) == 1)
        {
            fprintf(write_file,
                    "%-6u%-16s%-11s%10.2f\n",
                    client.acct_num,
                    client.last_name,
                    client.first_name,
                    client.balance);
        }
    }

    fclose(write_file);
}

void update_record(FILE *data_file)
{
    unsigned int account_num;
    double transaction;
    double new_balance;
    struct client_data client;

    account_num = read_unsigned_in_range("Enter account to update (1 - 100): ", 1, MAX_ACCOUNTS);
    if (read_account_slot(data_file, account_num, &client) == 0)
    {
        printf("Account #%u has no information.\n", account_num);
        return;
    }

    printf("Current: %-6u%-16s%-11s%10.2f\n", client.acct_num, client.last_name, client.first_name, client.balance);

    transaction = read_double_value("Enter charge (+) or payment (-): ");
    new_balance = client.balance + transaction;

    if (new_balance < MIN_BALANCE)
    {
        puts("Transaction denied: insufficient balance (no negative balance allowed).");
        return;
    }

    client.balance = new_balance;
    write_account_slot(data_file, account_num, &client);
    printf("Updated: %-6u%-16s%-11s%10.2f\n", client.acct_num, client.last_name, client.first_name, client.balance);
    log_transaction("UPDATE", &client, transaction, "Balance adjusted");
}

void add_record(FILE *data_file)
{
    unsigned int account_num;
    struct client_data client;

    account_num = read_unsigned_in_range("Enter new account number (1 - 100): ", 1, MAX_ACCOUNTS);

    if (read_account_slot(data_file, account_num, &client) == 1)
    {
        printf("Account #%u already contains information.\n", account_num);
        return;
    }

    blank_account(&client);
    client.acct_num = account_num;

    printf("Enter last name: ");
    while (scanf("%14s", client.last_name) != 1)
    {
        puts("Invalid input. Try again.");
        clear_input_buffer();
        printf("Enter last name: ");
    }

    printf("Enter first name: ");
    while (scanf("%9s", client.first_name) != 1)
    {
        puts("Invalid input. Try again.");
        clear_input_buffer();
        printf("Enter first name: ");
    }

    client.balance = read_non_negative_double("Enter opening balance: ");
    write_account_slot(data_file, account_num, &client);

    printf("Account #%u added successfully.\n", account_num);
    log_transaction("ADD", &client, client.balance, "New account created");
}

void delete_record(FILE *data_file)
{
    unsigned int account_num;
    struct client_data client;
    struct client_data blank_client;

    account_num = read_unsigned_in_range("Enter account number to delete (1 - 100): ", 1, MAX_ACCOUNTS);
    if (read_account_slot(data_file, account_num, &client) == 0)
    {
        printf("Account #%u does not exist.\n", account_num);
        return;
    }

    blank_account(&blank_client);
    write_account_slot(data_file, account_num, &blank_client);
    printf("Account #%u deleted successfully.\n", account_num);
    log_transaction("DELETE", &client, 0.0, "Account removed");
}

void transfer_funds(FILE *data_file)
{
    unsigned int from_account;
    unsigned int to_account;
    double amount;
    struct client_data from_client;
    struct client_data to_client;

    from_account = read_unsigned_in_range("Transfer from account (1 - 100): ", 1, MAX_ACCOUNTS);
    to_account = read_unsigned_in_range("Transfer to account (1 - 100): ", 1, MAX_ACCOUNTS);

    if (from_account == to_account)
    {
        puts("Source and destination accounts must be different.");
        return;
    }

    if (read_account_slot(data_file, from_account, &from_client) == 0)
    {
        printf("Account #%u has no information.\n", from_account);
        return;
    }

    if (read_account_slot(data_file, to_account, &to_client) == 0)
    {
        printf("Account #%u has no information.\n", to_account);
        return;
    }

    amount = read_non_negative_double("Enter transfer amount: ");
    if (from_client.balance - amount < MIN_BALANCE)
    {
        puts("Transfer denied: insufficient balance (no negative balance allowed).");
        return;
    }

    from_client.balance -= amount;
    to_client.balance += amount;

    write_account_slot(data_file, from_account, &from_client);
    write_account_slot(data_file, to_account, &to_client);

    puts("Transfer completed successfully.");
    log_transaction("TRANSFER_OUT", &from_client, amount, "Transferred to another account");
    log_transaction("TRANSFER_IN", &to_client, amount, "Received from another account");
}

void list_accounts(FILE *data_file)
{
    struct client_data client;
    unsigned int account_num;
    int found = 0;

    puts("");
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    for (account_num = 1; account_num <= MAX_ACCOUNTS; ++account_num)
    {
        if (read_account_slot(data_file, account_num, &client) == 1)
        {
            found = 1;
            printf("%-6u%-16s%-11s%10.2f\n", client.acct_num, client.last_name, client.first_name, client.balance);
        }
    }

    if (!found)
    {
        puts("No active accounts found.");
    }
}

void view_account(FILE *data_file)
{
    unsigned int account_num;
    struct client_data client;

    account_num = read_unsigned_in_range("Enter account to view (1 - 100): ", 1, MAX_ACCOUNTS);
    if (read_account_slot(data_file, account_num, &client) == 0)
    {
        printf("Account #%u has no information.\n", account_num);
        return;
    }

    printf("\nAccount Details\n");
    printf("Account No : %u\n", client.acct_num);
    printf("Last Name  : %s\n", client.last_name);
    printf("First Name : %s\n", client.first_name);
    printf("Balance    : %.2f\n", client.balance);
}

void show_summary(FILE *data_file)
{
    struct client_data client;
    unsigned int account_num;
    int active_count = 0;
    double total_balance = 0.0;
    double min_balance = 0.0;
    double max_balance = 0.0;

    for (account_num = 1; account_num <= MAX_ACCOUNTS; ++account_num)
    {
        if (read_account_slot(data_file, account_num, &client) == 1)
        {
            if (active_count == 0)
            {
                min_balance = client.balance;
                max_balance = client.balance;
            }
            else
            {
                if (client.balance < min_balance)
                {
                    min_balance = client.balance;
                }
                if (client.balance > max_balance)
                {
                    max_balance = client.balance;
                }
            }

            total_balance += client.balance;
            active_count++;
        }
    }

    printf("\nTransaction Summary\n");
    printf("Active Accounts : %d\n", active_count);
    printf("Total Balance   : %.2f\n", total_balance);

    if (active_count > 0)
    {
        printf("Average Balance : %.2f\n", total_balance / active_count);
        printf("Minimum Balance : %.2f\n", min_balance);
        printf("Maximum Balance : %.2f\n", max_balance);
    }
    else
    {
        puts("No active accounts found.");
    }
}

void list_accounts_sorted_by_balance(FILE *data_file)
{
    struct client_data clients[MAX_ACCOUNTS];
    struct client_data client;
    unsigned int account_num;
    int count = 0;
    int i;

    for (account_num = 1; account_num <= MAX_ACCOUNTS; ++account_num)
    {
        if (read_account_slot(data_file, account_num, &client) == 1)
        {
            clients[count] = client;
            count++;
        }
    }

    if (count == 0)
    {
        puts("No active accounts found.");
        return;
    }

    qsort(clients, (size_t)count, sizeof(struct client_data), compare_balance_asc);

    puts("\nAccounts Sorted by Balance (asc)");
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    for (i = 0; i < count; ++i)
    {
        printf("%-6u%-16s%-11s%10.2f\n",
               clients[i].acct_num,
               clients[i].last_name,
               clients[i].first_name,
               clients[i].balance);
    }
}

unsigned int read_unsigned_in_range(const char *prompt, unsigned int min, unsigned int max)
{
    unsigned int value;
    int status;

    while (1)
    {
        printf("%s", prompt);
        status = scanf("%u", &value);

        if (status == EOF)
        {
            puts("");
            return 0;
        }

        if (status == 1 && value >= min && value <= max)
        {
            return value;
        }

        puts("Invalid input. Please enter a valid number in range.");
        clear_input_buffer();
    }
}

double read_double_value(const char *prompt)
{
    double value;
    int status;

    while (1)
    {
        printf("%s", prompt);
        status = scanf("%lf", &value);

        if (status == EOF)
        {
            puts("");
            return 0.0;
        }

        if (status == 1)
        {
            return value;
        }

        puts("Invalid amount. Try again.");
        clear_input_buffer();
    }
}

double read_non_negative_double(const char *prompt)
{
    double value;

    while (1)
    {
        value = read_double_value(prompt);
        if (value >= 0.0)
        {
            return value;
        }

        puts("Invalid amount. Please enter a non-negative number.");
    }
}

void clear_input_buffer(void)
{
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF)
    {
    }
}

long account_offset(unsigned int account_num)
{
    return (long)(account_num - 1) * (long)sizeof(struct client_data);
}

void ensure_data_file_initialized(FILE *data_file)
{
    long expected_size = (long)MAX_ACCOUNTS * (long)sizeof(struct client_data);
    long current_size;
    struct client_data blank_client;
    unsigned int i;

    fseek(data_file, 0L, SEEK_END);
    current_size = ftell(data_file);

    if (current_size >= expected_size)
    {
        rewind(data_file);
        return;
    }

    blank_account(&blank_client);
    rewind(data_file);

    for (i = 0; i < MAX_ACCOUNTS; ++i)
    {
        fwrite(&blank_client, sizeof(struct client_data), 1, data_file);
    }

    fflush(data_file);
    rewind(data_file);
}

void sanitize_data_file(FILE *data_file)
{
    unsigned int account_num;
    struct client_data client;
    struct client_data blank_client;

    blank_account(&blank_client);

    for (account_num = 1; account_num <= MAX_ACCOUNTS; ++account_num)
    {
        fseek(data_file, account_offset(account_num), SEEK_SET);
        if (fread(&client, sizeof(struct client_data), 1, data_file) != 1)
        {
            continue;
        }

        if (client.acct_num != 0 && !is_valid_account_record(account_num, &client))
        {
            fseek(data_file, account_offset(account_num), SEEK_SET);
            fwrite(&blank_client, sizeof(struct client_data), 1, data_file);
        }
    }

    fflush(data_file);
}

int read_account_slot(FILE *data_file, unsigned int account_num, struct client_data *client)
{
    if (account_num < 1 || account_num > MAX_ACCOUNTS)
    {
        return 0;
    }

    fseek(data_file, account_offset(account_num), SEEK_SET);
    if (fread(client, sizeof(struct client_data), 1, data_file) != 1)
    {
        return 0;
    }

    if (!is_valid_account_record(account_num, client))
    {
        return 0;
    }

    return 1;
}

int write_account_slot(FILE *data_file, unsigned int account_num, const struct client_data *client)
{
    struct client_data copy = *client;

    if (account_num < 1 || account_num > MAX_ACCOUNTS)
    {
        return 0;
    }

    if (copy.acct_num != 0)
    {
        copy.acct_num = account_num;
    }

    fseek(data_file, account_offset(account_num), SEEK_SET);
    if (fwrite(&copy, sizeof(struct client_data), 1, data_file) != 1)
    {
        return 0;
    }

    fflush(data_file);
    return 1;
}

int is_valid_account_record(unsigned int account_num, const struct client_data *client)
{
    if (client->acct_num == 0)
    {
        return 0;
    }

    if (client->acct_num != account_num)
    {
        return 0;
    }

    return 1;
}

void blank_account(struct client_data *client)
{
    memset(client, 0, sizeof(struct client_data));
}

void log_transaction(const char *action, const struct client_data *client, double amount, const char *note)
{
    FILE *log_file = fopen("transactions.log", "a");

    if (log_file == NULL)
    {
        return;
    }

    fprintf(log_file,
            "ACTION=%s ACCT=%u NAME=%s,%s AMOUNT=%.2f BALANCE=%.2f NOTE=%s\n",
            action,
            client->acct_num,
            client->last_name,
            client->first_name,
            amount,
            client->balance,
            note);

    fclose(log_file);
}

int compare_balance_asc(const void *left, const void *right)
{
    const struct client_data *a = (const struct client_data *)left;
    const struct client_data *b = (const struct client_data *)right;

    if (a->balance < b->balance)
    {
        return -1;
    }
    if (a->balance > b->balance)
    {
        return 1;
    }

    if (a->acct_num < b->acct_num)
    {
        return -1;
    }
    if (a->acct_num > b->acct_num)
    {
        return 1;
    }

    return 0;
}
