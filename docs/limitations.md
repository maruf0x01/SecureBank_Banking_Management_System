# SecureBank Limitations

- Text files are suitable for local educational use but are not a substitute for a production database.
- The administrator account is fixed (`admin` / `admin123`) and hard-coded in the source.
- Customer passwords use a salted SHA-256 hash; a production system should use a slow password hash such as bcrypt or Argon2.
- Concurrent multi-user access is not supported. Two running instances can overwrite each other's changes; the GUI re-reads data on sign-in, when a customer switches pages, and on the admin's "Reload data" button.
- Transaction rollback depends on local file-system operations.
- The console version has no customer login and does not handle pending accounts or deposit requests.
- Customers cannot change their name or account type; an admin must do it.
- There is no password recovery; an admin resets a customer's login.
- Interest, loans, cards, and per-admin audit trails are outside the current scope.
- The GUI business logic (`BankService`) has no automated test suite yet.
