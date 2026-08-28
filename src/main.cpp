#include <iostream>
#include <limits>

void showMenu() {
    std::cout << "\n===== SecureBank Banking Management System =====\n"
              << "1. Create account\n"
              << "2. View all accounts\n"
              << "3. Search account\n"
              << "4. Update account\n"
              << "5. Close account\n"
              << "6. Deposit money\n"
              << "7. Withdraw money\n"
              << "8. Transfer money\n"
              << "9. Transaction history\n"
              << "0. Exit\n"
              << "Enter your choice: ";
}

int main() {
    int choice = -1;

    while (choice != 0) {
        showMenu();

        if (!(std::cin >> choice)) {
            std::cout << "Invalid input. Please enter a number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (choice == 0) {
            std::cout << "Thank you for using SecureBank.\n";
        } else if (choice >= 1 && choice <= 9) {
            std::cout << "This feature will be implemented in a future milestone.\n";
        } else {
            std::cout << "Invalid choice. Please select an available option.\n";
        }
    }

    return 0;
}
