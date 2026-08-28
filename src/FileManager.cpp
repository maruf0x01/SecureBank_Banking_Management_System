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
