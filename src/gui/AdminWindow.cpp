// ============================================================================
// FILE: AdminWindow.cpp
// PURPOSE: Implements the Administrator dashboard and management screens:
//          - Left Sidebar ("Dashboard", "Accounts", "Transactions", "Deposit requests", "Loans")
//          - Dashboard page (8 summary stat cards + account types + recent transactions table)
//          - Accounts page (search, filter, table, and 10 action buttons)
//          - Transactions page (full history with account & type filters)
//          - Deposit requests page ("Approve" and "Reject" customer deposits)
//          - Loans page ("Approve loan" and "Reject loan" customer loan applications)
// ============================================================================
#include "AdminWindow.h"

#include <QAction>
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include <map>

#include "Dialogs.h"
#include "Theme.h"

namespace
{
// Helper: Creates a table cell item (right = true aligns money/numbers to the right).
QTableWidgetItem* cell(const QString& text, bool right = false)
{
    auto* item = new QTableWidgetItem(text);
    if(right)
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return item;
}
}

AdminWindow::AdminWindow(BankService& bank)
    : bank(bank)
{
    // [WINDOW TITLE & SIZE] Change admin window title or initial size (width, height) here:
    setWindowTitle("SecureBank - Administrator");
    resize(1100, 680);

    auto* central = new QWidget;
    auto* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Add left sidebar and the 5 main pages on the right:
    pages = new QStackedWidget;
    layout->addWidget(buildSidebar());
    pages->addWidget(buildDashboard());     // Page index 0
    pages->addWidget(buildAccounts());      // Page index 1
    pages->addWidget(buildTransactions());  // Page index 2
    pages->addWidget(buildRequests());      // Page index 3
    pages->addWidget(buildLoans());         // Page index 4
    layout->addWidget(pages, 1);
    setCentralWidget(central);

    // When clicking a sidebar menu item, switch to that page and reload fresh data from disk:
    connect(sidebar, &QListWidget::currentRowChanged, this, [this](int row) {
        pages->setCurrentIndex(row);
        this->bank.reload(); // pick up changes made elsewhere (e.g. new customer requests)
        refreshAll();
    });
    sidebar->setCurrentRow(0);
    refreshAll();

    // Auto-jump to pending Loans, Deposit requests, or Pending Accounts on login if any are waiting:
    int pending = 0;
    for(const Account& a : bank.accounts())
        if(a.getStatus() == "Pending")
            pending++;
    int pendingDeposits = bank.pendingDepositCount();
    int pendingLoans = bank.pendingLoanCount();
    if(pendingLoans)
    {
        sidebar->setCurrentRow(4);
        statusBar()->showMessage(QString("%1 loan application(s) awaiting approval.").arg(pendingLoans), 10000);
    }
    else if(pendingDeposits)
    {
        sidebar->setCurrentRow(3);
        statusBar()->showMessage(QString("%1 deposit request(s) awaiting approval.").arg(pendingDeposits), 10000);
    }
    else if(pending)
    {
        statusFilter->setCurrentText("Pending");
        sidebar->setCurrentRow(1);
        statusBar()->showMessage(QString("%1 account(s) awaiting activation.").arg(pending), 10000);
    }
}

// ============================================================================
// LEFT SIDEBAR: Brand logo, navigation menu items, "Reload data" & "Sign out"
// ============================================================================
QWidget* AdminWindow::buildSidebar()
{
    auto* panel = new QWidget;
    panel->setFixedWidth(210); // [SIDEBAR WIDTH] Change 210 to make sidebar wider/narrower
    panel->setObjectName("sidePanel");
    panel->setAttribute(Qt::WA_StyledBackground, true);
    auto* v = new QVBoxLayout(panel);
    v->setContentsMargins(0, 0, 0, 12);
    v->setSpacing(0);

    // [SIDEBAR BRAND TEXT] Top-left logo title and subtitle:
    auto* brand = new QLabel("SecureBank");
    brand->setObjectName("brand");
    auto* sub = new QLabel("Administrator");
    sub->setObjectName("brandSub");

    // [SIDEBAR MENU ITEMS] Change sidebar page names here:
    sidebar = new QListWidget;
    sidebar->setObjectName("sidebar");
    sidebar->addItems({"Dashboard", "Accounts", "Transactions", "Deposit requests", "Loans"});
    sidebar->setFocusPolicy(Qt::NoFocus);
    for(int i = 0; i < sidebar->count(); i++)
        sidebar->item(i)->setSizeHint(QSize(0, 44));

    // [SIDEBAR BOTTOM BUTTONS] "Reload data" and "Sign out":
    auto* refresh = new QPushButton("Reload data");
    auto* logout = new QPushButton("Sign out");
    logout->setProperty("logout", true); // Makes "Sign out" red via Theme.h
    connect(refresh, &QPushButton::clicked, this, [this] { doReload(); });
    connect(logout, &QPushButton::clicked, this, [this] {
        emit loggedOut();
        close();
    });

    v->addWidget(brand);
    v->addWidget(sub);
    v->addWidget(sidebar, 1);
    v->addWidget(makeSideButtons({refresh, logout}));
    return panel;
}

