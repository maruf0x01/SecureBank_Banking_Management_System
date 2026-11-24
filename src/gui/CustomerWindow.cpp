// ============================================================================
// FILE: CustomerWindow.cpp
// PURPOSE: Implements the Customer online banking dashboard:
//          - Left Sidebar ("Overview", "Transactions", "Deposit requests", "Loans", "Profile & security")
//          - Overview page (Welcome title, blue Balance Hero Card, Send money / Deposit / Withdraw / Apply for loan)
//          - Transactions page (Customer's transaction history + type filter)
//          - Deposit requests page ("Request a deposit" button + request status table)
//          - Loans page ("Apply for a loan" & "Repay selected loan" buttons + loans status table)
//          - Profile & security page (Account info card + Change phone / Change password / Security questions)
// ============================================================================
#include "CustomerWindow.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableWidget>
#include <QVBoxLayout>

#include "Dialogs.h"
#include "Theme.h"

CustomerWindow::CustomerWindow(BankService& bank, int accountNumber)
    : bank(bank), accountNumber(accountNumber)
{
    // [WINDOW TITLE & SIZE] Change customer window title or initial size (width, height) here:
    setWindowTitle("SecureBank - My Account");
    resize(980, 640);

    auto* central = new QWidget;
    auto* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    pages = new QStackedWidget;
    layout->addWidget(buildSidebar());
    pages->addWidget(buildOverview()); // Page index 0
    pages->addWidget(buildHistory());  // Page index 1
    pages->addWidget(buildRequests()); // Page index 2
    pages->addWidget(buildLoans());    // Page index 3
    pages->addWidget(buildProfile());  // Page index 4
    layout->addWidget(pages, 1);
    setCentralWidget(central);

    connect(sidebar, &QListWidget::currentRowChanged, this, [this](int row) {
        pages->setCurrentIndex(row);
        refresh();
    });
    sidebar->setCurrentRow(0);
    refresh();
}

// ============================================================================
// LEFT SIDEBAR: Brand logo, navigation menu items, and "Sign out" button
// ============================================================================
QWidget* CustomerWindow::buildSidebar()
{
    auto* panel = new QWidget;
    panel->setFixedWidth(200); // [SIDEBAR WIDTH] Change 200 to make sidebar wider/narrower
    panel->setObjectName("sidePanel");
    panel->setAttribute(Qt::WA_StyledBackground, true);
    auto* v = new QVBoxLayout(panel);
    v->setContentsMargins(0, 0, 0, 12);
    v->setSpacing(0);

    // [SIDEBAR BRAND TEXT] Top-left logo title and subtitle:
    auto* brand = new QLabel("SecureBank");
    brand->setObjectName("brand");
    auto* sub = new QLabel("Online banking");
    sub->setObjectName("brandSub");

    // [SIDEBAR MENU ITEMS] Change customer sidebar menu names here:
    sidebar = new QListWidget;
    sidebar->setObjectName("sidebar");
    sidebar->addItems({"Overview", "Transactions", "Deposit requests", "Loans", "Profile & security"});
    sidebar->setFocusPolicy(Qt::NoFocus);
    for(int i = 0; i < sidebar->count(); i++)
        sidebar->item(i)->setSizeHint(QSize(0, 44));

    // [SIGN OUT BUTTON] Bottom red Sign out button:
    auto* logout = new QPushButton("Sign out");
    logout->setProperty("logout", true);
    connect(logout, &QPushButton::clicked, this, [this] {
        emit loggedOut();
        close();
    });
    v->addWidget(brand);
    v->addWidget(sub);
    v->addWidget(sidebar, 1);
    v->addWidget(makeSideButtons({logout}));
    return panel;
}

