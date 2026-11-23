// ============================================================================
// FILE: main_gui.cpp
// PURPOSE: Entry point (main function) of the Qt6 GUI application.
//          1. Initializes Qt and applies the visual theme from Theme.h
//          2. Ensures the "data/" folder exists
//          3. Runs a loop: LoginDialog -> AdminWindow or CustomerWindow -> Sign out
// ============================================================================
#include <QApplication>
#include <QDir>
#include <QEventLoop>

#include "AdminWindow.h"
#include "BankService.h"
#include "CustomerWindow.h"
#include "LoginDialog.h"
#include "Theme.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SecureBank"); // [APP NAME] Global application name
    app.setQuitOnLastWindowClosed(false);
    applyTheme(app);                      // Apply colors and styles from Theme.h

    // Locate the project's data/ directory (current working dir, or parent of build/).
    QDir dir(QDir::currentPath());
    if(!dir.exists("data"))
    {
        QDir appDir(QApplication::applicationDirPath());
        if(appDir.cd("..") && appDir.exists("data"))
            dir = appDir;
    }
    dir.mkpath("data");
    QDir::setCurrent(dir.absolutePath());

    // Load accounts, transactions, customer logins, and deposit requests.
    BankService bank;

    // Session loop: when a user clicks "Sign out", the login window opens again.
    while(true)
    {
        // Step 1: Show the Sign-in dialog
        LoginDialog login(bank);
        if(login.exec() != QDialog::Accepted)
            break; // User clicked "Exit" or closed the login window

        // Step 2: Open either AdminWindow or CustomerWindow based on who logged in
        bank.reload();
        QMainWindow* window = nullptr;
        bool signedOut = false;
        if(login.role() == LoginDialog::Admin)
        {
            auto* w = new AdminWindow(bank);
            QObject::connect(w, &AdminWindow::loggedOut, [&] { signedOut = true; });
            window = w;
        }
        else
        {
            auto* w = new CustomerWindow(bank, login.accountNumber());
            QObject::connect(w, &CustomerWindow::loggedOut, [&] { signedOut = true; });
            window = w;
        }

        // Step 3: Show the dashboard window and wait until it closes
        window->setAttribute(Qt::WA_DeleteOnClose);
        QEventLoop loop;
        QObject::connect(window, &QObject::destroyed, &loop, &QEventLoop::quit);
        window->show();
        loop.exec();

        // If the window was closed with the top-right [X] button (not "Sign out"), quit the app.
        if(!signedOut)
            break;
    }
    return 0;
}
