# SecureBank Banking Management System

A C++17 banking management system with two front-ends that share the same
account and file-storage code:

- a **Qt6 desktop GUI** with separate **Admin** and **Customer** sign-in, and
- the original **console (CLI)** application for administrators.

Data is stored in human-readable text files, so no database is required.

## Contents

- [Features](#features)
- [Requirements](#requirements)
- [Build and Run](#build-and-run)
- [Using the GUI](#using-the-gui)
- [Business Rules and Validation](#business-rules-and-validation)
- [Security Notes](#security-notes)
- [Project Structure](#project-structure)
- [Data Files](#data-files)
- [Tests](#tests)
- [Documentation](#documentation)
- [Known Limitations](#known-limitations)

## Features

### Administrator

- Dashboard: total/active/pending accounts, total balance, deposits, withdrawals,
  transfers, account-type breakdown and recent transactions
- Create, edit, search, filter, close, reopen and **delete** accounts
- **Activate** accounts that customers registered themselves
- Deposit, withdraw and transfer on behalf of a customer
- **Approve or reject** customer deposit requests
- Create, reset or remove customer logins
- Browse and filter the transaction history of all accounts

### Customer

- **Self-registration** from the sign-in window (account starts as *Pending*
  until an admin activates it)
- Sign in with username and password
- Overview with balance and recent activity
- **Send money** to another account, with receiver verification
- **Withdraw** money (password required)
- **Request a deposit** (credited only after admin approval)
- Transaction history with type filter, deposit request status
- Change phone number and password

### Console version

- Admin-only menu: create, view, search, update, close and reopen accounts;
  deposit, withdraw, transfer; transaction history; banking reports

## Requirements

| Component | Needed for | Notes |
|-----------|-----------|-------|
| C++17 compiler (g++) | everything | |
| CMake 3.16+ | GUI | |
| Qt 6 Widgets | GUI | e.g. `sudo apt install qt6-base-dev cmake` |

The console version only needs `g++`.

## Build and Run

### GUI (Qt6)

```bash
./run_gui.sh
```

This configures CMake in `build/cmake`, builds the `securebank_gui` target and
launches it. Manual equivalent:

```bash
cmake -S . -B build/cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/cmake -j"$(nproc)"
./build/cmake/securebank_gui      # run from the project root
```

The GUI looks for the `data/` folder in the current directory (or next to the
build folder) and creates it if missing. On Windows, build with CMake and a Qt 6
kit (for example from Qt Creator or `cmake -DCMAKE_PREFIX_PATH=<Qt dir>`).

`CMakeLists.txt` also builds the console executable `securebank`; the GUI target
is skipped automatically if Qt 6 is not found.

### Console version

Linux:

```bash
./run.sh
```

Windows PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\run.ps1
```

The scripts compile every `.cpp` file in `src/` (not `src/gui/`) with
`-std=c++17 -Wall -Wextra -pedantic`, then run the program. Admin credentials
for the console are the same as for the GUI (see below).

## Using the GUI

### Signing in

The sign-in window has a **Customer** and an **Admin** tab, a *Show password*
option, and a limit of **3 failed attempts** per sign-in window.

| Role | Credentials |
|------|-------------|
| Admin | `admin` / `admin123` (fixed, same as the console version) |
| Customer | the username and password chosen at registration, or set by an admin |

Closed accounts cannot sign in. Pending accounts show *"awaiting activation by
an administrator"*.

### Account lifecycle

```text
 Customer registers  ->  Pending  --admin activates-->  Active  <--reopen/close-->  Closed
 Admin creates account ---------------------------------> Active
 Admin deletes (balance must be 0, number typed to confirm)  ->  removed
```

1. A customer clicks **"New customer? Create a bank account"**, enters name,
   phone, account type, username and password. The account is created as
   **Pending** with a balance of 0.
2. The admin sees the pending count on the dashboard and in the sidebar
   (*Accounts (N pending)*), selects the account and clicks **Activate account**.
3. The customer can now sign in and transact.
4. Accounts created directly by an admin are active immediately; the admin can
   optionally create the customer's login at the same time.

### Admin screens

- **Dashboard** - summary cards and the 15 most recent transactions.
- **Accounts** - searchable table (number, name, phone) with status filter.
  Buttons and the right-click menu offer: New account, Edit details, Deposit,
  Withdraw, Transfer, View history, Set/Reset/Remove customer login,
  Close/Reopen/Activate, Delete account. Double-click a row to edit it.
- **Transactions** - history of all accounts, filter by account number and type.
- **Deposit requests** - pending, approved and rejected requests. Select a
  pending request and **Approve** (credits the account and records a Deposit
  transaction) or **Reject**.
- Sidebar: **Reload data** (re-read files from disk) and **Sign out**.

### Customer screens

- **Overview** - balance card, *Send money*, *Deposit*, *Withdraw* and recent
  activity.
- **Transactions** - full history (green `+` for money in, red `-` for money out).
- **Deposit requests** - status of every deposit request.
- **Profile & security** - account details, change phone number, change password.

### Customer money operations

| Operation | How it works |
|-----------|--------------|
| Send money | Enter the receiver's account number **and the receiver's name**. The name must match the account holder (case and extra spaces ignored), otherwise the transfer is refused. A confirmation shows amount, name and account number. The sender needs enough balance and both accounts must be active. |
| Withdraw | Enter amount and **your password**. A wrong password cancels the withdrawal; after 3 wrong passwords you are signed out. |
| Deposit | Submits a **deposit request** (max 10 pending). The balance changes only when an admin approves it. |

Every successful withdrawal shows a confirmation pop-up with the new balance.

## Business Rules and Validation

All rules are enforced in the dialogs *and* again in the service layer
(`src/gui/BankService.cpp`).

| Input | Rule |
|-------|------|
| Customer name | 1-50 characters; letters, spaces, `.`, `-`, `'` only |
| Phone number | 7-15 digits |
| Username | 3-20 characters: letters, digits, `_`, `.`; unique (case-insensitive) |
| Password | 6-64 characters, no spaces or `|`; a changed password must differ from the old one |
| Amount | positive, at most 2 decimals, at most 100,000,000.00 per transaction |
| Account number | digits only; must exist |
| Transfer | different sender and receiver, both active, sufficient balance, receiver name verified (customers) |
| Delete account | balance must be 0; admin must type the account number |
| Account types | Savings, Current, Student, Business |

Money is stored internally as **integer paisa** to avoid floating-point
rounding errors; files show it with two decimals.

Failed saves are rolled back in memory, and account/transaction files are
written through temporary files, so a failed write does not corrupt data.

## Security Notes

- Customer passwords are stored as **salted SHA-256 hashes** in
  `data/customers.txt`; plain-text passwords are never written.
- Admin credentials are **hard-coded** (`admin` / `admin123`) in
  `src/gui/LoginDialog.cpp` and `src/main.cpp`. Change them before any real use.
- Runtime data files contain personal information and are excluded from git.
- This is an educational project, not production banking software.

## Project Structure

```text
.
|-- CMakeLists.txt            Builds console (securebank) and GUI (securebank_gui)
|-- run.sh / run.ps1          Build and run the console version
|-- run_gui.sh                Build and run the Qt6 GUI
|-- test.sh / test.ps1        Build and run the automated tests
|-- src/
|   |-- main.cpp              Console entry point and menu
|   |-- Account.h/.cpp        Account model and balance rules
|   |-- BankSystem.h/.cpp     Console workflows (input/output)
|   |-- FileManager.h/.cpp    Loading/saving accounts and transactions
|   `-- gui/
|       |-- main_gui.cpp      GUI entry point, sign-in/sign-out loop
|       |-- LoginDialog       Customer/Admin sign-in and registration entry
|       |-- AdminWindow       Admin dashboard, accounts, transactions, deposit requests
|       |-- CustomerWindow    Customer overview, history, deposits, profile
|       |-- Dialogs           Reusable validated dialogs and table helpers
|       |-- BankService       Shared GUI business logic and persistence
|       `-- Theme.h           Application stylesheet and small UI helpers
|-- tests/test_main.cpp       Automated tests for Account and FileManager
|-- data/                     Runtime data (git-ignored, see below)
`-- docs/                     Design, data format, testing and limitations
```

### Architecture

```text
 AdminWindow / CustomerWindow / LoginDialog   (Qt widgets)
                 |
            BankService                       (rules, validation, customer logins, deposit requests)
                 |
   Account  +  FileManager                    (shared with the console version)
                 |
            data/*.txt
```

The console version (`BankSystem`) uses `Account` and `FileManager` directly and
is unaware of customer logins, pending accounts and deposit requests.

## Data Files

All files live in `data/`, use `|` as the field separator, and are created on
first use. They are listed in `.gitignore` (only `data/.gitkeep` is committed).

| File | Contents |
|------|----------|
| `accounts.txt` | `account_number\|name\|phone\|type\|balance\|status` |
| `transactions.txt` | `id\|account\|type\|amount\|date_time\|related_account` |
| `customers.txt` | `account_number\|username\|salt\|sha256_hash` |
| `deposit_requests.txt` | `id\|account\|amount_paisa\|requested_at\|status\|decided_at` |

Account status is `Active`, `Closed` or `Pending`. Deposit request status is
`Pending`, `Approved` or `Rejected`. Transaction types: `Deposit`, `Withdrawal`,
`Transfer Sent`, `Transfer Received`. See [docs/data-format.md](docs/data-format.md).

Deleting an account removes it, its login and its deposit requests; its past
transactions are kept for auditing.

## Tests

```bash
./test.sh            # Linux
.\test.ps1           # Windows PowerShell
```

The suite covers balance rules, account and transaction file round trips,
malformed/duplicate record handling and field validation (`Account` and
`FileManager`). The GUI service layer is not covered by the automated suite; see
[docs/testing.md](docs/testing.md).

## Documentation

- [docs/system-design.md](docs/system-design.md) - architecture and workflows
- [docs/data-format.md](docs/data-format.md) - file formats
- [docs/testing.md](docs/testing.md) - test coverage and manual checklist
- [docs/limitations.md](docs/limitations.md) - known limitations

## Known Limitations

- Text-file storage; no concurrent multi-user access (the GUI re-reads the files on
  sign-in, on the admin's *Reload data* button and when a customer switches
  pages, but two running instances can still overwrite each other's changes).
- Single hard-coded admin account.
- SHA-256 with a salt is used for passwords; a production system should use a
  slow password hash (bcrypt/Argon2).
- No interest, loans, cards or per-admin audit trail.
- The console version does not support customer login, pending accounts or
  deposit approval.
