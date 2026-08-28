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
            int parsedAccountNumber = stoi(accountNumber);
            double parsedBalance = stod(balance);

            if(parsedAccountNumber <= 0 || customerName.empty() || phoneNumber.empty() ||
               accountType.empty() || !isfinite(parsedBalance) || parsedBalance < 0 ||
               (status != "Active" && status != "Closed"))
                continue;

            accounts.emplace_back(parsedAccountNumber, customerName, phoneNumber,
                                  accountType, parsedBalance, status);
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
            int parsedAccountNumber = stoi(accountNumber);
            double parsedAmount = stod(amount);
            int parsedRelatedAccountNumber = stoi(relatedAccountNumber);

            if(transactionId.empty() || parsedAccountNumber <= 0 || type.empty() ||
               !isfinite(parsedAmount) || parsedAmount <= 0 || dateTime.empty() ||
               parsedRelatedAccountNumber < 0)
                continue;

            transactions.push_back({transactionId, parsedAccountNumber, type, parsedAmount,
                                    dateTime, parsedRelatedAccountNumber});
        }
        catch(...)
        {
            continue;
        }
    }

    return transactions;
}