// ============================================================================
// PAGE 0: DASHBOARD (8 statistic cards + Recent transactions table)
// ============================================================================
QWidget* AdminWindow::buildDashboard()
{
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    v->setContentsMargins(24, 20, 24, 20);

    // [PAGE TITLE] Dashboard heading and subtitle:
    makeTitle("Dashboard", "Overview of the bank", v);

    // [DASHBOARD STAT CARDS] 2 rows x 4 columns grid (row, col):
    auto* grid = new QGridLayout;
    grid->addWidget(makeStatCard("Total accounts", totalAccountsValue), 0, 0);
    grid->addWidget(makeStatCard("Active accounts", activeValue), 0, 1);
    grid->addWidget(makeStatCard("Total balance", balanceValue), 0, 2);
    grid->addWidget(makeStatCard("Pending activation", loginsValue), 0, 3);
    grid->addWidget(makeStatCard("Total deposits", depositsValue), 1, 0);
    grid->addWidget(makeStatCard("Total withdrawals", withdrawalsValue), 1, 1);
    grid->addWidget(makeStatCard("Total transferred", transfersValue), 1, 2);
    grid->addWidget(makeStatCard("Active loans due", loansValue), 1, 3);
    v->addLayout(grid);

    // [RECENT TRANSACTIONS SECTION] Heading, account type summary, and table on the dashboard:
    auto* headerRow = new QHBoxLayout;
    auto* recentTitle = new QLabel("<b>Recent transactions</b>");
    typeBreakdown = new QLabel;
    typeBreakdown->setObjectName("subtle");
    headerRow->addWidget(recentTitle);
    headerRow->addStretch();
    headerRow->addWidget(typeBreakdown);
    v->addSpacing(8);
    v->addLayout(headerRow);
    recentTable = makeTable({"Account", "Date", "Type", "Amount", "Counterparty", "Transaction ID"});
    v->addWidget(recentTable, 1);
    return page;
}

