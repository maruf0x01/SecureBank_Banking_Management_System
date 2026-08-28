#include "FileManager.h"

long long parseMoneyToPaisa(const string& value)
{
    size_t decimalPosition = value.find('.');
    string whole = decimalPosition == string::npos ? value : value.substr(0, decimalPosition);
    string fraction = decimalPosition == string::npos ? "" : value.substr(decimalPosition + 1);

    if(whole.empty() || fraction.size() > 2 || fraction.find_first_not_of("0123456789") != string::npos ||
       whole.find_first_not_of("0123456789") != string::npos)
        throw invalid_argument("Invalid money value");

    while(fraction.size() < 2)
        fraction += '0';

    long long wholeValue = stoll(whole);
    long long fractionValue = fraction.empty() ? 0 : stoll(fraction);
    if(wholeValue > (LLONG_MAX - fractionValue) / 100)
        throw out_of_range("Money value is too large");

    return wholeValue * 100 + fractionValue;
}

string formatPaisa(long long amountPaisa)
{
    return to_string(amountPaisa / 100) + "." + (amountPaisa % 100 < 10 ? "0" : "") +
           to_string(amountPaisa % 100);
}

vector<Account> FileManager::loadAccounts(const string& fileName)
{
    vector<Account> accounts;
    set<int> accountNumbers;
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
            long long parsedBalance = parseMoneyToPaisa(balance);

                if(parsedAccountNumber <= 0 || accountNumbers.count(parsedAccountNumber) > 0 ||
                    customerName.empty() || phoneNumber.empty() ||
               accountType.empty() || parsedBalance < 0 ||
               (status != "Active" && status != "Closed"))
                continue;

            accounts.emplace_back(parsedAccountNumber, customerName, phoneNumber,
                                  accountType, parsedBalance, status);
            accountNumbers.insert(parsedAccountNumber);
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

    for(const Account& account : accounts)
    {
        file << account.getAccountNumber() << '|'
             << account.getCustomerName() << '|'
             << account.getPhoneNumber() << '|'
             << account.getAccountType() << '|'
             << formatPaisa(account.getBalancePaisa()) << '|'
             << account.getStatus() << '\n';
    }

    file.close();
    if(file.fail())
        return false;

    remove(fileName.c_str());
    return rename(temporaryFile.c_str(), fileName.c_str()) == 0;
}

bool FileManager::saveAccountsAndTransactions(const string& accountsFile,
                                              const string& transactionsFile,
                                              const vector<Account>& accounts,
                                              const vector<Transaction>& newTransactions)
{
    const string accountsTemporaryFile = accountsFile + ".tmp";
    const string transactionsTemporaryFile = transactionsFile + ".tmp";
    vector<Transaction> transactions = loadTransactions(transactionsFile);
    transactions.insert(transactions.end(), newTransactions.begin(), newTransactions.end());

    ofstream accountOutput(accountsTemporaryFile);
    ofstream transactionOutput(transactionsTemporaryFile);
    if(!accountOutput || !transactionOutput)
        return false;

    for(const Account& account : accounts)
        accountOutput << account.getAccountNumber() << '|'
                      << account.getCustomerName() << '|'
                      << account.getPhoneNumber() << '|'
                      << account.getAccountType() << '|'
                      << formatPaisa(account.getBalancePaisa()) << '|'
                      << account.getStatus() << '\n';

    for(const Transaction& transaction : transactions)
        transactionOutput << transaction.transactionId << '|'
                          << transaction.accountNumber << '|'
                          << transaction.type << '|'
                          << formatPaisa(transaction.amountPaisa) << '|'
                          << transaction.dateTime << '|'
                          << transaction.relatedAccountNumber << '\n';

    accountOutput.close();
    transactionOutput.close();
    if(accountOutput.fail() || transactionOutput.fail())
        return false;

    remove(accountsFile.c_str());
    remove(transactionsFile.c_str());
    if(rename(accountsTemporaryFile.c_str(), accountsFile.c_str()) != 0)
        return false;

    if(rename(transactionsTemporaryFile.c_str(), transactionsFile.c_str()) != 0)
    {
        remove(accountsFile.c_str());
        return false;
    }

    return true;
}

bool FileManager::appendTransaction(const string& fileName, const Transaction& transaction)
{
    ofstream file(fileName, ios::app);

    if(!file)
        return false;

    file << transaction.transactionId << '|'
         << transaction.accountNumber << '|'
         << transaction.type << '|'
            << formatPaisa(transaction.amountPaisa) << '|'
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
            long long parsedAmount = parseMoneyToPaisa(amount);
            int parsedRelatedAccountNumber = stoi(relatedAccountNumber);

            if(transactionId.empty() || parsedAccountNumber <= 0 || type.empty() ||
               parsedAmount <= 0 || dateTime.empty() ||
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
