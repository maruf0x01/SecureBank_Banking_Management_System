#include "FileManager.h"

vector<Account> FileManager::loadAccounts(const string& fileName)
{
    vector<Account> accounts;
    ifstream file(fileName);
    string line;

    while(getline(file, line))
    {
        if(line.empty())
            continue;

        stringstream stream(line);
        string accountNumber, customerName, phoneNumber, accountType, balance, status;

        if(!getline(stream, accountNumber, '|') || !getline(stream, customerName, '|') ||
           !getline(stream, phoneNumber, '|') || !getline(stream, accountType, '|') ||
           !getline(stream, balance, '|') || !getline(stream, status))
            continue;

        try
        {
            accounts.emplace_back(stoi(accountNumber), customerName, phoneNumber,
                                  accountType, stod(balance), status);
        }
        catch(...)
        {
            continue;
        }
    }

    return accounts;
}

bool FileManager::saveAccounts(const string& fileName, const vector<Account>& accounts)
{
    const string temporaryFile = fileName + ".tmp";
    ofstream file(temporaryFile);

    if(!file)
        return false;

    file << fixed << setprecision(2);
    for(const Account& account : accounts)
    {
        file << account.getAccountNumber() << '|'
             << account.getCustomerName() << '|'
             << account.getPhoneNumber() << '|'
             << account.getAccountType() << '|'
             << account.getBalance() << '|'
             << account.getStatus() << '\n';
    }

    file.close();
    if(file.fail())
        return false;

    remove(fileName.c_str());
    return rename(temporaryFile.c_str(), fileName.c_str()) == 0;
}

bool FileManager::appendTransaction(const string& fileName, const Transaction& transaction)
{
    ofstream file(fileName, ios::app);
    if(!file)
        return false;

    file << fixed << setprecision(2)
         << transaction.transactionId << '|'
         << transaction.accountNumber << '|'
         << transaction.type << '|'
         << transaction.amount << '|'
         << transaction.dateTime << '|'
         << transaction.relatedAccountNumber << '\n';

    return file.good();
}

vector<Transaction> FileManager::loadTransactions(const string& fileName)
{
    vector<Transaction> transactions;
    ifstream file(fileName);
    string line;

    while(getline(file, line))
    {
        if(line.empty())
            continue;

        stringstream stream(line);
        string transactionId, accountNumber, type, amount, dateTime, relatedAccountNumber;

        if(!getline(stream, transactionId, '|') || !getline(stream, accountNumber, '|') ||
           !getline(stream, type, '|') || !getline(stream, amount, '|') ||
           !getline(stream, dateTime, '|') || !getline(stream, relatedAccountNumber))
            continue;

        try
        {
            transactions.push_back({transactionId, stoi(accountNumber), type, stod(amount),
                                    dateTime, stoi(relatedAccountNumber)});
        }
        catch(...)
        {
            continue;
        }
    }

    return transactions;
}
