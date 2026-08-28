# SecureBank Testing

Run the automated tests with `./test.sh` on Linux or `./test.ps1` in Windows PowerShell.

The test suite covers:

- Positive and invalid deposits
- Positive, negative, zero, and excessive withdrawals
- Transfer balance behavior
- Closed-account state preservation
- Account file save and load round trips
- Malformed, invalid, and duplicate account records
- Transaction file save and load round trips
- Invalid transaction records

The application should also be compiled with `./run.sh` or `./run.ps1` before demonstration.
