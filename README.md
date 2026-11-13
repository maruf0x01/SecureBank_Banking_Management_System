# SecureBank Banking Management System

A C++ banking management system for managing customer accounts and financial transactions.

## Current Scope

- Admin-only access
- Account creation and management
- Deposits and withdrawals
- Fund transfers
- Transaction history
- Text-file data storage

Customer login will be considered as a future enhancement.

## Project Structure

```text
SecureBank_Banking_Management_System/
|-- src/
|   |-- main.cpp
|   |-- gui/            (Qt6 GUI: windows, dialogs, BankService)
|   |-- Account.cpp
|   |-- Account.h
|   |-- BankSystem.cpp
|   |-- BankSystem.h
|   |-- FileManager.cpp
|   `-- FileManager.h
|-- data/
|   |-- accounts.txt
|   `-- transactions.txt
|-- tests/
|-- docs/
|   |-- system-design.md
|   |-- data-format.md
|   |-- testing.md
|   `-- limitations.md
|-- CMakeLists.txt
`-- README.md
```

## Build and Run

### Linux

Make the script executable once:

```bash
chmod +x run.sh
```

Build and run the application:

```bash
./run.sh
```

### Windows PowerShell

Build and run the application:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\run.ps1
```

Each script compiles all C++ source files in `src/` with C++17 and strict
warning flags, then runs the executable from the generated `build/` directory.

## Qt6 GUI

A Qt6 Widgets front-end lives in `src/gui/` and shares the same `Account` and
`FileManager` code and `data/` files as the console version. The sign-in window
has two roles.

**Admin** (`admin` / `admin123`)

- Dashboard with account, balance and transaction totals
- Create, edit, close/reopen and **delete** accounts (deletion needs a zero
  balance and typing the account number to confirm)
- **Activate** accounts that customers registered themselves (pending accounts)
- Deposit, withdraw and transfer on behalf of customers
- **Approve or reject deposit requests** from customers
- Create, reset or remove customer logins
- Search, filter and browse transaction history

**Customer**

- **Self-registration**: create a bank account from the sign-in window. The
  account stays *Pending* until an admin activates it.
- Overview with balance and recent activity
- **Send money**: the receiver's account number and holder name must match
  before the transfer is allowed, followed by a confirmation
- **Withdraw**: requires the customer's password (3 wrong attempts sign out)
- **Deposit**: submitted as a request; the balance is credited only after admin
  approval
- Transaction history, change phone number, change password

All inputs are validated (names, phone numbers, usernames, passwords, amounts
up to 100,000,000.00 per transaction, account numbers).

Customer passwords are stored as salted SHA-256 hashes.

Requirements: CMake 3.16+, a C++17 compiler, and Qt 6 (Widgets), e.g.
`sudo apt install qt6-base-dev cmake`.

```bash
./run_gui.sh
```

Or manually: `cmake -S . -B build/cmake && cmake --build build/cmake`, then run
`./build/cmake/securebank_gui` from the project root. The console version is
still available through `./run.sh`.

## Run Tests

On Linux, run the validation suite with:

```bash
chmod +x test.sh
./test.sh
```

On Windows PowerShell, run:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\test.ps1
```

The tests cover account balance rules, account file persistence, malformed
record handling, and transaction file persistence.

## Current Features

- Admin-only access with three login attempts
- Create, view, search, update, and close bank accounts
- Persistent account records stored in `data/accounts.txt`
- Safe temporary-file replacement during account saves
- Input validation for account numbers and text-file delimiters
- Deposits and withdrawals for active accounts
- Insufficient-balance protection for withdrawals
- Fund transfers between two active accounts
- Selective account updates with Enter-to-keep-current support
- Transaction history for deposits, withdrawals, and transfers
- Validation tests for balances and text-file records
- Banking reports with balance, transaction, status, and account-type summaries
- Customer names limited to 50 characters
- Phone numbers validated as 7-15 digits

## Money Representation

Balances and transaction amounts are stored internally as integer paisa. This
prevents floating-point rounding errors. Text files display money in ordinary
decimal form with two digits after the decimal point.

## Runtime Data

The files in `data/` (`accounts.txt`, `transactions.txt`, `customers.txt`,
`deposit_requests.txt`) are created and updated at runtime. They hold customer
information and password hashes, so they are listed in `.gitignore` and are not
committed. The directory is created automatically on first run.

Account statuses are `Active`, `Closed` and `Pending` (awaiting admin
activation). The console version does not know about pending accounts.
