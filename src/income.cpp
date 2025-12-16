#include <iostream>
#include <mysql/mysql.h>
#include <iomanip>
#include <regex>
#include <chrono>
#include <thread>
#include "income.h"
#include "utils.h"
#include <ctime>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <limits>

using namespace std;

// helper: numeric validation is provided by utils: bool isNumber(const string &str)

// small string helpers
string toLower(const string &s) {
    string out = s;
    transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\n");
    size_t end = s.find_last_not_of(" \t\n");
    if (start == string::npos) return "";
    return s.substr(start, end - start + 1);
}

string formatDate(int y, int m, int d) {
    char buf[11];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", y, m, d);
    return string(buf);
}

string getToday() {
    time_t now = time(0);
    tm *t = localtime(&now);
    return formatDate(1900 + t->tm_year, 1 + t->tm_mon, t->tm_mday);
}

// natural and numeric parsers reused from your existing code
string parseNaturalDate(const string &inputRaw) {
    string in = toLower(trim(inputRaw));

    time_t now = time(0);
    tm t = *localtime(&now);

    if (in == "today" || in == "td" || in == "t") return getToday();
    if (in == "yesterday" || in == "yd") { t.tm_mday -= 1; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday); }
    if (in == "tomorrow" || in == "tm") { t.tm_mday += 1; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday); }
    if (in.size() > 1 && in[0] == '+') { int n = stoi(in.substr(1)); t.tm_mday += n; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday); }
    if (in.size() > 1 && in[0] == '-') { int n = stoi(in.substr(1)); t.tm_mday -= n; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday); }
    if (in == "next week") { t.tm_mday += 7; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday); }
    if (in == "last week") { t.tm_mday -= 7; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday); }
    if (in == "next month") { t.tm_mon += 1; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, min(t.tm_mday, 28)); }
    if (in == "last month") { t.tm_mon -= 1; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, min(t.tm_mday, 28)); }

    return "";
}

string parseNumericDate(const string &inputRaw) {
    string in = trim(inputRaw);

    if (regex_match(in, regex("^[0-9]{6}$"))) {
        int d = stoi(in.substr(0, 2));
        int m = stoi(in.substr(2, 2));
        int y = 2000 + stoi(in.substr(4, 2));
        return formatDate(y, m, d);
    }
    if (regex_match(in, regex("^[0-9]{1,2}/[0-9]{1,2}/[0-9]{4}$"))) {
        int d, m, y; char s1, s2;
        stringstream ss(in); ss >> d >> s1 >> m >> s2 >> y;
        return formatDate(y, m, d);
    }
    if (regex_match(in, regex("^[0-9]{1,2}-[0-9]{1,2}-[0-9]{4}$"))) {
        int d, m, y; char s1, s2;
        stringstream ss(in); ss >> d >> s1 >> m >> s2 >> y;
        return formatDate(y, m, d);
    }
    if (regex_match(in, regex("^[0-9]{4}-[0-9]{2}-[0-9]{2}$"))) return in;
    return "";
}

// A robust date input that allows:
//  - press ENTER → uses today's date
//  - type 0 → cancel and return empty string
//  - type natural/numeric date (validated) → returns "YYYY-MM-DD"
//  - retry until valid or cancelled
string getDateWithCancel() {
    while (true) {
        cout << "Enter date (press ENTER for today, type 0 to cancel, examples: today, +3, 02/02/2025): ";
        string input;
        getline(cin, input);

        if (input == "0") return "";          // user cancelled
        if (input.empty()) return getToday(); // accept today

        string nat = parseNaturalDate(input);
        if (!nat.empty()) return nat;

        string num = parseNumericDate(input);
        if (!num.empty()) return num;

        cout << "❌ Invalid date format. Try again or type 0 to cancel.\n";
    }
}

// helper: get user_id from username
int getUserId(MYSQL *conn, const string &username) {
    string q = "SELECT id FROM users WHERE username='" + username + "'";
    if (mysql_query(conn, q.c_str())) return -1;
    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) return -1;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) { mysql_free_result(res); return -1; }
    int id = stoi(row[0]);
    mysql_free_result(res);
    return id;
}

