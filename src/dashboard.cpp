#include <iostream>
#include <mysql/mysql.h>
#include <thread>
#include <chrono>
#include <limits>
#include <sstream>
#include "dashboard.h"
#include "userProfile.h"
#include "income.h"
#include "expense.h"
#include "report.h"

using namespace std;

/* ===============================
   ANSI COLOR CODES (UI ONLY)
================================ */
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

/* ===============================
   DASHBOARD PAGE
================================ */
void dashboardPage(const string &username) {
    if (username.empty()) return;

    int choice = 0;

    do {
        cout << BLUE << "\n============================================\n" << RESET;
        cout << CYAN << BOLD << "            PERSONAL FINANCE DASHBOARD\n" << RESET;
        cout << BLUE << "============================================\n" << RESET;

        cout << GREEN << "Logged in as: " << RESET << username << "\n";
        cout << BLUE << "--------------------------------------------\n" << RESET;

        cout << YELLOW;
        cout << " 1. User Profile\n";
        cout << " 2. Income Management\n";
        cout << " 3. Expense Management\n";
        cout << " 4. Report & Smart Insights\n";
        cout << " 5. Logout\n";
        cout << RESET;

        cout << BLUE << "--------------------------------------------\n" << RESET;
        cout << "Enter your choice (1–5): ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << RED << "❌ Invalid input. Please enter a number.\n" << RESET;
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {

            case 1: {
                bool exists = userProfile(username);
                if (!exists) {
                    cout << RED << "\n⚠️  Account deleted. Logging out...\n" << RESET;
                    this_thread::sleep_for(chrono::seconds(1));
                    return; // back to main menu
                }
                break;
            }

            case 2:
                incomeManagement(username);
                break;

            case 3:
                expenseManagement(username);
                break;

            case 4:
                reportManagement(username);
                break;

            case 5:
                cout << RED << "\nLogging out...\n" << RESET;
                this_thread::sleep_for(chrono::seconds(1));
                break;

            default:
                cout << RED << "❌ Invalid choice. Please try again.\n" << RESET;
        }

    } while (choice != 5);
}