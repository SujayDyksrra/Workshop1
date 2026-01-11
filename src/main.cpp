#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <limits>
#include "login.h"
#include "register.h"
#include "dashboard.h"

using namespace std;

/* ===============================
   ANSI COLORS (UI ONLY)
================================ */
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

/* ===============================
   FUNCTION DECLARATIONS
================================ */
void showHomePage();

int main() {
    int choice;

    do {
        showHomePage();

        cout << CYAN << "Enter your choice (1–3): " << RESET;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << RED << "\n❌ Invalid input. Please enter a number.\n" << RESET;
            this_thread::sleep_for(chrono::seconds(2));
            continue;
        }

        switch (choice) {

            case 1: {
                string user = loginPage();
                if (!user.empty()) {
                    dashboardPage(user);
                }
                break;
            }

            case 2:
                registerPage();
                break;

            case 3:
                cout << GREEN
                     << "\nThank you for using Smart Personal Finance Manager.\n"
                     << "Goodbye! 👋\n" << RESET;
                this_thread::sleep_for(chrono::seconds(2));
                break;

            default:
                cout << RED << "\n❌ Invalid choice. Please try again.\n" << RESET;
                this_thread::sleep_for(chrono::seconds(2));
        }

        if (choice != 3) {
            cout << YELLOW << "\nReturning to homepage...\n" << RESET;
            this_thread::sleep_for(chrono::seconds(1));
        }

#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif

    } while (choice != 3);

    return 0;
}

/* ===============================
   HOME PAGE UI
================================ */
void showHomePage() {

    cout << BLUE << BOLD;
    cout << "============================================================\n";
    cout << "        ███████╗██████╗ ███████╗███╗   ███╗\n";
    cout << "        ██╔════╝██╔══██╗██╔════╝████╗ ████║\n";
    cout << "        ███████╗██████╔╝█████╗  ██╔████╔██║\n";
    cout << "        ╚════██║██╔═══╝ ██╔══╝  ██║╚██╔╝██║\n";
    cout << "        ███████║██║     ██║     ██║ ╚═╝ ██║\n";
    cout << "        ╚══════╝╚═╝     ╚═╝     ╚═╝     ╚═╝\n";
    cout << "============================================================\n";
    cout << RESET;

    cout << CYAN << BOLD
         << "        SMART PERSONAL FINANCE MANAGER (SPFM)\n"
         << RESET;

    cout << BLUE << "------------------------------------------------------------\n" << RESET;

    cout << YELLOW;
    cout << "  1. Login to your account\n";
    cout << "  2. Create a new account\n";
    cout << "  3. Exit the system\n";
    cout << RESET;

    cout << BLUE << "------------------------------------------------------------\n" << RESET;
}