void incomeManagement(const string &username) {
    MYSQL *conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, "127.0.0.1", "root", "", "finance_db", 3306, NULL, 0)) {
        cout << "❌ Database connection failed: " << mysql_error(conn) << endl;
        return;
    }

    int userId = getUserId(conn, username);
    if (userId < 0) {
        cout << "❌ Unable to identify user.\n";
        mysql_close(conn);
        return;
    }

    int choice = 0;

    while (true) {
        cout << "\n========== INCOME MANAGEMENT ==========\n";
        cout << "1. Add Income\n";
        cout << "2. View Income Records\n";
        cout << "3. Update Income\n";
        cout << "4. Delete Income\n";
        cout << "5. Back to Dashboard\n";
        cout << "=======================================\n";
        cout << "Enter choice: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Try again.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        // -------------------------------------------------------
        // 1. ADD INCOME
        // -------------------------------------------------------
        if (choice == 1) {
            while (true) {
                string source;
                string amountStr;
                string date;

                cout << "\n--- Add Income (type 0 at any prompt to cancel) ---\n";

                // SOURCE (required)
                cout << "Enter income source: ";
                getline(cin, source);
                if (source == "0") { cout << "Add income cancelled. Returning to Income menu.\n"; break; }
                if (trim(source).empty()) {
                    cout << "❌ Source cannot be empty. Retry or type 0 to cancel.\n";
                    continue; // retry whole add flow
                }

                // AMOUNT (required & validated)
                bool amountOk = false;
                do {
                    cout << "Enter amount (RM): ";
                    if (!getline(cin, amountStr)) amountStr = "";
                    if (amountStr == "0") { cout << "Add income cancelled. Returning to Income menu.\n"; amountOk = false; break; }
                    if (!isNumber(amountStr) || stod(amountStr) <= 0) {
                        cout << "❌ Invalid amount. Enter a positive number (e.g. 1200.50). Type 0 to cancel.\n";
                        amountStr = "";
                        continue;
                    }
                    amountOk = true;
                } while (!amountOk);

                if (!amountOk) break; // user cancelled

                // DATE (required & validated via getDateWithCancel)
                date = getDateWithCancel();
                if (date.empty()) { cout << "Add income cancelled. Returning to Income menu.\n"; break; }

                // All fields valid — insert into DB
                string q = "INSERT INTO income (user_id, source, amount, income_date) VALUES ("
                            + to_string(userId) + ", '" + source + "', " + amountStr + ", '" + date + "')";
                if (!mysql_query(conn, q.c_str())) {
                    cout << "✅ Income added successfully!\n";
                } else {
                    cout << "❌ Failed to add income: " << mysql_error(conn) << endl;
                }

                this_thread::sleep_for(chrono::seconds(1));
                break; // exit add-income loop and return to income menu
            } // end add-income outer loop
        }

        // -------------------------------------------------------
        // 2. VIEW INCOME
        // -------------------------------------------------------
        else if (choice == 2) {
            string q = "SELECT id, source, amount, income_date FROM income WHERE user_id=" + to_string(userId) + " ORDER BY income_date DESC";
            if (mysql_query(conn, q.c_str())) {
                cout << "❌ Query failed: " << mysql_error(conn) << endl;
                continue;
            }
            MYSQL_RES *res = mysql_store_result(conn);
            int rows = mysql_num_rows(res);
            if (rows == 0) {
                cout << "No income records found.\n";
                if (res) mysql_free_result(res);
                continue;
            }
            cout << "\n------ Income Records ------\n";
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                cout << "ID: " << row[0]
                     << " | Source: " << row[1]
                     << " | Amount: RM " << row[2]
                     << " | Date: " << row[3] << "\n";
            }
            cout << "-----------------------------\n";
            mysql_free_result(res);
        }

        // -------------------------------------------------------
        // 3. UPDATE INCOME
        // -------------------------------------------------------