// ============================================================================
// PAGE 1: ACCOUNTS (Search box, status filter, accounts table, action buttons)
// ============================================================================
QWidget* AdminWindow::buildAccounts()
{
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    v->setContentsMargins(24, 20, 24, 20);

    // [PAGE TITLE] Accounts heading and subtitle:
    makeTitle("Accounts", "Create, edit and manage customer accounts. Right-click a row for more actions.", v);

    // [SEARCH & FILTER BAR] Search input box and Status dropdown:
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("Search by account number, name or phone...");
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setMaxLength(50);
    statusFilter = new QComboBox;
    statusFilter->addItems({"All statuses", "Pending", "Active", "Closed"});
    auto* filterRow = new QHBoxLayout;
    filterRow->addWidget(searchEdit, 1);
    filterRow->addWidget(statusFilter);
    v->addLayout(filterRow);

    // [ACCOUNTS TABLE] Change column header names here:
    accountTable = makeTable({"Account No.", "Customer", "Phone", "Type", "Balance", "Active Loan", "Status", "Login"});
    accountTable->setContextMenuPolicy(Qt::CustomContextMenu);
    v->addWidget(accountTable, 1);

    // [ACCOUNT ACTION BUTTONS] Change button labels here (used for both buttons and right-click menu):
    newAct = new QAction("New account", this);
    editAct = new QAction("Edit details", this);
    depositAct = new QAction("Deposit", this);
    withdrawAct = new QAction("Withdraw", this);
    transferAct = new QAction("Transfer", this);
    toggleAct = new QAction("Close account", this); // Dynamically changes to "Activate account" / "Reopen account"
    loginAct = new QAction("Set customer login", this);
    removeLoginAct = new QAction("Remove customer login", this);
    historyAct = new QAction("View history", this);
    deleteAct = new QAction("Delete account", this);
    newAct->setShortcut(QKeySequence::New);

    // Connect each action to its handler function:
    connect(newAct, &QAction::triggered, this, [this] { doNew(); });
    connect(editAct, &QAction::triggered, this, [this] { doEdit(); });
    connect(depositAct, &QAction::triggered, this, [this] { doDeposit(); });
    connect(withdrawAct, &QAction::triggered, this, [this] { doWithdraw(); });
    connect(transferAct, &QAction::triggered, this, [this] { doTransfer(); });
    connect(toggleAct, &QAction::triggered, this, [this] { doToggle(); });
    connect(loginAct, &QAction::triggered, this, [this] { doLogin(); });
    connect(removeLoginAct, &QAction::triggered, this, [this] { doRemoveLogin(); });
    connect(historyAct, &QAction::triggered, this, [this] { doHistory(); });
    connect(deleteAct, &QAction::triggered, this, [this] { doDelete(); });

    // Helper lambda to create a QPushButton linked to a QAction:
    auto addButton = [&](QHBoxLayout* row, QAction* act, bool primary = false, bool danger = false) {
        QPushButton* b = makeButton(act->text(), primary, danger);
        connect(b, &QPushButton::clicked, act, &QAction::trigger);
        connect(act, &QAction::changed, b, [b, act] {
            b->setEnabled(act->isEnabled());
            b->setText(act->text());
        });
        row->addWidget(b);
    };

    // [BUTTON ROWS LAYOUT] Move addButton(...) between row1 and row2 to rearrange buttons:
    auto* row1 = new QHBoxLayout;
    addButton(row1, newAct, true); // true = Blue primary button
    addButton(row1, editAct);
    addButton(row1, depositAct);
    addButton(row1, withdrawAct);
    addButton(row1, transferAct);
    row1->addStretch();

    auto* row2 = new QHBoxLayout;
    addButton(row2, historyAct);
    addButton(row2, loginAct);
    addButton(row2, removeLoginAct);
    row2->addStretch();
    addButton(row2, toggleAct);
    addButton(row2, deleteAct, false, true); // danger = true (Red button)
    v->addLayout(row1);
    v->addLayout(row2);

    connect(searchEdit, &QLineEdit::textChanged, this, [this] { refreshAccounts(); });
    connect(statusFilter, &QComboBox::currentIndexChanged, this, [this] { refreshAccounts(); });
    connect(accountTable, &QTableWidget::itemSelectionChanged, this, [this] { updateActions(); });
    connect(accountTable, &QTableWidget::cellDoubleClicked, this, [this] { doEdit(); });
    connect(accountTable, &QTableWidget::customContextMenuRequested, this,
            [this](const QPoint& p) { showAccountMenu(p); });
    return page;
}

// ============================================================================
// PAGE 2: TRANSACTIONS (Filter by account number / type + full history table)
// ============================================================================
QWidget* AdminWindow::buildTransactions()
{
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    v->setContentsMargins(24, 20, 24, 20);

    // [PAGE TITLE] Transactions heading and subtitle:
    makeTitle("Transactions", "Browse the transaction history of every account.", v);

    // [FILTER BAR] Account number search box and transaction type dropdown:
    historyAccount = new QLineEdit;
    historyAccount->setPlaceholderText("Filter by account number (blank = all)");
    historyAccount->setClearButtonEnabled(true);
    historyAccount->setMaxLength(9);
    historyAccount->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]*"), historyAccount));
    historyType = new QComboBox;
    historyType->addItems({"All types", "Deposit", "Withdrawal", "Transfer Sent", "Transfer Received", "Loan Disbursed", "Loan Repayment"});
    auto* row = new QHBoxLayout;
    row->addWidget(historyAccount, 1);
    row->addWidget(historyType);
    v->addLayout(row);
    historyTable = makeTable({});
    v->addWidget(historyTable, 1);

    connect(historyAccount, &QLineEdit::textChanged, this, [this] { refreshHistory(); });
    connect(historyType, &QComboBox::currentIndexChanged, this, [this] { refreshHistory(); });
    return page;
}

// ---------- Data Refresh & Table Population ----------

int AdminWindow::selectedAccount() const
{
    int row = accountTable->currentRow();
    if(row < 0 || !accountTable->item(row, 0) || accountTable->selectedItems().isEmpty())
        return -1;
    return accountTable->item(row, 0)->text().toInt();
}

