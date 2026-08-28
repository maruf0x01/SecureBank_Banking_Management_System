#include "BankSystem.h"
#include "FileManager.h"

BankSystem::BankSystem(const string& accountsFile)
    : accountsFile(accountsFile), transactionsFile("data/transactions.txt")
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

string BankSystem::readOptionalText(const string& prompt)
{
    string value;

    while(true)
    {
        cout << prompt;
        getline(cin, value);
        if(value.empty() || value.find('|') == string::npos)
            return value;

        cout << "Invalid input. The value cannot contain '|'.\n";
    }
}

string BankSystem::readAccountType(bool allowSkip)
{
    const vector<string> accountTypes = {"Savings", "Current", "Student", "Business"};
    string input;

    while(true)
    {
        cout << "\nSelect account type:\n"
             << "1. Savings\n"
             << "2. Current\n"
             << "3. Student\n"
             << "4. Business\n"
             << (allowSkip ? "Enter your choice or press Enter to keep current type: " : "Enter your choice: ");

        getline(cin, input);
        if(allowSkip && input.empty())
            return "";

        try
        {
            int choice = stoi(input);
            if(choice >= 1 && choice <= static_cast<int>(accountTypes.size()))
                return accountTypes[choice - 1];
        }
        catch(...)
        {
        }

        cout << "Invalid account type selection.\n";
    }
}

int BankSystem::readAccountNumber(const string& prompt) const
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