else if (choice == 3) {

    // ==========================================================
    //  AUTO-DISPLAY ALL INCOME RECORDS BEFORE ASKING FOR ID
    // ==========================================================
    {
        string listQ = "SELECT id, source, amount, income_date FROM income WHERE user_id=" 
                        + to_string(userId) + " ORDER BY income_date DESC";

        if (mysql_query(conn, listQ.c_str())) {
            cout << "❌ Failed to load income records.\n";
        } 
        else {
            MYSQL_RES *resList = mysql_store_result(conn);
            int rowsList = mysql_num_rows(resList);

            if (rowsList == 0) {
                cout << "\nNo income records available to update.\n";
                if (resList) mysql_free_result(resList);
                continue;  // Return to income menu because nothing to update
            }

            cout << "\n------ Your Income Records ------\n";
            MYSQL_ROW rr;
            while ((rr = mysql_fetch_row(resList))) {
                cout << "ID: " << rr[0]
                     << " | Source: " << rr[1]
                     << " | Amount: RM " << rr[2]
                     << " | Date: " << rr[3] << "\n";
            }
            cout << "---------------------------------\n";

            mysql_free_result(resList);
        }
    }

    // ==========================================================
    //  NOW ASK FOR THE INCOME ID TO UPDATE
    // ==========================================================
    int id;
    cout << "Enter the Income ID to update (0 to cancel): ";
    if (!(cin >> id)) { 
        cin.clear(); 
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
        cout << "Invalid input.\n"; 
        continue; 
    }
    if (id == 0) { 
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
        continue; 
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    // Load existing record
    string fetchQ = "SELECT source, amount, income_date FROM income WHERE id=" 
                    + to_string(id) + " AND user_id=" + to_string(userId);

    if (mysql_query(conn, fetchQ.c_str())) {
        cout << "❌ Failed to fetch income.\n";
        continue;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res || mysql_num_rows(res) == 0) {
        cout << "❌ No income found with this ID.\n";
        if (res) mysql_free_result(res);
        continue;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    string oldSource = row[0];
    string oldAmount = row[1];
    string oldDate   = row[2];
    mysql_free_result(res);

    cout << "\n--- Current Income Record ---\n";
    cout << "Source : " << oldSource << "\n";
    cout << "Amount : RM " << oldAmount << "\n";
    cout << "Date   : " << oldDate << "\n";
    cout << "--------------------------------\n";

    // ==========================================================
    //  UPDATE MENU LOOP
    // ==========================================================
    string newSource = oldSource;
    string newAmount = oldAmount;
    string newDate   = oldDate;
    bool updating = true;

    while (updating) {
        cout << "\n--- Update Income Menu ---\n";
        cout << "1. Update Source\n";
        cout << "2. Update Amount\n";
        cout << "3. Update Date\n";
        // cout << "4. Save Changes\n";
        cout << "4. Cancel and Return\n";
        cout << "Choose an option: ";

        int upChoice;
        if (!(cin >> upChoice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (upChoice) {

            // ------------------------------------
            // UPDATE SOURCE
            // ------------------------------------
            case 1: {
                cout << "Enter new source (or 0 to keep current): ";
                string tmp;
                getline(cin, tmp);
                if (tmp != "0" && !trim(tmp).empty())
                    newSource = tmp;
                break;
            }

            // ------------------------------------
            // UPDATE AMOUNT
            // ------------------------------------
            case 2: {
                string amountInput;
                while (true) {
                    cout << "Enter new amount (RM) (or 0 to keep current): ";
                    getline(cin, amountInput);

                    if (amountInput == "0") {
                        newAmount = oldAmount;
                        break;
                    }

                    if (!isNumber(amountInput) || stod(amountInput) <= 0) {
                        cout << "❌ Invalid amount. Try again.\n";
                        continue;
                    }
                    newAmount = amountInput;
                    break;
                }
                break;
            }

            // ------------------------------------
            // UPDATE DATE
            // ------------------------------------
            case 3: {
                cout << "Enter new date (natural/numeric, ENTER = today, 0 = keep current): ";
                string tmp;
                getline(cin, tmp);

                if (tmp == "0") {
                    newDate = oldDate;
                }
                else if (tmp.empty()) {
                    newDate = getToday();
                }
                else {
                    string nat = parseNaturalDate(tmp);
                    string num = parseNumericDate(tmp);
                    if (!nat.empty()) newDate = nat;
                    else if (!num.empty()) newDate = num;
                    else {
                        cout << "❌ Invalid date format. Keeping current date.\n";
                        newDate = oldDate;
                    }
                }
                break;
            }

            // ------------------------------------
            // SAVE CHANGES
            // ------------------------------------
            // case 4: {
            //     string q =
            //         "UPDATE income SET source='" + newSource +
            //         "', amount=" + newAmount +
            //         ", income_date='" + newDate +
            //         "' WHERE id=" + to_string(id) +
            //         " AND user_id=" + to_string(userId);

            //     if (!mysql_query(conn, q.c_str()))
            //         cout << "✅ Income updated successfully.\n";
            //     else
            //         cout << "❌ Update failed: " << mysql_error(conn) << endl;

            //     updating = false;
            //     break;
            // }

            // ------------------------------------
            // CANCEL UPDATE
            // ------------------------------------
            case 4:
                cout << "Update cancelled. No changes saved.\n";
                updating = false;
                break;

            default:
                cout << "Invalid option.\n";
        }
    }
}

 // -------------------------------------------------------
// 4. DELETE INCOME (display records, allow multiple deletes)
// -------------------------------------------------------
else if (choice == 4) {
    // 1) show current records so user can choose IDs
    {
        string listQ = "SELECT id, source, amount, income_date FROM income WHERE user_id=" 
                        + to_string(userId) + " ORDER BY income_date DESC";

        if (mysql_query(conn, listQ.c_str())) {
            cout << "❌ Failed to load income records.\n";
            continue;
        }

        MYSQL_RES *resList = mysql_store_result(conn);
        int rowsList = mysql_num_rows(resList);
        if (rowsList == 0) {
            cout << "\nNo income records available to delete.\n";
            if (resList) mysql_free_result(resList);
            continue;
        }

        cout << "\n------ Your Income Records ------\n";
        MYSQL_ROW rr;
        while ((rr = mysql_fetch_row(resList))) {
            cout << "ID: " << rr[0]
                 << " | Source: " << rr[1]
                 << " | Amount: RM " << rr[2]
                 << " | Date: " << rr[3] << "\n";
        }
        cout << "---------------------------------\n";
        mysql_free_result(resList);
    }

    // 2) ask for comma-separated IDs to delete
    cout << "Enter Income ID(s) to delete separated by commas (e.g. 3,5,7) or 0 to cancel: ";
    string idsInput;
    getline(cin, idsInput);
    idsInput = trim(idsInput);
    if (idsInput.empty()) { cout << "No input detected. Returning to Income menu.\n"; continue; }
    if (idsInput == "0") { cout << "Delete cancelled. Returning to Income menu.\n"; continue; }

    // sanitize & parse input into vector<int>
    vector<string> tokens;
    {
        string tmp;
        for (char c : idsInput) {
            if (c == ',') {
                if (!tmp.empty()) { tokens.push_back(trim(tmp)); tmp.clear(); }
            } else tmp.push_back(c);
        }
        if (!tmp.empty()) tokens.push_back(trim(tmp));
    }

    vector<int> ids;
    bool badToken = false;
    for (auto &t : tokens) {
        if (!regex_match(t, regex("^[0-9]+$"))) { badToken = true; break; }
        try {
            ids.push_back(stoi(t));
        } catch (...) {
            badToken = true; break;
        }
    }
    if (badToken || ids.empty()) {
        cout << "❌ Invalid ID list. Please supply comma-separated numeric IDs. Returning to Income menu.\n";
        continue;
    }

    // build CSV for SQL IN clause
    string idsCSV;
    for (size_t i = 0; i < ids.size(); ++i) {
        if (i) idsCSV += ",";
        idsCSV += to_string(ids[i]);
    }

    // 3) confirm these IDs belong to the user and fetch details
    string fetchQ = "SELECT id, source, amount, income_date FROM income WHERE id IN (" + idsCSV + ") AND user_id=" + to_string(userId);
    if (mysql_query(conn, fetchQ.c_str())) {
        cout << "❌ Failed to check income IDs: " << mysql_error(conn) << "\n";
        continue;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res || mysql_num_rows(res) == 0) {
        cout << "❌ None of the provided IDs were found for your account. Returning to Income menu.\n";
        if (res) mysql_free_result(res);
        continue;
    }

    // collect found IDs and display
    vector<int> foundIds;
    cout << "\nYou selected the following records:\n";
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        int fid = stoi(row[0]);
        foundIds.push_back(fid);
        cout << "ID: " << row[0]
             << " | Source: " << row[1]
             << " | Amount: RM " << row[2]
             << " | Date: " << row[3] << "\n";
    }
    mysql_free_result(res);

    // check if user asked to delete some IDs that don't belong to them
    // (if counts differ, warn and cancel)
    if (foundIds.size() != ids.size()) {
        cout << "\n⚠️  Some IDs you entered were not found or do not belong to your account.\n";
        cout << "No records were deleted. Returning to Income menu.\n";
        continue;
    }

    // 4) final confirmation
    cout << "\nThis action will PERMANENTLY delete the above " << foundIds.size() << " record(s).\n";
    cout << "Type DELETE to confirm, or 0 to cancel: ";
    string confirm;
    getline(cin, confirm);
    confirm = trim(confirm);

    if (confirm == "0") {
        cout << "Delete cancelled. Returning to Income menu.\n";
        continue;
    }
    if (confirm != "DELETE") {
        cout << "❌ Incorrect confirmation. Delete cancelled. Returning to Income menu.\n";
        continue;
    }

    // 5) perform deletion
    string delQ = "DELETE FROM income WHERE id IN (" + idsCSV + ") AND user_id=" + to_string(userId);
    if (!mysql_query(conn, delQ.c_str())) {
        cout << "🗑️ Successfully deleted " << ids.size() << " record(s).\n";
    } else {
        cout << "❌ Failed to delete records: " << mysql_error(conn) << "\n";
    }

    // return to income menu automatically after deletion
    continue;
}

        // -------------------------------------------------------
        // 5. BACK TO DASHBOARD
        // -------------------------------------------------------
        else if (choice == 5) {
            cout << "Returning to dashboard...\n";
            this_thread::sleep_for(chrono::seconds(1));
            break;
        }

        else {
            cout << "Invalid option.\n";
        }
    } // end while

    mysql_close(conn);
}