// Refreshes all 5 pages and updates the pending badge counters on the sidebar.
void AdminWindow::refreshAll()
{
    refreshDashboard();
    refreshAccounts();
    refreshHistory();
    refreshRequests();
    refreshLoans();
    int pending = 0;
    for(const Account& a : bank.accounts())
        if(a.getStatus() == "Pending")
            pending++;
    // [SIDEBAR BADGE TEXT] Shows how many new accounts, deposit requests, or loans are waiting:
    sidebar->item(1)->setText(pending ? QString("Accounts  (%1 new)").arg(pending) : "Accounts");
    int deposits = bank.pendingDepositCount();
    sidebar->item(3)->setText(deposits ? QString("Deposit requests  (%1)").arg(deposits) : "Deposit requests");
    int pendingLoans = bank.pendingLoanCount();
    sidebar->item(4)->setText(pendingLoans ? QString("Loans  (%1)").arg(pendingLoans) : "Loans");
}

// Calculates totals and updates the 8 stat cards + recent transactions on the Dashboard page.
void AdminWindow::refreshDashboard()
{
    long long balance = 0, deposits = 0, withdrawals = 0, transfers = 0;
    int active = 0, logins = 0;
    std::map<std::string, int> types;
    for(const Account& a : bank.accounts())
    {
        balance += a.getBalancePaisa();
        types[a.getAccountType()]++;
        if(a.getStatus() == "Active")
            active++;
        if(a.getStatus() == "Pending")
            logins++;
    }
    auto txns = bank.history();
    for(const Transaction& t : txns)
    {
        if(t.type == "Deposit")
            deposits += t.amountPaisa;
        else if(t.type == "Withdrawal")
            withdrawals += t.amountPaisa;
        else if(t.type == "Transfer Sent")
            transfers += t.amountPaisa;
    }

    totalAccountsValue->setText(QString::number(bank.accounts().size()));
    activeValue->setText(QString::number(active));
    balanceValue->setText(money(balance));
    loginsValue->setText(QString::number(logins));
    loginsValue->setStyleSheet(logins > 0 ? "color: #b45309;" : "");
    depositsValue->setText(money(deposits));
    withdrawalsValue->setText(money(withdrawals));
    transfersValue->setText(money(transfers));
    loansValue->setText(money(bank.activeLoanDebt(0)));

    QStringList parts;
    for(const auto& e : types)
        parts << QString("%1: %2").arg(QString::fromStdString(e.first)).arg(e.second);
    typeBreakdown->setText(parts.isEmpty() ? "" : "Account types: " + parts.join("  |  "));

    // Show at most 15 recent transactions on the Dashboard:
    if(txns.size() > 15)
        txns.resize(15);
    fillHistory(recentTable, bank, txns, true);
}

// Populates the Accounts table based on the search box and status filter dropdown.
void AdminWindow::refreshAccounts()
{
    const QString filter = searchEdit->text().trimmed();
    const QString status = statusFilter->currentIndex() == 0 ? QString() : statusFilter->currentText();
    const int keep = selectedAccount();

    accountTable->setRowCount(0);
    for(const Account& a : bank.accounts())
    {
        QString number = QString::number(a.getAccountNumber());
        QString name = QString::fromStdString(a.getCustomerName());
        QString phone = QString::fromStdString(a.getPhoneNumber());
        QString st = QString::fromStdString(a.getStatus());
        if(!filter.isEmpty() && !number.contains(filter) && !name.contains(filter, Qt::CaseInsensitive) &&
           !phone.contains(filter))
            continue;
        if(!status.isEmpty() && st != status)
            continue;

        int row = accountTable->rowCount();
        accountTable->insertRow(row);
        accountTable->setItem(row, 0, cell(number));
        accountTable->setItem(row, 1, cell(name));
        accountTable->setItem(row, 2, cell(phone));
        accountTable->setItem(row, 3, cell(QString::fromStdString(a.getAccountType())));
        accountTable->setItem(row, 4, cell(money(a.getBalancePaisa()), true));
        long long debt = bank.activeLoanDebt(a.getAccountNumber());
        auto* debtItem = cell(debt > 0 ? money(debt) : "-", true);
        if(debt > 0)
            debtItem->setForeground(QColor("#b45309"));
        accountTable->setItem(row, 5, debtItem);
        auto* s = cell(st);
        // [STATUS COLORS] Green for Active, Orange for Pending, Red for Closed:
        s->setForeground(st == "Active" ? QColor("#15803d") : st == "Pending" ? QColor("#b45309") : QColor("#b91c1c"));
        accountTable->setItem(row, 6, s);
        QString login = bank.loginName(a.getAccountNumber());
        accountTable->setItem(row, 7, cell(login.isEmpty() ? "-" : login));
        if(a.getAccountNumber() == keep)
            accountTable->selectRow(row);
    }
    updateActions();
}