long long BankSystem::readAmount(const string& prompt)
{
    string input;

    while(true)
    {
        cout << prompt;
        cin >> input;
        try
        {
            long long amountPaisa = parseMoneyToPaisa(input);
            if(amountPaisa > 0)
                return amountPaisa;
        }
        catch(...)
        {
        }

        cout << "Invalid amount. Enter a positive amount with up to two decimals.\n";
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
         << "Balance: " << formatPaisa(account.getBalancePaisa()) << '\n'
         << "Status: " << account.getStatus() << "\n\n";
}

bool BankSystem::saveData()
{
    return FileManager::saveAccounts(accountsFile, accounts);
}

Transaction BankSystem::createTransaction(int accountNumber, const string& type,
                                          long long amountPaisa, int relatedAccountNumber) const
{
    auto currentTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
    string timeText = ctime(&currentTime);
    timeText.erase(remove(timeText.begin(), timeText.end(), '\n'), timeText.end());
    string transactionId = "TXN" + to_string(currentTime) + to_string(accountNumber);

    return {transactionId, accountNumber, type, amountPaisa, timeText, relatedAccountNumber};
}

bool BankSystem::saveFinancialChanges(const vector<Transaction>& newTransactions)
{
    return FileManager::saveAccountsAndTransactions(accountsFile, transactionsFile,
                                                     accounts, newTransactions);
}

void BankSystem::createAccount()
{
    int nextAccountNumber = 100001;
    for(const Account& account : accounts)
        nextAccountNumber = max(nextAccountNumber, account.getAccountNumber() + 1);

    string customerName = readRequiredText("Customer name: ");
    string phoneNumber = readRequiredText("Phone number: ");
    string accountType = readAccountType();
    Account account(nextAccountNumber, customerName, phoneNumber, accountType, 0, "Active");
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
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string customerName = readOptionalText("New customer name (press Enter to keep current): ");
    string phoneNumber = readOptionalText("New phone number (press Enter to keep current): ");
    string accountType = readAccountType(true);

    if(!customerName.empty())
        account.setCustomerName(customerName);
    if(!phoneNumber.empty())
        account.setPhoneNumber(phoneNumber);
    if(!accountType.empty())
        account.setAccountType(accountType);

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

void BankSystem::depositMoney()
{
    int accountNumber = readAccountNumber("Enter account number for deposit: ");
    int index = findAccountIndex(accountNumber);

    if(index == -1)
    {
        cout << "Account not found.\n";
        return;
    }

    if(accounts[index].getStatus() != "Active")
    {
        cout << "Transaction denied. Account is not active.\n";
        return;
    }

    long long amountPaisa = readAmount("Enter deposit amount: ");
    accounts[index].deposit(amountPaisa);

    if(saveFinancialChanges({createTransaction(accountNumber, "Deposit", amountPaisa)}))
    {
        cout << "Deposit successful. New balance: " << formatPaisa(accounts[index].getBalancePaisa()) << '\n';
    }
    else
    {
        accounts[index].withdraw(amountPaisa);
        cout << "Unable to save account data.\n";
    }
}

void BankSystem::withdrawMoney()
{
    int accountNumber = readAccountNumber("Enter account number for withdrawal: ");
    int index = findAccountIndex(accountNumber);

    if(index == -1)
    {
        cout << "Account not found.\n";
        return;
    }

    if(accounts[index].getStatus() != "Active")
    {
        cout << "Transaction denied. Account is not active.\n";
        return;
    }

    long long amountPaisa = readAmount("Enter withdrawal amount: ");
    if(!accounts[index].withdraw(amountPaisa))
    {
        cout << "Withdrawal denied. Insufficient balance.\n";
        return;
    }

    if(saveFinancialChanges({createTransaction(accountNumber, "Withdrawal", amountPaisa)}))
    {
        cout << "Withdrawal successful. New balance: " << formatPaisa(accounts[index].getBalancePaisa()) << '\n';
    }
    else
    {
        accounts[index].deposit(amountPaisa);
        cout << "Unable to save account data.\n";
    }
}

void BankSystem::transferMoney()
{
    int senderNumber = readAccountNumber("Enter sender account number: ");
    int receiverNumber = readAccountNumber("Enter receiver account number: ");

    if(senderNumber == receiverNumber)
    {
        cout << "Transfer denied. Sender and receiver accounts must be different.\n";
        return;
    }

    int senderIndex = findAccountIndex(senderNumber);
    int receiverIndex = findAccountIndex(receiverNumber);

    if(senderIndex == -1 || receiverIndex == -1)
    {
        cout << "Transfer denied. One or both accounts were not found.\n";
        return;
    }

    if(accounts[senderIndex].getStatus() != "Active" ||
       accounts[receiverIndex].getStatus() != "Active")
    {
        cout << "Transfer denied. Both accounts must be active.\n";
        return;
    }

    long long amountPaisa = readAmount("Enter transfer amount: ");
    if(!accounts[senderIndex].withdraw(amountPaisa))
    {
        cout << "Transfer denied. Insufficient balance in sender account.\n";
        return;
    }

    accounts[receiverIndex].deposit(amountPaisa);
    if(saveFinancialChanges({createTransaction(senderNumber, "Transfer Sent", amountPaisa, receiverNumber),
                             createTransaction(receiverNumber, "Transfer Received", amountPaisa, senderNumber)}))
    {
        cout << "Transfer successful.\n"
               << "Sender balance: " << formatPaisa(accounts[senderIndex].getBalancePaisa()) << '\n'
               << "Receiver balance: " << formatPaisa(accounts[receiverIndex].getBalancePaisa()) << '\n';
    }
    else
    {
        accounts[senderIndex].deposit(amountPaisa);
        accounts[receiverIndex].withdraw(amountPaisa);
        cout << "Unable to save account data. Transfer cancelled.\n";
    }
}

void BankSystem::transactionHistory() const
{
    int accountNumber = readAccountNumber("Enter account number for transaction history: ");
    vector<Transaction> transactions = FileManager::loadTransactions(transactionsFile);
    bool found = false;

    cout << "\n===== Transaction History =====\n";
    for(const Transaction& transaction : transactions)
    {
        if(transaction.accountNumber == accountNumber)
        {
            cout << "Transaction ID: " << transaction.transactionId << '\n'
                 << "Type: " << transaction.type << '\n'
                 << "Amount: " << formatPaisa(transaction.amountPaisa) << '\n'
                 << "Date: " << transaction.dateTime << '\n';
            if(transaction.relatedAccountNumber != 0)
                cout << "Related Account: " << transaction.relatedAccountNumber << '\n';
            cout << "\n";
            found = true;
        }
    }

    if(!found)
        cout << "No transactions found for this account.\n";
}

void BankSystem::showReports() const
{
    vector<Transaction> transactions = FileManager::loadTransactions(transactionsFile);
    long long totalDeposits = 0;
    long long totalWithdrawals = 0;
    long long totalTransfers = 0;
    long long totalBalance = 0;
    int activeAccounts = 0;
    int closedAccounts = 0;
    map<string, int> accountTypeCounts;

    for(const Account& account : accounts)
    {
        totalBalance += account.getBalancePaisa();
        accountTypeCounts[account.getAccountType()]++;
        account.getStatus() == "Active" ? activeAccounts++ : closedAccounts++;
    }

    for(const Transaction& transaction : transactions)
    {
        if(transaction.type == "Deposit")
            totalDeposits += transaction.amountPaisa;
        else if(transaction.type == "Withdrawal")
            totalWithdrawals += transaction.amountPaisa;
        else if(transaction.type == "Transfer Sent")
            totalTransfers += transaction.amountPaisa;
    }

    cout << "\n===== Banking Reports =====\n"
         << "Total accounts: " << accounts.size() << '\n'
         << "Active accounts: " << activeAccounts << '\n'
         << "Closed accounts: " << closedAccounts << '\n'
         << "Total balance: " << formatPaisa(totalBalance) << '\n'
         << "Total deposits: " << formatPaisa(totalDeposits) << '\n'
         << "Total withdrawals: " << formatPaisa(totalWithdrawals) << '\n'
         << "Total transferred: " << formatPaisa(totalTransfers) << '\n'
         << "\nAccount types:\n";

    for(const auto& entry : accountTypeCounts)
        cout << entry.first << ": " << entry.second << '\n';
}
