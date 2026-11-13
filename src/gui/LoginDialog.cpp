#include "LoginDialog.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStyle>

#include "Dialogs.h"

LoginDialog::LoginDialog(BankService& bank, QWidget* parent)
    : QDialog(parent), bank(bank)
{
    setWindowTitle("SecureBank - Sign in");
    setMinimumWidth(400);

    auto* title = new QLabel("SecureBank");
    title->setObjectName("pageTitle");
    title->setAlignment(Qt::AlignCenter);
    auto* sub = new QLabel("Sign in to continue");
    sub->setObjectName("subtle");
    sub->setAlignment(Qt::AlignCenter);

    customerTab = makeButton("Customer");
    adminTab = makeButton("Admin");
    for(auto* b : {customerTab, adminTab})
        b->setCheckable(true);
    auto* tabs = new QHBoxLayout;
    tabs->addWidget(customerTab);
    tabs->addWidget(adminTab);

    usernameEdit = new QLineEdit;
    usernameEdit->setPlaceholderText("Username");
    passwordEdit = new QLineEdit;
    passwordEdit->setPlaceholderText("Password");
    usernameEdit->setMaxLength(50);
    passwordEdit->setMaxLength(64);
    passwordEdit->setEchoMode(QLineEdit::Password);
    auto* showPassword = new QCheckBox("Show password");
    messageLabel = new QLabel;
    messageLabel->setStyleSheet("color: #b91c1c;");
    messageLabel->setWordWrap(true);
    hintLabel = new QLabel;
    hintLabel->setObjectName("subtle");
    hintLabel->setWordWrap(true);

    auto* loginButton = makeButton("Sign in", true);
    loginButton->setMinimumHeight(36);
    loginButton->setDefault(true);
    auto* cancelButton = makeButton("Exit");
    registerButton = makeButton("New customer? Create a bank account");
    registerButton->setFlat(true);
    registerButton->setStyleSheet("border: none; background: transparent; color: #2563eb; text-decoration: underline;");

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(cancelButton);
    buttons->addWidget(loginButton, 1);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(10);
    layout->addWidget(title);
    layout->addWidget(sub);
    layout->addSpacing(6);
    layout->addLayout(tabs);
    layout->addWidget(usernameEdit);
    layout->addWidget(passwordEdit);
    layout->addWidget(showPassword);
    layout->addWidget(messageLabel);
    layout->addLayout(buttons);
    layout->addWidget(hintLabel);
    layout->addWidget(registerButton);

    connect(customerTab, &QPushButton::clicked, this, [this] { setRole(Customer); });
    connect(adminTab, &QPushButton::clicked, this, [this] { setRole(Admin); });
    connect(showPassword, &QCheckBox::toggled, this, [this](bool on) {
        passwordEdit->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
    });
    connect(registerButton, &QPushButton::clicked, this, &LoginDialog::openRegistration);
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::attemptLogin);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(usernameEdit, &QLineEdit::returnPressed, passwordEdit, qOverload<>(&QWidget::setFocus));
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::attemptLogin);

    setRole(Customer);
}

void LoginDialog::setRole(Role role)
{
    selected = role;
    customerTab->setChecked(role == Customer);
    adminTab->setChecked(role == Admin);
    customerTab->setProperty("primary", role == Customer);
    adminTab->setProperty("primary", role == Admin);
    for(auto* b : {customerTab, adminTab})
    {
        b->style()->unpolish(b);
        b->style()->polish(b);
    }
    hintLabel->setText(role == Customer ? "" : "Administrator access.");
    registerButton->setVisible(role == Customer);
    messageLabel->clear();
    usernameEdit->setFocus();
}

void LoginDialog::openRegistration()
{
    QString name, phone, type, username, password;
    if(!askRegistration(this, name, phone, type, username, password))
        return;
    Result r = bank.registerAccount(name, phone, type, username, password);
    if(r.ok)
    {
        QMessageBox::information(this, "Account created", r.message);
        usernameEdit->setText(username);
        passwordEdit->clear();
        passwordEdit->setFocus();
    }
    else
        QMessageBox::warning(this, "Could not create account", r.message);
}

void LoginDialog::attemptLogin()
{
    const QString user = usernameEdit->text().trimmed();
    const QString pass = passwordEdit->text();
    if(user.isEmpty() || pass.isEmpty())
    {
        messageLabel->setText("Please enter both username and password.");
        return;
    }

    if(selected == Admin)
    {
        // Same credentials as the console version.
        if(user == "admin" && pass == "admin123")
        {
            loggedRole = Admin;
            accept();
            return;
        }
    }
    else
    {
        bank.reload();
        int account = bank.authenticate(user, pass);
        if(account > 0)
        {
            loggedRole = Customer;
            loggedAccount = account;
            accept();
            return;
        }
        if(account == -3)
        {
            messageLabel->setText("Your account is awaiting activation by an administrator.");
            passwordEdit->clear();
            return;
        }
        if(account == -2)
        {
            messageLabel->setText("This account is closed. Please contact the bank.");
            passwordEdit->clear();
            return;
        }
    }

    attempts++;
    passwordEdit->clear();
    if(attempts >= 3)
    {
        reject();
        return;
    }
    messageLabel->setText(QString("Invalid username or password. Attempts remaining: %1").arg(3 - attempts));
}
