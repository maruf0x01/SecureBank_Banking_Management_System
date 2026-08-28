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

## Money Representation

Balances and transaction amounts are stored internally as integer paisa. This
prevents floating-point rounding errors. Text files display money in ordinary
decimal form with two digits after the decimal point.

Additional documentation will be added in later milestones.
