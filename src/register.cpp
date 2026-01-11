#include <iostream>
#include <mysql/mysql.h>
#include <thread>
#include <chrono>
#include <regex>
#include <limits>
#include "utils.h"

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

// Password strength checker (UNCHANGED)
string passwordStrength(const string &pw) {
    int len = (int)pw.size();
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    for (char c : pw) {
        if (islower((unsigned char)c)) hasLower = true;
        else if (isupper((unsigned char)c)) hasUpper = true;
        else if (isdigit((unsigned char)c)) hasDigit = true;
        else hasSpecial = true;
    }
    int classes = (int)hasLower + (int)hasUpper + (int)hasDigit + (int)hasSpecial;

    if (len >= 10 && classes >= 3) return "Strong";
    if (len >= 8 && classes >= 2) return "Medium";
    return "Weak";
}

void registerPage() {

    string username, password, confirmPwd, budgetStr, goalStr;

    cout << BLUE << BOLD;
    cout << "\n============================================================\n";
    cout << "               SPFM CREATE ACCOUNT\n";
    cout << "============================================================\n";
    cout << RESET;

    cout << YELLOW
         << "Type 0 at ANY time to cancel and return to main menu.\n\n"
         << RESET;

    // 1. Connect to DB
    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "127.0.0.1", "root", "", "finance_db", 3306, NULL, 0)) {
        cout << RED << "❌ Database connection failed: "
             << mysql_error(conn) << RESET << endl;
        return;
    }

    /* ===============================
       USERNAME
    ================================ */
    bool validUsername = false;
    while (!validUsername) {
        cout << CYAN << "Username: " << RESET;
        cin >> username;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (username == "0") {
            cout << YELLOW << "\n↩️ Returning to main menu...\n" << RESET;
            mysql_close(conn);
            this_thread::sleep_for(chrono::seconds(1));
            return;
        }

        if (username.empty()) {
            cout << RED << "❌ Username cannot be empty.\n" << RESET;
            continue;
        }

        string checkQuery = "SELECT id FROM users WHERE username='" + username + "'";
        mysql_query(conn, checkQuery.c_str());
        MYSQL_RES *res = mysql_store_result(conn);

        if (mysql_num_rows(res) > 0) {
            cout << RED << "❌ Username already exists. Try another.\n" << RESET;
        } else {
            validUsername = true;
        }
        mysql_free_result(res);
    }

    /* ===============================
       PASSWORD
    ================================ */
    while (true) {
        cout << CYAN << "Password (TAB to toggle): " << RESET;
        password = getPasswordWithStrength();

        if (password == "0") {
            cout << YELLOW << "\n↩️ Returning to main menu...\n" << RESET;
            mysql_close(conn);
            this_thread::sleep_for(chrono::seconds(1));
            return;
        }

        string strength = passwordStrength(password);
        cout << "Password strength: ";

        if (strength == "Strong")
            cout << GREEN << strength << RESET << "\n";
        else if (strength == "Medium")
            cout << YELLOW << strength << RESET << "\n";
        else {
            cout << RED << strength << RESET << "\n";
            cout << RED << "❌ Password too weak.\n" << RESET;
            cout << YELLOW
                 << "Hint: At least 8 chars, mix upper/lowercase, numbers, symbols.\n\n"
                 << RESET;
            continue;
        }

        cout << CYAN << "Confirm Password: " << RESET;
        confirmPwd = getPasswordWithStrength();

        if (confirmPwd == "0") {
            cout << YELLOW << "\n↩️ Returning to main menu...\n" << RESET;
            mysql_close(conn);
            this_thread::sleep_for(chrono::seconds(1));
            return;
        }

        if (password != confirmPwd) {
            cout << RED << "❌ Passwords do not match.\n" << RESET;
            continue;
        }
        break;
    }

    /* ===============================
       MONTHLY BUDGET
    ================================ */
    do {
        cout << CYAN << "Monthly Budget (RM): " << RESET;
        cin >> budgetStr;

        if (budgetStr == "0") {
            cout << YELLOW << "\n↩️ Returning to main menu...\n" << RESET;
            mysql_close(conn);
            this_thread::sleep_for(chrono::seconds(1));
            return;
        }

        if (!isNumber(budgetStr) || stod(budgetStr) <= 0) {
            cout << RED << "❌ Invalid amount.\n" << RESET;
            budgetStr = "";
        }
    } while (budgetStr.empty());

    /* ===============================
       SAVING GOAL
    ================================ */
    do {
        cout << CYAN << "Saving Goal (RM): " << RESET;
        cin >> goalStr;

        if (goalStr == "0") {
            cout << YELLOW << "\n↩️ Returning to main menu...\n" << RESET;
            mysql_close(conn);
            this_thread::sleep_for(chrono::seconds(1));
            return;
        }

        if (!isNumber(goalStr) || stod(goalStr) <= 0) {
            cout << RED << "❌ Invalid amount.\n" << RESET;
            goalStr = "";
        }
    } while (goalStr.empty());

    /* ===============================
       INSERT USER
    ================================ */
    string insertQuery =
        "INSERT INTO users (username, password, monthly_budget, saving_goal) VALUES ('" +
        username + "', '" + sha256(password) + "', " + budgetStr + ", " + goalStr + ")";

    if (!mysql_query(conn, insertQuery.c_str())) {
        cout << GREEN << BOLD
             << "\n🎉 Account created successfully!\n"
             << "You may now log in.\n"
             << RESET;
    } else {
        cout << RED << "❌ Failed to create account: "
             << mysql_error(conn) << RESET << endl;
    }

    mysql_close(conn);

    cout << YELLOW << "\nPress ENTER to return to main menu..." << RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();

    cout << "Returning...\n";
    this_thread::sleep_for(chrono::seconds(1));
}