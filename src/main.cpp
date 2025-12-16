#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include "login.h"
#include "register.h"
#include "dashboard.h"
using namespace std;

void showHomePage();

int main() {
    int choice;
    do {
        showHomePage();
        cout << "Enter your choice (1-3): ";
        cin >> choice;

        switch (choice) {
            case 1: {
                // loginPage now returns username
                string user = loginPage();
                if (!user.empty()) {
                    // go to dashboard after successful login
                    dashboardPage(user);
                }
                break;
            }
            case 2:
                registerPage();
                break;
            case 3:
                cout << "\nExiting the system... Thank you for using Smart Personal Finance Manager.\n";
                this_thread::sleep_for(std::chrono::seconds(2));
                break;
            default:
                cout << "\nInvalid choice. Please try again.\n";
                std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        if (choice != 3) {
            cout << "\nReturning to homepage...\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif

    } while (choice != 3);

    return 0;
}

void showHomePage() {
    cout << "=============================================\n";
    cout << "     SMART PERSONAL FINANCE MANAGER\n";
    cout << "=============================================\n";
    cout << "1. Login to your account\n";
    cout << "2. Create a new account\n";
    cout << "3. Exit the system\n";
    cout << "---------------------------------------------\n";
}