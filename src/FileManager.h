#pragma once

#include <bits/stdc++.h>
#include "Account.h"
using namespace std;

class FileManager
{
public:
    static vector<Account> loadAccounts(const string& fileName);
    static bool saveAccounts(const string& fileName, const vector<Account>& accounts);
};
