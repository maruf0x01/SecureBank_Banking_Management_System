#include <bits/stdc++.h>
using namespace std;

void showMenu()
{
    cout << "\n===== SecureBank Banking Management System =====\n"
         << "1. Create account\n"
         << "2. View all accounts\n"
         << "3. Search account\n"
         << "4. Update account\n"
         << "5. Close account\n"
         << "6. Deposit money\n"
         << "7. Withdraw money\n"
         << "8. Transfer money\n"
         << "9. Transaction history\n"
         << "0. Exit\n"
         << "Enter your choice: ";
}

bool adminLogin()
{
    const string correctUsername = "admin";
    const string correctPassword = "admin123";
    string username, password;

    for(int attempt = 1; attempt <= 3; attempt++)
    {
        cout << "\n===== Admin Login =====\n";
        cout << "Username: ";
        cin >> username;
        cout << "Password: ";
        cin >> password;

        if(username == correctUsername && password == correctPassword)
        {
            cout << "Login successful.\n";
            return true;
        }

        int remainingAttempts = 3 - attempt;
        cout << (remainingAttempts > 0 ? "Invalid credentials. Attempts remaining: " : "Invalid credentials.\n")
             << (remainingAttempts > 0 ? to_string(remainingAttempts) : "");
        cout << (remainingAttempts > 0 ? "\n" : "");
    }

    cout << "\nToo many failed attempts. Access denied.\n";
    return false;
}

int main()
{
    if(!adminLogin())
        return 0;

    int choice = -1;

    while(choice != 0)
    {
        showMenu();

        if(!(cin >> choice))
        {
            cout << "Invalid input. Please enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if(choice == 0)
            cout << "Thank you for using SecureBank.\n";
        else if(choice >= 1 && choice <= 9)
            cout << "This feature will be implemented in a future milestone.\n";
        else
            cout << "Invalid choice. Please select an available option.\n";
    }

    return 0;
}