// ============================================================================
// PAGE 0: OVERVIEW (Welcome heading, Blue Balance Hero Card, Action buttons, Recent activity)
// ============================================================================
QWidget* CustomerWindow::buildOverview()
{
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    v->setContentsMargins(24, 20, 24, 20);

    // [WELCOME HEADING] Filled dynamically with "Welcome, <Name>" in refresh():
    welcomeLabel = new QLabel;
    welcomeLabel->setObjectName("pageTitle");
    v->addWidget(welcomeLabel);

    // [HERO BALANCE CARD] Blue gradient card showing "Available balance" and account number:
    auto* hero = new QFrame;
    hero->setObjectName("heroCard");
    auto* hv = new QVBoxLayout(hero);
    hv->setContentsMargins(24, 20, 24, 20);
    auto* caption = new QLabel("Available balance"); // Text above the balance number
    caption->setObjectName("onBlue");
    balanceLabel = new QLabel;
    balanceLabel->setObjectName("balanceValue");
    accountLabel = new QLabel;
    accountLabel->setObjectName("onBlue");
    hv->addWidget(caption);
    hv->addWidget(balanceLabel);
    hv->addWidget(accountLabel);
    v->addWidget(hero);

    // [QUICK ACTION BUTTONS] "Send money", "Deposit", "Withdraw", and "Apply for loan":
    statusLabel = new QLabel;
    statusLabel->setObjectName("subtle");
    auto* deposit = makeButton("Deposit", false);     // Change false -> true to make it blue
    deposit->setMinimumHeight(38);
    connect(deposit, &QPushButton::clicked, this, [this] { doRequestDeposit(); });
    auto* withdraw = makeButton("Withdraw", false);
    auto* transfer = makeButton("Send money", true);  // true = Blue primary button
    auto* loanBtn = makeButton("Apply for loan", false);
    withdraw->setMinimumHeight(38);
    transfer->setMinimumHeight(38);
    loanBtn->setMinimumHeight(38);
    connect(withdraw, &QPushButton::clicked, this, [this] { doWithdraw(); });
    connect(transfer, &QPushButton::clicked, this, [this] { doTransfer(); });
    connect(loanBtn, &QPushButton::clicked, this, [this] { doRequestLoan(); });

    // [BUTTON ORDER] Change the order of row->addWidget(...) to move buttons left/right:
    auto* row = new QHBoxLayout;
    row->addWidget(transfer);
    row->addWidget(deposit);
    row->addWidget(withdraw);
    row->addWidget(loanBtn);
    row->addStretch();
    row->addWidget(statusLabel);
    v->addLayout(row);

    // [RECENT ACTIVITY TABLE] Shows the customer's 8 latest transactions:
    v->addSpacing(6);
    v->addWidget(new QLabel("<b>Recent activity</b>"));
    recentTable = makeTable({});
    v->addWidget(recentTable, 1);
    return page;
}

// ============================================================================
// PAGE 1: TRANSACTIONS (Type filter dropdown + complete transaction history table)
// ============================================================================
QWidget* CustomerWindow::buildHistory()
{
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    v->setContentsMargins(24, 20, 24, 20);
    makeTitle("Transactions", "Your complete transaction history.", v);
    historyType = new QComboBox;
    historyType->addItems({"All types", "Deposit", "Withdrawal", "Transfer Sent", "Transfer Received", "Loan Disbursed", "Loan Repayment"});
    auto* row = new QHBoxLayout;
    row->addWidget(historyType);
    row->addStretch();
    v->addLayout(row);
    historyTable = makeTable({});
    v->addWidget(historyTable, 1);
    connect(historyType, &QComboBox::currentIndexChanged, this, [this] { refresh(); });
    return page;
}

// ============================================================================
// PAGE 2: DEPOSIT REQUESTS ("Request a deposit" button + requests status table)
// ============================================================================
QWidget* CustomerWindow::buildRequests()
{
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    v->setContentsMargins(24, 20, 24, 20);
    makeTitle("Deposit requests", "Deposits are credited to your balance once an administrator approves them.", v);
    auto* btn = makeButton("Request a deposit", true);
    connect(btn, &QPushButton::clicked, this, [this] { doRequestDeposit(); });
    auto* row = new QHBoxLayout;
    row->addWidget(btn);
    row->addStretch();
    v->addLayout(row);
    requestTable = makeTable({"Request ID", "Requested", "Amount", "Status", "Decided"});
    v->addWidget(requestTable, 1);
    return page;
}

