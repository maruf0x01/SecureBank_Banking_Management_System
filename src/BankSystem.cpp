#include "BankSystem.h"
#include "FileManager.h"

BankSystem::BankSystem(const string& accountsFile)
    : accountsFile(accountsFile)
{
    accounts = FileManager::loadAccounts(accountsFile);
}

string BankSystem::readRequiredText(const string& prompt)
{
    string value;

    while(value.empty() || value.find('|') != string::npos)
    {
        cout << prompt;
        getline(cin >> ws, value);
        if(value.empty() || value.find('|') != string::npos)
            cout << "Invalid input. The value cannot be empty or contain '|'.\n";
    }

    return value;
}

string BankSystem::readAccountType()
{
    const vector<string> accountTypes = {"Savings", "Current", "Student", "Business"};
    int choice;

    while(true)
    {
        cout << "\nSelect account type:\n"
             << "1. Savings\n"
             << "2. Current\n"
             << "3. Student\n"
             << "4. Business\n"
             << "Enter your choice: ";

        if(cin >> choice && choice >= 1 && choice <= static_cast<int>(accountTypes.size()))
            return accountTypes[choice - 1];

        cout << "Invalid account type selection.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

int BankSystem::readAccountNumber(const string& prompt)
{
    int accountNumber;

    while(true)
    {
        cout << prompt;
        if(cin >> accountNumber && accountNumber > 0)
            return accountNumber;

        cout << "Invalid account number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

int BankSystem::findAccountIndex(int accountNumber) const
{
    for(int i = 0; i < static_cast<int>(accounts.size()); i++)
        if(accounts[i].getAccountNumber() == accountNumber)
            return i;

    return -1;
}

void BankSystem::printAccount(const Account& account) const
{
    cout << "Account Number: " << account.getAccountNumber() << '\n'
         << "Customer Name: " << account.getCustomerName() << '\n'
         << "Phone Number: " << account.getPhoneNumber() << '\n'
         << "Account Type: " << account.getAccountType() << '\n'
         << "Balance: " << fixed << setprecision(2) << account.getBalance() << '\n'
         << "Status: " << account.getStatus() << "\n\n";
}

bool BankSystem::saveData()
{
    return FileManager::saveAccounts(accountsFile, accounts);
}

void BankSystem::createAccount()
{
    int nextAccountNumber = 100001;
    for(const Account& account : accounts)
        nextAccountNumber = max(nextAccountNumber, account.getAccountNumber() + 1);

    string customerName = readRequiredText("Customer name: ");
    string phoneNumber = readRequiredText("Phone number: ");
    string accountType = readAccountType();
    Account account(nextAccountNumber, customerName, phoneNumber, accountType, 0.0, "Active");
    accounts.push_back(account);

    if(saveData())
        cout << "Account created successfully. Account number: " << account.getAccountNumber() << '\n';
    else
    {
        accounts.pop_back();
        cout << "Unable to save account data.\n";
    }
}

void BankSystem::viewAccounts() const
{
    if(accounts.empty())
    {
        cout << "No accounts found.\n";
        return;
    }

    for(const Account& account : accounts)
        printAccount(account);
}

void BankSystem::searchAccount() const
{
    string searchText;
    cout << "Enter account number or customer name: ";
    cin >> ws;
    getline(cin, searchText);

    bool found = false;
    for(const Account& account : accounts)
    {
        if(to_string(account.getAccountNumber()) == searchText ||
           account.getCustomerName().find(searchText) != string::npos)
        {
            printAccount(account);
            found = true;
        }
    }

    if(!found)
        cout << "No matching account found.\n";
}

void BankSystem::updateAccount()
{
    int accountNumber = readAccountNumber("Enter account number to update: ");
    int index = findAccountIndex(accountNumber);

    if(index == -1)
    {
        cout << "Account not found.\n";
        return;
    }

    Account& account = accounts[index];
    account.setCustomerName(readRequiredText("New customer name: "));
    account.setPhoneNumber(readRequiredText("New phone number: "));
    account.setAccountType(readAccountType());

    if(saveData())
        cout << "Account updated successfully.\n";
    else
        cout << "Unable to save account data.\n";
}

void BankSystem::closeAccount()
{
    int accountNumber = readAccountNumber("Enter account number to close: ");
    int index = findAccountIndex(accountNumber);

    if(index == -1)
    {
        cout << "Account not found.\n";
        return;
    }

    if(accounts[index].getStatus() == "Closed")
    {
        cout << "Account is already closed.\n";
        return;
    }

    accounts[index].setStatus("Closed");
    if(saveData())
        cout << "Account closed successfully.\n";
    else
    {
        accounts[index].setStatus("Active");
        cout << "Unable to save account data.\n";
    }
}

void BankSystem::reopenAccount()
{
    int accountNumber = readAccountNumber("Enter account number to reopen: ");
    int index = findAccountIndex(accountNumber);

    if(index == -1)
    {
        cout << "Account not found.\n";
        return;
    }

    if(accounts[index].getStatus() == "Active")
    {
        cout << "Account is already active.\n";
        return;
    }

    accounts[index].setStatus("Active");
    if(saveData())
        cout << "Account reopened successfully.\n";
    else
    {
        accounts[index].setStatus("Closed");
        cout << "Unable to save account data.\n";
    }
}