// Populates the Transactions table filtered by account number and transaction type.
void AdminWindow::refreshHistory()
{
    int account = 0;
    if(!historyAccount->text().trimmed().isEmpty())
        account = historyAccount->text().trimmed().toInt();
    auto txns = bank.history(account);
    if(historyType->currentIndex() > 0)
    {
        std::string type = historyType->currentText().toStdString();
        txns.erase(std::remove_if(txns.begin(), txns.end(),
                                  [&](const Transaction& t) { return t.type != type; }),
                   txns.end());
    }
    if(account < 0 || (account == 0 && !historyAccount->text().trimmed().isEmpty()))
        txns.clear();
    fillHistory(historyTable, bank, txns, true);
}

// Enables/disables buttons and updates dynamic button labels depending on which account is selected.
void AdminWindow::updateActions()
{
    int n = selectedAccount();
    const Account* a = bank.find(n);
    bool sel = a != nullptr;
    bool active = sel && a->getStatus() == "Active";
    editAct->setEnabled(sel);
    depositAct->setEnabled(active);
    withdrawAct->setEnabled(active);
    transferAct->setEnabled(true);
    toggleAct->setEnabled(sel);
    bool pending = sel && a->getStatus() == "Pending";
    // [DYNAMIC BUTTON TEXT] Changes between "Activate account", "Reopen account", and "Close account":
    toggleAct->setText(pending ? "Activate account" : sel && !active ? "Reopen account" : "Close account");
    loginAct->setEnabled(sel);
    loginAct->setText(sel && bank.hasLogin(n) ? "Reset customer login" : "Set customer login");
    removeLoginAct->setEnabled(sel && bank.hasLogin(n));
    historyAct->setEnabled(sel);
    deleteAct->setEnabled(sel);
    newAct->setEnabled(true);
}

// Shows success messages in the bottom status bar or error messages in a warning pop-up.
void AdminWindow::report(const Result& r)
{
    if(r.ok)
        statusBar()->showMessage(r.message, 7000);
    else
        QMessageBox::warning(this, "SecureBank", r.message);
    refreshAll();
}

// Shows the right-click context menu on a row in the Accounts table.
void AdminWindow::showAccountMenu(const QPoint& pos)
{
    if(!accountTable->itemAt(pos))
        return;
    accountTable->selectRow(accountTable->itemAt(pos)->row());
    QMenu menu(this);
    for(QAction* a : {editAct, depositAct, withdrawAct, transferAct, historyAct})
        menu.addAction(a);
    menu.addSeparator();
    for(QAction* a : {loginAct, removeLoginAct, toggleAct, deleteAct})
        menu.addAction(a);
    menu.exec(accountTable->viewport()->mapToGlobal(pos));
}

// ---------- Account Button Click Handlers ----------

void AdminWindow::doReload()
{
    bank.reload();
    refreshAll();
    statusBar()->showMessage("Data reloaded from disk.", 4000);
}

void AdminWindow::doNew()
{
    QString name, phone, type;
    if(!askAccountDetails(this, "New account", name, phone, type, false))
        return;
    Result r = bank.createAccount(name, phone, type);
    report(r);
    if(r.ok)
    {
        // Select the newly created account and offer to set up its customer login immediately:
        searchEdit->clear();
        statusFilter->setCurrentIndex(0);
        refreshAccounts();
        for(int i = 0; i < accountTable->rowCount(); i++)
            if(accountTable->item(i, 0)->text().toInt() == r.value)
                accountTable->selectRow(i);
        if(QMessageBox::question(this, "Customer login",
                                 "Create an online login for this customer now?") == QMessageBox::Yes)
            doLogin();
    }
}

void AdminWindow::doEdit()
{
    const Account* a = bank.find(selectedAccount());
    if(!a)
        return;
    int number = a->getAccountNumber();
    QString name = QString::fromStdString(a->getCustomerName());
    QString phone = QString::fromStdString(a->getPhoneNumber());
    QString type = QString::fromStdString(a->getAccountType());
    if(!askAccountDetails(this, QString("Edit account %1").arg(number), name, phone, type, true))
        return;
    report(bank.updateAccount(number, name, phone, type));
}