// ============================================================================
// PAGE 3: LOANS ("Apply for a loan" & "Repay selected loan" buttons + loans table)
// ============================================================================
QWidget* CustomerWindow::buildLoans()
{
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    v->setContentsMargins(24, 20, 24, 20);
    makeTitle("Loans", "Apply for a loan or repay an active loan from your account balance.", v);

    auto* applyBtn = makeButton("Apply for a loan", true);
    repayLoanBtn = makeButton("Repay selected loan", false);
    repayLoanBtn->setEnabled(false);
    loanSummaryLabel = new QLabel;
    loanSummaryLabel->setObjectName("subtle");

    connect(applyBtn, &QPushButton::clicked, this, [this] { doRequestLoan(); });
    connect(repayLoanBtn, &QPushButton::clicked, this, [this] { doRepayLoan(); });

    auto* row = new QHBoxLayout;
    row->addWidget(applyBtn);
    row->addWidget(repayLoanBtn);
    row->addStretch();
    row->addWidget(loanSummaryLabel);
    v->addLayout(row);

    loanTable = makeTable({"Loan ID", "Requested", "Purpose", "Loan Amount", "Remaining Due", "Status", "Decided"});
    v->addWidget(loanTable, 1);

    connect(loanTable, &QTableWidget::itemSelectionChanged, this, [this] {
        bool activeLoan = selectedLoanId() > 0 && loanTable->item(loanTable->currentRow(), 5)->text() == "Active";
        repayLoanBtn->setEnabled(activeLoan);
    });
    return page;
}

// ============================================================================
// PAGE 4: PROFILE & SECURITY (Account details card + Phone / Password / Security questions)
// ============================================================================
QWidget* CustomerWindow::buildProfile()
{
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    v->setContentsMargins(24, 20, 24, 20);
    makeTitle("Profile & security", "Your account details and sign-in settings.", v);

    // [PROFILE CARD] Displays Name, Account number, Account type, Phone, Status, and Security questions:
    auto* card = makeCard();
    auto* form = new QFormLayout(card);
    form->setContentsMargins(20, 16, 20, 16);
    form->setVerticalSpacing(10);
    profileName = new QLabel;
    profileAccount = new QLabel;
    profileType = new QLabel;
    profilePhone = new QLabel;
    profileStatus = new QLabel;
    profileSecurity = new QLabel;
    form->addRow("Name", profileName);
    form->addRow("Account number", profileAccount);
    form->addRow("Account type", profileType);
    form->addRow("Phone", profilePhone);
    form->addRow("Status", profileStatus);
    form->addRow("Security questions", profileSecurity);
    v->addWidget(card);

    // [PROFILE BUTTONS] "Change phone number", "Change password", and "Update security questions":
    auto* phone = makeButton("Change phone number");
    auto* pass = makeButton("Change password");
    auto* secBtn = makeButton("Update security questions");
    connect(phone, &QPushButton::clicked, this, [this] { doChangePhone(); });
    connect(pass, &QPushButton::clicked, this, [this] { doChangePassword(); });
    connect(secBtn, &QPushButton::clicked, this, [this] { doUpdateSecurityQuestions(); });
    auto* row = new QHBoxLayout;
    row->addWidget(phone);
    row->addWidget(pass);
    row->addWidget(secBtn);
    row->addStretch();
    v->addLayout(row);
    v->addStretch();
    return page;
}

// ---------- Data Refresh & Customer Action Handlers ----------

