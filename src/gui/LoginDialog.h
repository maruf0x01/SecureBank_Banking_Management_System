// ============================================================================
// FILE: LoginDialog.h
// PURPOSE: Header for the Sign-In window (supports Customer and Admin roles,
//          plus a link for new customers to self-register an account).
// ============================================================================
#pragma once

#include <QDialog>

#include "BankService.h"

class QLineEdit;
class QLabel;
class QPushButton;

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    enum Role { None, Admin, Customer };

    explicit LoginDialog(BankService& bank, QWidget* parent = nullptr);

    // Returns which role successfully signed in (Admin or Customer)
    Role role() const { return loggedRole; }
    // Returns the logged-in customer's account number (only used when role() == Customer)
    int accountNumber() const { return loggedAccount; }

private slots:
    void attemptLogin();       // Triggered when clicking "Sign in" or pressing Enter
    void openRegistration();   // Triggered when clicking "New customer? Create a bank account"
    void setRole(Role role);   // Switches between Customer tab and Admin tab

private:
    BankService& bank;
    QPushButton* customerTab;    // "Customer" role switch button
    QPushButton* adminTab;       // "Admin" role switch button
    QPushButton* registerButton; // "New customer? Create a bank account" link button
    QLineEdit* usernameEdit;     // Username input field
    QLineEdit* passwordEdit;     // Password input field
    QLabel* messageLabel;        // Red error message label (wrong password, pending account, etc.)
    QLabel* hintLabel;           // Gray hint text ("Administrator access.")
    Role selected = Customer;
    Role loggedRole = None;
    int loggedAccount = 0;
    int attempts = 0;            // Counts failed login attempts (max 3)
};