void AdminWindow::doDeposit()
{
    const Account* a = bank.find(selectedAccount());
    if(!a)
        return;
    int number = a->getAccountNumber();
    long long amount;
    if(askAmount(this, "Deposit",
                 QString("Deposit into <b>%1</b> (%2)<br>Current balance: <b>%3</b>")
                     .arg(QString::fromStdString(a->getCustomerName())).arg(number)
                     .arg(money(a->getBalancePaisa())), amount))
        report(bank.deposit(number, amount));
}

void AdminWindow::doWithdraw()
{
    const Account* a = bank.find(selectedAccount());
    if(!a)
        return;
    int number = a->getAccountNumber();
    long long amount;
    if(askAmount(this, "Withdraw",
                 QString("Withdraw from <b>%1</b> (%2)<br>Available balance: <b>%3</b>")
                     .arg(QString::fromStdString(a->getCustomerName())).arg(number)
                     .arg(money(a->getBalancePaisa())), amount))
    {
        Result r = bank.withdraw(number, amount);
        report(r);
        // [WITHDRAWAL POPUP] Shown after an admin withdraws money from an account:
        if(r.ok)
            QMessageBox::information(this, "Withdrawal successful",
                                     QString("Withdrew <b>%1</b> from account %2.<br>%3")
                                         .arg(money(amount)).arg(number).arg(r.message.section(". ", 1)));
    }
}

void AdminWindow::doTransfer()
{
    int from, to;
    long long amount;
    QString receiverName;
    if(askTransfer(this, bank, 0, selectedAccount(), from, to, amount, receiverName))
        report(bank.transfer(from, to, amount));
}

void AdminWindow::doToggle()
{
    const Account* a = bank.find(selectedAccount());
    if(!a)
        return;
    int number = a->getAccountNumber();
    bool reopen = a->getStatus() != "Active"; // includes activating a pending account
    if(!reopen && QMessageBox::question(this, "Close account",
                                        QString("Close account %1? The customer will not be able to transact or sign in.")
                                            .arg(number)) != QMessageBox::Yes)
        return;
    report(bank.setActive(number, reopen));
}

void AdminWindow::doDelete()
{
    const Account* a = bank.find(selectedAccount());
    if(!a)
        return;
    int number = a->getAccountNumber();
    if(a->getBalancePaisa() != 0)
    {
        QMessageBox::warning(this, "Cannot delete account",
                             QString("Account %1 still has a balance of %2.\nWithdraw or transfer the money first.")
                                 .arg(number).arg(money(a->getBalancePaisa())));
        return;
    }
    long long debt = bank.activeLoanDebt(number);
    if(debt > 0)
    {
        QMessageBox::warning(this, "Cannot delete account",
                             QString("Account %1 has an unpaid active loan of %2.\nThe loan must be repaid before deleting the account.")
                                 .arg(number).arg(money(debt)));
        return;
    }
    bool ok = false;
    QString typed = QInputDialog::getText(
        this, "Delete account",
        QString("This permanently deletes account <b>%1</b> (%2), its customer login, deposit requests, and loan records. "
                "This cannot be undone.<br><br>Type the account number to confirm:")
            .arg(number).arg(QString::fromStdString(a->getCustomerName())),
        QLineEdit::Normal, "", &ok).trimmed();
    if(!ok)
        return;
    if(typed != QString::number(number))
    {
        QMessageBox::warning(this, "Delete account", "The account number did not match. Nothing was deleted.");
        return;
    }
    report(bank.deleteAccount(number));
}

void AdminWindow::doLogin()
{
    const Account* a = bank.find(selectedAccount());
    if(!a)
        return;
    int number = a->getAccountNumber();
    bool existing = bank.hasLogin(number);
    QString username = existing ? bank.loginName(number)
                                : QString::fromStdString(a->getCustomerName()).toLower().remove(QRegularExpression("[^a-z0-9]"));
    QString password;
    if(!askCredentials(this, existing ? "Reset customer login" : "Set customer login",
                       QString("Online login for <b>%1</b> (account %2).<br>Share the credentials with the customer securely.")
                           .arg(QString::fromStdString(a->getCustomerName())).arg(number),
                       username, password, false))
        return;
    report(bank.setCustomerLogin(number, username, password));
}

void AdminWindow::doRemoveLogin()
{
    int n = selectedAccount();
    if(n < 0)
        return;
    if(QMessageBox::question(this, "Remove login",
                             QString("Remove the customer login for account %1?").arg(n)) == QMessageBox::Yes)
        report(bank.removeCustomerLogin(n));
}