// Reloads data from disk and updates all labels and tables across the customer pages.
void CustomerWindow::refresh()
{
    bank.reload(); // pick up changes made by an administrator
    const Account* a = bank.find(accountNumber);
    if(!a)
        return;
    QString name = QString::fromStdString(a->getCustomerName());
    long long debt = bank.activeLoanDebt(accountNumber);

    // [OVERVIEW TEXTS] Welcome heading, balance number, and account subtitle (with active loan info):
    welcomeLabel->setText("Welcome, " + name);
    balanceLabel->setText(money(a->getBalancePaisa()));
    QString subText = QString("%1 account  -  No. %2").arg(QString::fromStdString(a->getAccountType())).arg(accountNumber);
    if(debt > 0)
        subText += QString("   |   Active loan due: %1").arg(money(debt));
    accountLabel->setText(subText);
    bool active = a->getStatus() == "Active";
    statusLabel->setText(active ? "" : "This account is closed. Transactions are disabled.");

    profileName->setText(name);
    profileAccount->setText(QString::number(accountNumber));
    profileType->setText(QString::fromStdString(a->getAccountType()));
    profilePhone->setText(QString::fromStdString(a->getPhoneNumber()));
    profileStatus->setText(QString::fromStdString(a->getStatus()));
    profileSecurity->setText(bank.hasSecurityAnswers(accountNumber) ? "Configured (3 questions set)" : "Not set");

    auto all = bank.history(accountNumber);
    auto recent = all;
    if(recent.size() > 8)
        recent.resize(8);
    fillHistory(recentTable, bank, recent, false);

    if(historyType->currentIndex() > 0)
    {
        std::string type = historyType->currentText().toStdString();
        all.erase(std::remove_if(all.begin(), all.end(), [&](const Transaction& t) { return t.type != type; }),
                  all.end());
    }
    fillHistory(historyTable, bank, all, false);

    auto make = [](const QString& t, bool right = false) {
        auto* i = new QTableWidgetItem(t);
        if(right)
            i->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        return i;
    };

    requestTable->setRowCount(0);
    for(const DepositRequest& d : bank.depositRequests(accountNumber))
    {
        int row = requestTable->rowCount();
        requestTable->insertRow(row);
        requestTable->setItem(row, 0, make(QString::number(d.id)));
        requestTable->setItem(row, 1, make(d.requestedAt));
        requestTable->setItem(row, 2, make(money(d.amountPaisa), true));
        auto* st = make(d.status);
        st->setForeground(d.status == "Approved" ? QColor("#15803d")
                          : d.status == "Rejected" ? QColor("#b91c1c") : QColor("#b45309"));
        requestTable->setItem(row, 3, st);
        requestTable->setItem(row, 4, make(d.decidedAt.isEmpty() ? "-" : d.decidedAt));
    }

    int keepLoan = selectedLoanId();
    loanTable->setRowCount(0);
    for(const LoanRequest& l : bank.loanRequests(accountNumber))
    {
        int row = loanTable->rowCount();
        loanTable->insertRow(row);
        loanTable->setItem(row, 0, make(QString::number(l.id)));
        loanTable->setItem(row, 1, make(l.requestedAt));
        loanTable->setItem(row, 2, make(l.purpose));
        loanTable->setItem(row, 3, make(money(l.amountPaisa), true));
        loanTable->setItem(row, 4, make(money(l.remainingPaisa), true));
        auto* st = make(l.status);
        st->setForeground((l.status == "Active" || l.status == "Paid") ? QColor("#15803d")
                          : l.status == "Rejected" ? QColor("#b91c1c") : QColor("#b45309"));
        loanTable->setItem(row, 5, st);
        loanTable->setItem(row, 6, make(l.decidedAt.isEmpty() ? "-" : l.decidedAt));
        if(l.id == keepLoan)
            loanTable->selectRow(row);
    }
    loanSummaryLabel->setText(debt > 0 ? QString("Total active loan due: %1").arg(money(debt))
                                       : "No active loan due");
    bool activeLoan = selectedLoanId() > 0 && loanTable->item(loanTable->currentRow(), 5)->text() == "Active";
    repayLoanBtn->setEnabled(activeLoan);
}

void CustomerWindow::report(const Result& r)
{
    if(r.ok)
        statusBar()->showMessage(r.message, 7000);
    else
        QMessageBox::warning(this, "SecureBank", r.message);
    refresh();
}

int CustomerWindow::selectedLoanId() const
{
    int row = loanTable->currentRow();
    if(row < 0 || loanTable->selectedItems().isEmpty() || !loanTable->item(row, 0))
        return -1;
    return loanTable->item(row, 0)->text().toInt();
}

// Triggered by "Deposit" / "Request a deposit" button:
void CustomerWindow::doRequestDeposit()
{
    long long amount;
    if(askAmount(this, "Request deposit", "Enter the amount to deposit. It will be added to your balance after an administrator approves it.", amount))
        report(bank.requestDeposit(accountNumber, amount));
}

