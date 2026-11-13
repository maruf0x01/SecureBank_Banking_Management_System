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

    Role role() const { return loggedRole; }
    int accountNumber() const { return loggedAccount; }

private slots:
    void attemptLogin();
    void openRegistration();
    void setRole(Role role);

private:
    BankService& bank;
    QPushButton* customerTab;
    QPushButton* adminTab;
    QPushButton* registerButton;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QLabel* messageLabel;
    QLabel* hintLabel;
    Role selected = Customer;
    Role loggedRole = None;
    int loggedAccount = 0;
    int attempts = 0;
};