void AdminWindow::doHistory()
{
    int n = selectedAccount();
    if(n < 0)
        return;
    historyAccount->setText(QString::number(n));
    historyType->setCurrentIndex(0);
    sidebar->setCurrentRow(2);
}

// ============================================================================
// PAGE 3: DEPOSIT REQUESTS (Filter + Approve/Reject buttons + requests table)
// ============================================================================
QWidget* AdminWindow::buildRequests()
{
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    v->setContentsMargins(24, 20, 24, 20);

    // [PAGE TITLE] Deposit requests heading and subtitle:
    makeTitle("Deposit requests", "Customers request deposits here. Approving credits the account immediately.", v);

    // [FILTER & ACTION BUTTONS] Status filter dropdown + "Approve" and "Reject" buttons:
    requestFilter = new QComboBox;
    requestFilter->addItems({"Pending", "Approved", "Rejected", "All"});
    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel("Show:"));
    row->addWidget(requestFilter);
    row->addStretch();
    approveBtn = makeButton("Approve", true);      // Blue primary button
    rejectBtn = makeButton("Reject", false, true); // Red danger button
    row->addWidget(approveBtn);
    row->addWidget(rejectBtn);
    v->addLayout(row);

    // [DEPOSIT REQUESTS TABLE] Change column headers here:
    requestTable = makeTable({"Request ID", "Requested", "Account", "Customer", "Amount", "Status", "Decided"});
    v->addWidget(requestTable, 1);

    connect(requestFilter, &QComboBox::currentIndexChanged, this, [this] { refreshRequests(); });
    connect(requestTable, &QTableWidget::itemSelectionChanged, this, [this] {
        bool pending = selectedRequest() > 0 && requestTable->item(requestTable->currentRow(), 5)->text() == "Pending";
        approveBtn->setEnabled(pending);
        rejectBtn->setEnabled(pending);
    });
    connect(approveBtn, &QPushButton::clicked, this, [this] { doApprove(); });
    connect(rejectBtn, &QPushButton::clicked, this, [this] { doReject(); });
    approveBtn->setEnabled(false);
    rejectBtn->setEnabled(false);
    return page;
}

int AdminWindow::selectedRequest() const
{
    int row = requestTable->currentRow();
    if(row < 0 || requestTable->selectedItems().isEmpty() || !requestTable->item(row, 0))
        return -1;
    return requestTable->item(row, 0)->text().toInt();
}

void AdminWindow::refreshRequests()
{
    const QString filter = requestFilter->currentText();
    requestTable->setRowCount(0);
    for(const DepositRequest& d : bank.depositRequests())
    {
        if(filter != "All" && d.status != filter)
            continue;
        const Account* a = bank.find(d.accountNumber);
        int row = requestTable->rowCount();
        requestTable->insertRow(row);
        requestTable->setItem(row, 0, cell(QString::number(d.id)));
        requestTable->setItem(row, 1, cell(d.requestedAt));
        requestTable->setItem(row, 2, cell(QString::number(d.accountNumber)));
        requestTable->setItem(row, 3, cell(a ? QString::fromStdString(a->getCustomerName()) : "-"));
        requestTable->setItem(row, 4, cell(money(d.amountPaisa), true));
        auto* st = cell(d.status);
        st->setForeground(d.status == "Approved" ? QColor("#15803d")
                          : d.status == "Rejected" ? QColor("#b91c1c") : QColor("#b45309"));
        requestTable->setItem(row, 5, st);
        requestTable->setItem(row, 6, cell(d.decidedAt.isEmpty() ? "-" : d.decidedAt));
    }
}

void AdminWindow::doApprove()
{
    int id = selectedRequest();
    if(id < 0)
        return;
    for(const DepositRequest& d : bank.depositRequests())
        if(d.id == id)
        {
            if(QMessageBox::question(this, "Approve deposit",
                                     QString("Approve deposit of <b>%1</b> to account %2?")
                                         .arg(money(d.amountPaisa)).arg(d.accountNumber)) != QMessageBox::Yes)
                return;
            break;
        }
    report(bank.approveDeposit(id));
}

void AdminWindow::doReject()
{
    int id = selectedRequest();
    if(id < 0)
        return;
    if(QMessageBox::question(this, "Reject deposit", "Reject this deposit request?") == QMessageBox::Yes)
        report(bank.rejectDeposit(id));
}

