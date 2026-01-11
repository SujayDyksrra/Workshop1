#include <iostream>
#include <mysql/mysql.h>
#include <thread>
#include <chrono>
#include <limits>
#include <regex>
#include "userProfile.h"
#include "utils.h"

using namespace std;

/* ===============================
   ANSI COLOR DEFINITIONS (UI ONLY)
================================ */
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

// Helper to check password strength
static string passwordStrength(const string &pw) {
    int len = pw.size();
    bool lower=false, upper=false, digit=false, special=false;

    for (char c : pw) {
        if (islower(c)) lower = true;
        else if (isupper(c)) upper = true;
        else if (isdigit(c)) digit = true;
        else special = true;
    }

    int classes = lower + upper + digit + special;

    if (len >= 10 && classes >= 3) return "Strong";
    if (len >= 8 && classes >= 2) return "Medium";
    return "Weak";
}

// Mask password
static string maskedPassword() {
    return "********";
}

// MAIN FUNCTION
bool userProfile(const string &username) {

    string currentUsername = username;

    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "127.0.0.1", "root", "", "finance_db", 3306, NULL, 0)) {
        cout << RED << "❌ Database connection failed: "
             << mysql_error(conn) << RESET << endl;
        return true;
    }

    // Fetch profile
    string q = "SELECT username, monthly_budget, saving_goal FROM users WHERE username='" + currentUsername + "'";
    if (mysql_query(conn, q.c_str())) {
        cout << RED << "❌ Query failed: " << mysql_error(conn) << RESET << endl;
        mysql_close(conn);
        return true;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res || mysql_num_rows(res) == 0) {
        cout << RED << "❌ User not found.\n" << RESET;
        if (res) mysql_free_result(res);
        mysql_close(conn);
        return true;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    string uname   = row[0] ? row[0] : "";
    string monthly = row[1] ? row[1] : "0.00";
    string saving  = row[2] ? row[2] : "0.00";
    mysql_free_result(res);

    /* ===============================
       PROFILE DISPLAY
    ================================ */
    cout << BLUE << BOLD;
    cout << "\n====================================================\n";
    cout << "                USER PROFILE (SPFM)\n";
    cout << "====================================================\n";
    cout << RESET;

    cout << CYAN << "Username        : " << RESET << uname << "\n";
    cout << CYAN << "Password        : " << RESET << maskedPassword() << " (hidden)\n";
    cout << CYAN << "Monthly Budget  : " << RESET << "RM " << monthly << "\n";
    cout << CYAN << "Saving Goal     : " << RESET << "RM " << saving << "\n";

    cout << BLUE << "----------------------------------------------------\n" << RESET;

    int choice;

    while (true) {
        cout << "\n" << BOLD << "Choose an option:\n" << RESET;
        cout << "1. Update Profile\n";
        cout << "2. Delete Profile\n";
        cout << "3. Back to Dashboard\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << RED << "Invalid input.\n" << RESET;
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        /* ===============================
           UPDATE PROFILE
        ================================ */
        if (choice == 1) {
            bool done = false;

            while (!done) {
                cout << BLUE << "\n----------- UPDATE PROFILE -----------\n" << RESET;
                cout << "1. Update Username\n";
                cout << "2. Update Password\n";
                cout << "3. Update Monthly Budget\n";
                cout << "4. Update Saving Goal\n";
                cout << "5. Return\n";
                cout << "Choose: ";

                int opt;
                if (!(cin >> opt)) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    continue;
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                // Update Username
                if (opt == 1) {
                    cout << "New username (0 to cancel): ";
                    string newU;
                    cin >> newU;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (newU == "0") continue;
                    if (newU.empty()) {
                        cout << RED << "❌ Invalid username.\n" << RESET;
                        continue;
                    }

                    string checkQ = "SELECT id FROM users WHERE username='" + newU + "'";
                    mysql_query(conn, checkQ.c_str());
                    MYSQL_RES *r = mysql_store_result(conn);

                    if (mysql_num_rows(r) > 0) {
                        cout << RED << "❌ Username already exists.\n" << RESET;
                        mysql_free_result(r);
                        continue;
                    }
                    mysql_free_result(r);

                    string upd = "UPDATE users SET username='" + newU + "' WHERE username='" + currentUsername + "'";
                    if (!mysql_query(conn, upd.c_str())) {
                        cout << GREEN << "✅ Username updated successfully.\n" << RESET;
                        currentUsername = newU;
                        uname = newU;
                    } else {
                        cout << RED << "❌ Update failed.\n" << RESET;
                    }
                }

                // Update Password
                else if (opt == 2) {
                    cout << "New password (0 to cancel): ";
                    string pwd = getPasswordWithToggle();
                    if (pwd == "0") continue;

                    if (passwordStrength(pwd) == "Weak") {
                        cout << RED << "❌ Weak password.\n" << RESET;
                        continue;
                    }

                    cout << "Confirm password: ";
                    string cfm = getPasswordWithToggle();
                    if (cfm != pwd) {
                        cout << RED << "❌ Passwords do not match.\n" << RESET;
                        continue;
                    }

                    string upd = "UPDATE users SET password='" + sha256(pwd) +
                                 "' WHERE username='" + currentUsername + "'";
                    if (!mysql_query(conn, upd.c_str()))
                        cout << GREEN << "✅ Password updated.\n" << RESET;
                    else
                        cout << RED << "❌ Update failed.\n" << RESET;
                }

                // Update Budget
                else if (opt == 3) {
                    cout << "New monthly budget (0 to cancel): ";
                    string newB;
                    cin >> newB;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (newB == "0") continue;
                    if (!isNumber(newB) || stod(newB) <= 0) {
                        cout << RED << "❌ Invalid amount.\n" << RESET;
                        continue;
                    }

                    string upd = "UPDATE users SET monthly_budget=" + newB +
                                 " WHERE username='" + currentUsername + "'";
                    if (!mysql_query(conn, upd.c_str())) {
                        cout << GREEN << "✅ Budget updated.\n" << RESET;
                        monthly = newB;
                    }
                }

                // Update Saving Goal
                else if (opt == 4) {
                    cout << "New saving goal (0 to cancel): ";
                    string newG;
                    cin >> newG;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (newG == "0") continue;
                    if (!isNumber(newG) || stod(newG) <= 0) {
                        cout << RED << "❌ Invalid amount.\n" << RESET;
                        continue;
                    }

                    string upd = "UPDATE users SET saving_goal=" + newG +
                                 " WHERE username='" + currentUsername + "'";
                    if (!mysql_query(conn, upd.c_str())) {
                        cout << GREEN << "✅ Saving goal updated.\n" << RESET;
                        saving = newG;
                    }
                }

                else if (opt == 5) {
                    cout << YELLOW << "Returning to profile...\n" << RESET;
                    done = true;
                }
            }

            cout << BLUE << "\n-------- UPDATED PROFILE --------\n" << RESET;
            cout << "Username        : " << currentUsername << "\n";
            cout << "Monthly Budget  : RM " << monthly << "\n";
            cout << "Saving Goal     : RM " << saving << "\n";
            cout << BLUE << "--------------------------------\n" << RESET;
        }

        /* ===============================
           DELETE PROFILE
        ================================ */
        else if (choice == 2) {
            cout << RED << "\n⚠️ WARNING: This action is permanent!\n" << RESET;
            cout << "Type DELETE to confirm, or 0 to cancel: ";

            string confirm;
            cin >> confirm;

            if (confirm == "0") {
                cout << YELLOW << "Deletion cancelled.\n" << RESET;
                continue;
            }

            if (confirm == "DELETE") {
                string delQ = "DELETE FROM users WHERE username='" + currentUsername + "'";
                if (!mysql_query(conn, delQ.c_str())) {
                    cout << GREEN << "✅ Account deleted successfully.\n" << RESET;
                    mysql_close(conn);
                    return false;
                } else {
                    cout << RED << "❌ Delete failed.\n" << RESET;
                }
            } else {
                cout << RED << "❌ Incorrect confirmation.\n" << RESET;
            }
        }

        /* ===============================
           BACK
        ================================ */
        else if (choice == 3) {
            cout << YELLOW << "Returning to dashboard...\n" << RESET;
            this_thread::sleep_for(chrono::seconds(1));
            break;
        }
        else {
            cout << RED << "Invalid option.\n" << RESET;
        }
    }

    mysql_close(conn);
    return true;
}