// ============================================================================
// FILE: LoginDialog.cpp
// PURPOSE: Builds and handles the Sign-In window UI:
//          - Customer / Admin tab buttons
//          - Username & Password fields + "Show password" checkbox
//          - "Sign in", "Exit", and "New customer? Create a bank account" buttons
//          - Admin credentials check ("admin" / "admin123") and 3-attempt limit
// ============================================================================
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
    // [WINDOW TITLE & SIZE] Change login window title or width here:
    setWindowTitle("SecureBank - Sign in");
    setMinimumWidth(400);

    // [HEADER TEXT] Main heading and subtitle at the top of the login window:
    auto* title = new QLabel("SecureBank");
    title->setObjectName("pageTitle");
    title->setAlignment(Qt::AlignCenter);
    auto* sub = new QLabel("Sign in to continue");
    sub->setObjectName("subtle");
    sub->setAlignment(Qt::AlignCenter);

    // [ROLE TABS] "Customer" and "Admin" toggle buttons:
    customerTab = makeButton("Customer");
    adminTab = makeButton("Admin");
    for(auto* b : {customerTab, adminTab})
        b->setCheckable(true);
    auto* tabs = new QHBoxLayout;
    tabs->addWidget(customerTab);
    tabs->addWidget(adminTab);

    // [INPUT FIELDS] Username and Password boxes + "Show password" checkbox:
    usernameEdit = new QLineEdit;
    usernameEdit->setPlaceholderText("Username"); // Placeholder inside username box
    passwordEdit = new QLineEdit;
    passwordEdit->setPlaceholderText("Password"); // Placeholder inside password box
    usernameEdit->setMaxLength(50);
    passwordEdit->setMaxLength(64);
    passwordEdit->setEchoMode(QLineEdit::Password);
    auto* showPassword = new QCheckBox("Show password"); // Checkbox text

    // [LABELS] Red error message label and gray hint label:
    messageLabel = new QLabel;
    messageLabel->setStyleSheet("color: #b91c1c;");
    messageLabel->setWordWrap(true);
    hintLabel = new QLabel;
    hintLabel->setObjectName("subtle");
    hintLabel->setWordWrap(true);

    // [ACTION BUTTONS] "Sign in", "Exit", and "New customer? Create a bank account":
    auto* loginButton = makeButton("Sign in", true); // true = Blue primary button
    loginButton->setMinimumHeight(36);
    loginButton->setDefault(true);
    auto* cancelButton = makeButton("Exit");         // Exit button
    registerButton = makeButton("New customer? Create a bank account"); // Self-registration link
    registerButton->setFlat(true);
    registerButton->setStyleSheet("border: none; background: transparent; color: #2563eb; text-decoration: underline;");

    // Horizontal row holding [Exit] and [Sign in] buttons:
    auto* buttons = new QHBoxLayout;
    buttons->addWidget(cancelButton);
    buttons->addWidget(loginButton, 1);

    // [LAYOUT] Arrange all widgets vertically from top to bottom:
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

    // [BUTTON CLICK CONNECTIONS] Connect each button/field to its action:
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

    // Default selected tab when window opens:
    setRole(Customer);
}

// Switches visual styling and helper text when clicking "Customer" or "Admin" tab.
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
    // [HINT TEXT] Shown when Admin tab is selected:
    hintLabel->setText(role == Customer ? "" : "Administrator access.");
    // Hide the registration link on the Admin tab:
    registerButton->setVisible(role == Customer);
    messageLabel->clear();
    usernameEdit->setFocus();
}

// Opens the "Create a bank account" pop-up dialog when a new customer clicks the registration link.
void LoginDialog::openRegistration()
{
    QString name, phone, type, username, password;
    if(!askRegistration(this, name, phone, type, username, password))
        return;
    Result r = bank.registerAccount(name, phone, type, username, password);
    if(r.ok)
    {
        // [POPUP] Shown after a customer submits a new account registration:
        QMessageBox::information(this, "Account created", r.message);
        usernameEdit->setText(username);
        passwordEdit->clear();
        passwordEdit->setFocus();
    }
    else
        QMessageBox::warning(this, "Could not create account", r.message);
}

// Validates credentials when the user clicks "Sign in".
void LoginDialog::attemptLogin()
{
    const QString user = usernameEdit->text().trimmed();
    const QString pass = passwordEdit->text();
    if(user.isEmpty() || pass.isEmpty())
    {
        // [ERROR TEXT] Empty username or password:
        messageLabel->setText("Please enter both username and password.");
        return;
    }

    if(selected == Admin)
    {
        // [ADMIN CREDENTIALS] Change admin username ("admin") or password ("admin123") here:
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
            // [ERROR TEXT] Account is still Pending (waiting for admin activation):
            messageLabel->setText("Your account is awaiting activation by an administrator.");
            passwordEdit->clear();
            return;
        }
        if(account == -2)
        {
            // [ERROR TEXT] Account is Closed:
            messageLabel->setText("This account is closed. Please contact the bank.");
            passwordEdit->clear();
            return;
        }
    }

    // [LOGIN ATTEMPTS LIMIT] Default is 3 attempts. Change 3 to 5 for 5 attempts:
    attempts++;
    passwordEdit->clear();
    if(attempts >= 3)
    {
        reject();
        return;
    }
    messageLabel->setText(QString("Invalid username or password. Attempts remaining: %1").arg(3 - attempts));
}
