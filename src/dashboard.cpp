#include <iostream>
#include <mysql/mysql.h>
#include <thread>
#include <chrono>
#include <limits>
#include <sstream>
#include "dashboard.h"
#include "userProfile.h"
#include "income.h"
#include "expense.h"   // ✅ NEW — Add this header to enable expense module

using namespace std;


// dashboardPage: shows menu and routes to userProfile or placeholders
void dashboardPage(const string &username) {
    if (username.empty()) return;

    int choice = 0;
    do {
        cout << "\n=======================================\n";
        cout << "            USER DASHBOARD\n";
        cout << "Logged in as: " << username << "\n";
        cout << "=======================================\n";
        cout << "1. User Profile\n";
        cout << "2. Income Management\n";
        cout << "3. Expense Management\n";
        cout << "4. Report & Insights (coming soon)\n";
        cout << "5. Back to Login/Main Menu\n";
        cout << "---------------------------------------\n";
        cout << "Enter your choice (1-5): ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Try again.\n";
            continue;
        }

        switch (choice) {
            case 1:
{
    bool exists = userProfile(username);
    if (!exists) {
        // user deleted their account — return to main menu
        cout << "You have been logged out as your account was deleted.\n";
        this_thread::sleep_for(std::chrono::seconds(1));
        return;  // exit dashboard and go back to main menu
    }
    break;
}
            case 2:
    incomeManagement(username);
    break;
             case 3:
                expenseManagement(username);     // ✅ Redirect to Expense Module
                break;
            case 4:
                cout << "\n[Report & Insights] — feature coming soon.\n";
                break;
            case 5:
                cout << "\nLogging out and returning to main menu...\n";
                this_thread::sleep_for(chrono::seconds(1));
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 5);
}