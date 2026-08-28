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
|   `-- main.cpp
`-- README.md
```

## Build and Run

### Linux

Make the script executable once:

```bash
chmod +x build.sh
```

Build and run the application:

```bash
./build.sh
```

### Windows PowerShell

Build and run the application:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\build.ps1
```

Each script compiles all C++ source files in `src/` with C++17 and strict
warning flags, then runs the executable from the generated `build/` directory.

The current version provides the main menu. Features will be implemented incrementally.
