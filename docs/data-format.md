# SecureBank Data Format

## Accounts

File: `data/accounts.txt`

Each line contains:

```text
account_number|customer_name|phone_number|account_type|balance|status
100001|Example Customer|01700000000|Savings|1250.50|Active
```

The balance is stored as a readable decimal value in the file and converted to integer paisa in memory. Valid statuses are `Active` and `Closed`. The pipe character is reserved as the field delimiter.

## Transactions

File: `data/transactions.txt`

```text
transaction_id|account_number|type|amount|date_time|related_account_number
TXN001|100001|Deposit|100.00|Fri Aug 29 12:00:00 2026|0
```

Transfer operations create two records: `Transfer Sent` for the sender and `Transfer Received` for the receiver. A related account number is recorded for transfers and `0` for deposits and withdrawals.

Malformed records are ignored during loading instead of terminating the application.