// ============================================================================
// PAGE 4: LOANS (Filter + Approve/Reject loan buttons + loans table)
// ============================================================================
QWidget* AdminWindow::buildLoans()
{
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    v->setContentsMargins(24, 20, 24, 20);

    // [PAGE TITLE] Loans heading and subtitle:
    makeTitle("Loans", "Review customer loan applications. Approving a loan credits the customer's account immediately.", v);

    // [FILTER & ACTION BUTTONS] Status filter dropdown + "Approve loan" and "Reject loan" buttons:
    loanFilter = new QComboBox;
    loanFilter->addItems({"Pending", "Active", "Paid", "Rejected", "All"});
    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel("Show:"));
    row->addWidget(loanFilter);
    row->addStretch();
    approveLoanBtn = makeButton("Approve loan", true);      // Blue primary button
    rejectLoanBtn = makeButton("Reject loan", false, true); // Red danger button
    row->addWidget(approveLoanBtn);
    row->addWidget(rejectLoanBtn);
    v->addLayout(row);

    // [LOANS TABLE] Change column headers here:
    loanTable = makeTable({"Loan ID", "Requested", "Account", "Customer", "Purpose", "Amount", "Remaining Due", "Status", "Decided"});
    v->addWidget(loanTable, 1);

    connect(loanFilter, &QComboBox::currentIndexChanged, this, [this] { refreshLoans(); });
    connect(loanTable, &QTableWidget::itemSelectionChanged, this, [this] {
        bool pending = selectedLoan() > 0 && loanTable->item(loanTable->currentRow(), 7)->text() == "Pending";
        approveLoanBtn->setEnabled(pending);
        rejectLoanBtn->setEnabled(pending);
    });
    connect(approveLoanBtn, &QPushButton::clicked, this, [this] { doApproveLoan(); });
    connect(rejectLoanBtn, &QPushButton::clicked, this, [this] { doRejectLoan(); });
    approveLoanBtn->setEnabled(false);
    rejectLoanBtn->setEnabled(false);
    return page;
}

int AdminWindow::selectedLoan() const
{
    int row = loanTable->currentRow();
    if(row < 0 || loanTable->selectedItems().isEmpty() || !loanTable->item(row, 0))
        return -1;
    return loanTable->item(row, 0)->text().toInt();
}

void AdminWindow::refreshLoans()
{
    const QString filter = loanFilter->currentText();
    loanTable->setRowCount(0);
    for(const LoanRequest& l : bank.loanRequests())
    {
        if(filter != "All" && l.status != filter)
            continue;
        const Account* a = bank.find(l.accountNumber);
        int row = loanTable->rowCount();
        loanTable->insertRow(row);
        loanTable->setItem(row, 0, cell(QString::number(l.id)));
        loanTable->setItem(row, 1, cell(l.requestedAt));
        loanTable->setItem(row, 2, cell(QString::number(l.accountNumber)));
        loanTable->setItem(row, 3, cell(a ? QString::fromStdString(a->getCustomerName()) : "-"));
        loanTable->setItem(row, 4, cell(l.purpose));
        loanTable->setItem(row, 5, cell(money(l.amountPaisa), true));
        loanTable->setItem(row, 6, cell(money(l.remainingPaisa), true));
        auto* st = cell(l.status);
        st->setForeground((l.status == "Active" || l.status == "Paid") ? QColor("#15803d")
                          : l.status == "Rejected" ? QColor("#b91c1c") : QColor("#b45309"));
        loanTable->setItem(row, 7, st);
        loanTable->setItem(row, 8, cell(l.decidedAt.isEmpty() ? "-" : l.decidedAt));
    }
    bool pending = selectedLoan() > 0 && loanTable->item(loanTable->currentRow(), 7)->text() == "Pending";
    approveLoanBtn->setEnabled(pending);
    rejectLoanBtn->setEnabled(pending);
}

void AdminWindow::doApproveLoan()
{
    int id = selectedLoan();
    if(id < 0)
        return;
    for(const LoanRequest& l : bank.loanRequests())
        if(l.id == id)
        {
            if(QMessageBox::question(this, "Approve loan",
                                     QString("Approve <b>%1</b> loan of <b>%2</b> for account %3?")
                                         .arg(l.purpose).arg(money(l.amountPaisa)).arg(l.accountNumber)) != QMessageBox::Yes)
                return;
            break;
        }
    report(bank.approveLoan(id));
}

void AdminWindow::doRejectLoan()
{
    int id = selectedLoan();
    if(id < 0)
        return;
    if(QMessageBox::question(this, "Reject loan", "Reject this loan application?") == QMessageBox::Yes)
        report(bank.rejectLoan(id));
}
