#include <iostream>
#include <mysql/mysql.h>
#include <thread>
#include <chrono>
#include <limits>
#include <regex>
#include "userProfile.h"
#include "utils.h"

using namespace std;

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

// Helper numeric validation
// static bool isNumber(const string &str) {
//     return regex_match(str, regex("^[0-9]+(\\.[0-9]{1,2})?$"));
// }

// Mask password (always show ********)
static string maskedPassword() {
    return "********";
}

// MAIN FUNCTION — user profile module
bool userProfile(const string &username) {

    // Internal username copy (so we can update username safely locally)
    string currentUsername = username;

    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "127.0.0.1", "root", "", "finance_db", 3306, NULL, 0)) {
        cout << "❌ Database connection failed: " << mysql_error(conn) << endl;
        return true;
    }

    // Fetch profile data
    string q = "SELECT username, monthly_budget, saving_goal FROM users WHERE username='" + currentUsername + "'";
    if (mysql_query(conn, q.c_str())) {
        cout << "❌ Query failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return true;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res || mysql_num_rows(res) == 0) {
        cout << "❌ User not found.\n";
        if (res) mysql_free_result(res);
        mysql_close(conn);
        return true;
    }

    MYSQL_ROW row = mysql_fetch_row(res);

    string uname   = row[0] ? row[0] : "";
    string monthly = row[1] ? row[1] : "0.00";
    string saving  = row[2] ? row[2] : "0.00";

    mysql_free_result(res);

    // Display profile
    cout << "\n========== USER PROFILE ==========\n";
    cout << "Username      : " << uname << "\n";
    cout << "Password      : " << maskedPassword() << " (hidden)\n";
    cout << "Monthly Budget: RM " << monthly << "\n";
    cout << "Saving Goal   : RM " << saving << "\n";
    cout << "==================================\n";

    int choice;

    while (true) {
        cout << "\nWhat do you want to do?\n";
        cout << "1. Update Profile\n";
        cout << "2. Delete Profile\n";
        cout << "3. Back to Dashboard\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n";
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        // ===========================================
        // 1 — UPDATE PROFILE
        // ===========================================
        if (choice == 1) {

            bool done = false;

            while (!done) {
                cout << "\n--- UPDATE PROFILE ---\n";
                cout << "1. Update Username\n";
                cout << "2. Update Password\n";
                cout << "3. Update Monthly Budget\n";
                cout << "4. Update Saving Goal\n";
                cout << "5. Finish and return to Profile\n";
                // cout << "6. Cancel updates\n";
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
                    cout << "Enter new username (0 to cancel): ";
                    string newU;
                    cin >> newU;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (newU == "0") continue;
                    if (newU.empty()) {
                        cout << "❌ Invalid username.\n";
                        continue;
                    }

                    // Check uniqueness
                    string checkQ = "SELECT id FROM users WHERE username='" + newU + "'";
                    mysql_query(conn, checkQ.c_str());
                    MYSQL_RES *r = mysql_store_result(conn);
                    int count = mysql_num_rows(r);
                    mysql_free_result(r);

                    if (count > 0) {
                        cout << "❌ Username already exists.\n";
                        continue;
                    }

                    string upd = "UPDATE users SET username='" + newU + "' WHERE username='" + currentUsername + "'";
                    if (!mysql_query(conn, upd.c_str())) {
                        cout << "✅ Username updated.\n";
                        currentUsername = newU;
                        uname = newU;
                    } else {
                        cout << "❌ Update failed: " << mysql_error(conn) << endl;
                    }
                }

                // Update Password
                else if (opt == 2) {
                    cout << "Enter new password (0 to cancel): ";
                    string pwd = getPasswordWithToggle();
                    if (pwd == "0") continue;

                    string strength = passwordStrength(pwd);
                    if (strength == "Weak") {
                        cout << "❌ Weak password. Try again.\n";
                        continue;
                    }

                    cout << "Confirm password: ";
                    string cfm = getPasswordWithToggle();
                    if (cfm != pwd) {
                        cout << "❌ Passwords do not match.\n";
                        continue;
                    }

                    string hashed = sha256(pwd);
                    string upd = "UPDATE users SET password='" + hashed + "' WHERE username='" + currentUsername + "'";
                    if (!mysql_query(conn, upd.c_str())) {
                        cout << "✅ Password updated.\n";
                    } else {
                        cout << "❌ Update failed.\n";
                    }
                }

                // Update Monthly Budget
                else if (opt == 3) {
                    cout << "Enter new monthly budget (0 to cancel): ";
                    string newB;
                    cin >> newB;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (newB == "0") continue;
                    if (!isNumber(newB) || stod(newB) <= 0) {
                        cout << "❌ Invalid amount.\n";
                        continue;
                    }

                    string upd = "UPDATE users SET monthly_budget=" + newB + " WHERE username='" + currentUsername + "'";
                    if (!mysql_query(conn, upd.c_str())) {
                        cout << "✅ Budget updated.\n";
                        monthly = newB;
                    } else {
                        cout << "❌ Update failed.\n";
                    }
                }

                // Update Saving Goal
                else if (opt == 4) {
                    cout << "Enter new saving goal (0 to cancel): ";
                    string newG;
                    cin >> newG;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (newG == "0") continue;
                    if (!isNumber(newG) || stod(newG) <= 0) {
                        cout << "❌ Invalid amount.\n";
                        continue;
                    }

                    string upd = "UPDATE users SET saving_goal=" + newG + " WHERE username='" + currentUsername + "'";
                    if (!mysql_query(conn, upd.c_str())) {
                        cout << "✅ Saving goal updated.\n";
                        saving = newG;
                    } else {
                        cout << "❌ Update failed.\n";
                    }
                }

                else if (opt == 5) {
                    cout << "Returning to profile…\n";
                    done = true;
                }

                // else if (opt == 6) {
                //     cout << "Cancelling all updates.\n";
                //     done = true;
                // }
            }

            // Show updated profile
            cout << "\n========== UPDATED PROFILE ==========\n";
            cout << "Username      : " << currentUsername << "\n";
            cout << "Password      : ********\n";
            cout << "Monthly Budget: RM " << monthly << "\n";
            cout << "Saving Goal   : RM " << saving << "\n";
            cout << "=====================================\n";
        }

        // ===========================================
        // 2 — DELETE PROFILE
        // ===========================================
        else if (choice == 2) {
            cout << "\n⚠️  This will permanently delete your account.\n";
            cout << "Type DELETE to confirm, or 0 to cancel: ";

            string confirm;
            cin >> confirm;

            if (confirm == "0") {
                cout << "Deletion cancelled.\n";
                continue;
            }

            if (confirm == "DELETE") {
                string delQ = "DELETE FROM users WHERE username='" + currentUsername + "'";
                if (!mysql_query(conn, delQ.c_str())) {
                    cout << "✅ Account deleted.\n";
                    mysql_close(conn);
                    return false;   // signal to dashboard: logout now
                } else {
                    cout << "❌ Delete failed: " << mysql_error(conn) << endl;
                }
            } else {
                cout << "Incorrect confirmation.\n";
            }
        }

        // ===========================================
        // 3 — BACK TO DASHBOARD
        // ===========================================
        else if (choice == 3) {
            cout << "Returning to Dashboard...\n";
            this_thread::sleep_for(chrono::seconds(1));
            break;
        }

        else {
            cout << "Invalid option.\n";
        }
    }

    mysql_close(conn);
    return true;
}