// Triggered by "Apply for loan" / "Apply for a loan" button:
void CustomerWindow::doRequestLoan()
{
    long long amount;
    QString purpose;
    if(askLoanRequest(this, amount, purpose))
    {
        Result r = bank.requestLoan(accountNumber, amount, purpose);
        report(r);
        if(r.ok)
            QMessageBox::information(this, "Loan application submitted", r.message);
    }
}

// Triggered by "Repay selected loan" button on the Loans page:
void CustomerWindow::doRepayLoan()
{
    int id = selectedLoanId();
    if(id < 0)
        return;
    const Account* a = bank.find(accountNumber);
    if(!a)
        return;
    long long remaining = 0;
    for(const LoanRequest& l : bank.loanRequests(accountNumber))
        if(l.id == id)
        {
            remaining = l.remainingPaisa;
            break;
        }
    if(remaining <= 0)
        return;
    if(a->getBalancePaisa() <= 0)
    {
        QMessageBox::warning(this, "Repay loan", "Your account balance is 0.00. Deposit money before repaying the loan.");
        return;
    }
    long long amount;
    if(askLoanRepayment(this, id, remaining, a->getBalancePaisa(), amount))
    {
        Result r = bank.repayLoan(accountNumber, id, amount);
        report(r);
        if(r.ok)
            QMessageBox::information(this, "Loan repayment", r.message);
    }
}

// Triggered by "Withdraw" button (verifies customer password and shows confirmation pop-up):
void CustomerWindow::doWithdraw()
{
    const Account* a = bank.find(accountNumber);
    long long amount;
    if(!a)
        return;
    QString password;
    if(!askAmount(this, "Withdraw",
                  QString("Available balance: <b>%1</b>").arg(money(a->getBalancePaisa())), amount, &password))
        return;
    if(!bank.verifyPassword(accountNumber, password))
    {
        failedPasswords++;
        if(failedPasswords >= 3)
        {
            QMessageBox::critical(this, "SecureBank", "Too many incorrect passwords. You have been signed out.");
            emit loggedOut();
            close();
            return;
        }
        QMessageBox::warning(this, "SecureBank",
                             QString("Incorrect password. Withdrawal cancelled. Attempts remaining: %1")
                                 .arg(3 - failedPasswords));
        return;
    }
    failedPasswords = 0;
    Result r = bank.withdraw(accountNumber, amount);
    report(r);
    // [WITHDRAWAL POPUP] Shown after a customer successfully withdraws money:
    if(r.ok)
        QMessageBox::information(this, "Withdrawal successful",
                                 QString("You withdrew <b>%1</b>.<br>%2").arg(money(amount)).arg(r.message.section(". ", 1)));
}

// Triggered by "Send money" button:
void CustomerWindow::doTransfer()
{
    int from, to;
    long long amount;
    QString receiverName;
    if(askTransfer(this, bank, accountNumber, accountNumber, from, to, amount, receiverName))
        report(bank.transfer(from, to, amount, receiverName));
}

// Triggered by "Change phone number" button:
void CustomerWindow::doChangePhone()
{
    const Account* a = bank.find(accountNumber);
    if(!a)
        return;
    bool ok;
    QString phone = QString::fromStdString(a->getPhoneNumber());
    while(true)
    {
        phone = QInputDialog::getText(this, "Change phone number", "New phone number (7-15 digits):",
                                      QLineEdit::Normal, phone, &ok).trimmed();
        if(!ok)
            return;
        QString err = BankService::validatePhone(phone);
        if(err.isEmpty())
            break;
        QMessageBox::warning(this, "Invalid phone number", err);
    }
    report(bank.updateAccount(accountNumber, QString::fromStdString(a->getCustomerName()), phone,
                              QString::fromStdString(a->getAccountType())));
}

// Triggered by "Change password" button:
void CustomerWindow::doChangePassword()
{
    QString oldPass, newPass;
    if(askChangePassword(this, oldPass, newPass))
        report(bank.changePassword(accountNumber, oldPass, newPass));
}

// Triggered by "Update security questions" button:
void CustomerWindow::doUpdateSecurityQuestions()
{
    QString ans1, ans2, ans3;
    if(askSecurityQuestions(this, ans1, ans2, ans3))
        report(bank.setSecurityAnswers(accountNumber, ans1, ans2, ans3));
}
