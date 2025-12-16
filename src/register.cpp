#include <iostream>
#include <mysql/mysql.h>
#include <thread>
#include <chrono>
#include <regex>
#include <limits>
#include "utils.h"  // getPasswordWithToggle(), sha256()
using namespace std;

// Validate numeric input
// bool isNumber(const string &str) {
//     return regex_match(str, regex("^[0-9]+(\\.[0-9]{1,2})?$"));
// }

// Password strength: returns "Weak", "Medium", or "Strong"
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

    cout << "\n========== CREATE ACCOUNT ==========\n";
    cout << "Type 0 at ANY time (username/password/budget/goal) to cancel and return to main menu.\n\n";

    // 1. Connect to DB
    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "127.0.0.1", "root", "", "finance_db", 3306, NULL, 0)) {
        cout << "❌ Database connection failed: " << mysql_error(conn) << endl;
        return;
    }

    // 2. Username input + immediate check
    bool validUsername = false;
    while (!validUsername) {
        cout << "Enter username: ";
        cin >> username;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (username == "0") {
            cout << "\n↩️ Returning to main menu...\n";
            mysql_close(conn);
            this_thread::sleep_for(chrono::seconds(1));
            return;
        }
        if (username.empty()) {
            cout << "❌ Username cannot be empty.\n";
            continue;
        }

        string checkQuery = "SELECT id FROM users WHERE username='" + username + "'";
        if (mysql_query(conn, checkQuery.c_str())) {
            cout << "❌ Query failed: " << mysql_error(conn) << endl;
            mysql_close(conn);
            return;
        }

        MYSQL_RES *res = mysql_store_result(conn);
        int count = mysql_num_rows(res);
        mysql_free_result(res);

        if (count > 0) {
            cout << "❌ Username already exists. Try a different one.\n";
        } else {
            validUsername = true;
        }
    }

    // 3. Password input (hidden) + strength + confirm
    const string MIN_OK = "Medium"; // enforce at least Medium; change to "Weak" to relax
    while (true) {
        cout << "Enter password (press TAB to toggle show/hide): ";
        password = getPasswordWithStrength(); // use utils → prints stars per char

        if (password == "0") { // allow cancel
            cout << "\n↩️ Returning to main menu...\n";
            mysql_close(conn);
            this_thread::sleep_for(chrono::seconds(1));
            return;
        }
        if (password.empty()) {
            cout << "❌ Password cannot be empty.\n";
            continue;
        }

        // show strength
        string strength = passwordStrength(password);
        cout << "Password strength: " << strength << "\n";

        if (strength == "Weak") {
    cout << "\n❌ Password too weak! Please use a stronger password.\n";
    cout << "Hints: At least 8 characters, mix upper/lowercase, numbers, symbols.\n\n";
    continue;
}

        // confirm password
        cout << "Confirm password (press TAB to toggle show/hide): ";
        confirmPwd = getPasswordWithStrength();
        if (confirmPwd == "0") {
            cout << "\n↩️ Returning to main menu...\n";
            mysql_close(conn);
            this_thread::sleep_for(chrono::seconds(1));
            return;
        }

        if (password != confirmPwd) {
            cout << "❌ Passwords do not match. Please try again.\n";
            continue; // loop to re-enter password
        }

        // password OK
        break;
    }

    // 4. Monthly budget
    do {
        cout << "Enter monthly budget (RM): ";
        cin >> budgetStr;
        if (budgetStr == "0") {
            cout << "\n↩️ Returning to main menu...\n";
            mysql_close(conn);
            this_thread::sleep_for(chrono::seconds(1));
            return;
        }
        if (!isNumber(budgetStr) || stod(budgetStr) <= 0) {
            cout << "❌ Invalid amount. Enter a positive number.\n";
            budgetStr = "";
        }
    } while (budgetStr.empty());

    // 5. Saving goal
    do {
        cout << "Enter saving goal (RM): ";
        cin >> goalStr;
        if (goalStr == "0") {
            cout << "\n↩️ Returning to main menu...\n";
            mysql_close(conn);
            this_thread::sleep_for(chrono::seconds(1));
            return;
        }
        if (!isNumber(goalStr) || stod(goalStr) <= 0) {
            cout << "❌ Invalid amount. Enter a positive number.\n";
            goalStr = "";
        }
    } while (goalStr.empty());

    // 6. Insert into DB (store hashed password)
    string insertQuery =
        "INSERT INTO users (username, password, monthly_budget, saving_goal) VALUES ('" +
        username + "', '" + sha256(password) + "', " + budgetStr + ", " + goalStr + ")";

    if (!mysql_query(conn, insertQuery.c_str())) {
        cout << "\n🎉 Account created successfully! You may now log in.\n";
    } else {
        cout << "❌ Failed to create account: " << mysql_error(conn) << endl;
    }

    mysql_close(conn);

    // 7. Return to menu
    cout << "\nPress ENTER to return to the main menu...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();

    cout << "Returning...\n";
    this_thread::sleep_for(chrono::seconds(1));
}