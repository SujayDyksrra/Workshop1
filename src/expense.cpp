// src/expense.cpp
#include <iostream>
#include <mysql/mysql.h>
#include <iomanip>
#include <regex>
#include <chrono>
#include <thread>
#include "expense.h"
#include "utils.h"   // isNumber(...)
#include <ctime>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <limits>
#include <map>
#include <vector>

using namespace std;

// ----- small helpers (file-local) -----
static string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    if (start == string::npos) return "";
    return s.substr(start, end - start + 1);
}
static string toLower(const string &s) {
    string out = s;
    transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return tolower(c);});
    return out;
}
static string formatDate(int y, int m, int d) {
    char buf[11];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", y, m, d);
    return string(buf);
}
static string getToday() {
    time_t now = time(0);
    tm *t = localtime(&now);
    return formatDate(1900 + t->tm_year, 1 + t->tm_mon, t->tm_mday);
}
static string parseNaturalDate(const string &inputRaw) {
    string in = toLower(trim(inputRaw));
    time_t now = time(0);
    tm t = *localtime(&now);

    if (in == "today" || in == "td" || in == "t") return getToday();
    if (in == "yesterday" || in == "yd") { t.tm_mday -= 1; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday); }
    if (in == "tomorrow" || in == "tm")  { t.tm_mday += 1; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday); }
    if (!in.empty() && in[0] == '+') { int n = stoi(in.substr(1)); t.tm_mday += n; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday); }
    if (!in.empty() && in[0] == '-') { int n = stoi(in.substr(1)); t.tm_mday -= n; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday); }
    if (in == "next week") { t.tm_mday += 7; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday); }
    if (in == "last week") { t.tm_mday -= 7; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday); }
    if (in == "next month") { t.tm_mon += 1; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, min(t.tm_mday,28)); }
    if (in == "last month") { t.tm_mon -= 1; mktime(&t); return formatDate(1900 + t.tm_year, 1 + t.tm_mon, min(t.tm_mday,28)); }
    return "";
}
static string parseNumericDate(const string &inputRaw) {
    string in = trim(inputRaw);
    if (regex_match(in, regex("^[0-9]{6}$"))) {
        int d = stoi(in.substr(0,2));
        int m = stoi(in.substr(2,2));
        int y = 2000 + stoi(in.substr(4,2));
        return formatDate(y,m,d);
    }
    if (regex_match(in, regex("^[0-9]{1,2}/[0-9]{1,2}/[0-9]{4}$"))) {
        int d,m,y; char c1,c2; stringstream ss(in); ss>>d>>c1>>m>>c2>>y; return formatDate(y,m,d);
    }
    if (regex_match(in, regex("^[0-9]{1,2}-[0-9]{1,2}-[0-9]{4}$"))) {
        int d,m,y; char c1,c2; stringstream ss(in); ss>>d>>c1>>m>>c2>>y; return formatDate(y,m,d);
    }
    if (regex_match(in, regex("^[0-9]{4}-[0-9]{2}-[0-9]{2}$"))) return in;
    return "";
}
static string getDateWithCancel() {
    while (true) {
        cout << "Enter date (press ENTER for today, type 0 to cancel; examples: today, +3, 02/02/2025): ";
        string input;
        getline(cin, input);
        if (input == "0") return "";
        if (input.empty()) return getToday();
        string nat = parseNaturalDate(input);
        if (!nat.empty()) return nat;
        string num = parseNumericDate(input);
        if (!num.empty()) return num;
        cout << "❌ Invalid date format. Try again or type 0 to cancel.\n";
    }
}

// fetch user id
static int getUserId(MYSQL *conn, const string &username) {
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

// ----------------- Module Entry -----------------
void expenseManagement(const string &username) {
    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "127.0.0.1", "root", "", "finance_db", 3306, NULL, 0)) {
        cout << "❌ Database connection failed: " << mysql_error(conn) << endl;
        return;
    }

    int userId = getUserId(conn, username);
    if (userId < 0) { cout << "❌ Unable to identify user.\n"; mysql_close(conn); return; }

    while (true) {
        int choice = 0;
        cout << "\n========== EXPENSE MANAGEMENT ==========\n";
        cout << "1. Add Expense\n2. View Expense Records\n3. Update Expense\n4. Delete Expense\n5. Report (by category / month)\n6. Back to Dashboard\n";
        cout << "=========================================\n";
        cout << "Enter choice: ";
        if (!(cin >> choice)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input.\n"; continue; }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        // -------- 1. Add Expense --------
        if (choice == 1) {
            while (true) {
                cout << "\n--- Add Expense (type 0 at any prompt to cancel) ---\n";
                string category, amountStr, date, notes;
                cout << "Enter category (e.g. Food & Beverages, Transport): ";
                getline(cin, category);
                if (category == "0") { cout << "Add cancelled.\n"; break; }
                if (trim(category).empty()) { cout << "❌ Category cannot be empty. Retry or type 0 to cancel.\n"; continue; }

                // amount
                bool okAmount = false;
                do {
                    cout << "Enter amount (RM): ";
                    if (!getline(cin, amountStr)) amountStr = "";
                    if (amountStr == "0") { cout << "Add cancelled.\n"; okAmount = false; break; }
                    if (!isNumber(amountStr) || stod(amountStr) <= 0) {
                        cout << "❌ Invalid amount. Enter a positive number. Type 0 to cancel.\n"; amountStr = ""; continue;
                    }
                    okAmount = true;
                } while (!okAmount);
                if (!okAmount) break;

                // date
                date = getDateWithCancel();
                if (date.empty()) { cout << "Add cancelled.\n"; break; }

                cout << "Optional notes (press ENTER to skip): ";
                getline(cin, notes);

                // insert
                string q = "INSERT INTO expenses (user_id, category, amount, expense_date, notes) VALUES (" +
                           to_string(userId) + ", '" + /*escape simple single quotes*/ 
                           string([&category]()->string { string s=category; size_t p=0; while((p=s.find('\'',p))!=string::npos){ s.insert(p, "\\"); p+=2;} return s;}()) +
                           "', " + amountStr + ", '" + date + "', '" +
                           string([&notes]()->string { string s=notes; size_t p=0; while((p=s.find('\'',p))!=string::npos){ s.insert(p, "\\"); p+=2;} return s;}()) + "')";

                if (!mysql_query(conn, q.c_str())) cout << "✅ Expense added successfully!\n";
                else cout << "❌ Failed to add expense: " << mysql_error(conn) << endl;

                this_thread::sleep_for(chrono::seconds(1));
                break;
            }
        }

        // -------- 2. View Expenses --------
        else if (choice == 2) {
            string q = "SELECT id, category, amount, expense_date, notes FROM expenses WHERE user_id=" + to_string(userId) + " ORDER BY expense_date DESC";
            if (mysql_query(conn, q.c_str())) { cout << "❌ Query failed: " << mysql_error(conn) << endl; continue; }
            MYSQL_RES *res = mysql_store_result(conn);
            int rows = res ? mysql_num_rows(res) : 0;
            if (rows == 0) { cout << "No expense records found.\n"; if (res) mysql_free_result(res); continue; }
            cout << "\n------ Expense Records ------\n";
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                cout << "ID: " << row[0] << " | Category: " << row[1] << " | Amount: RM " << row[2] << " | Date: " << row[3];
                if (row[4] && strlen(row[4])>0) cout << " | Notes: " << row[4];
                cout << "\n";
            }
            cout << "-----------------------------\n";
            mysql_free_result(res);
        }

        // -------- 3. Update Expense (auto-display then choose ID) --------
        else if (choice == 3) {
            // auto display same as view
            string listQ = "SELECT id, category, amount, expense_date FROM expenses WHERE user_id=" + to_string(userId) + " ORDER BY expense_date DESC";
            if (mysql_query(conn, listQ.c_str())) { cout << "❌ Failed to load records.\n"; continue; }
            MYSQL_RES *resList = mysql_store_result(conn);
            if (!resList || mysql_num_rows(resList) == 0) { cout << "No expenses to update.\n"; if (resList) mysql_free_result(resList); continue; }
            cout << "\n------ Your Expense Records ------\n";
            MYSQL_ROW rr;
            while ((rr = mysql_fetch_row(resList))) {
                cout << "ID: " << rr[0] << " | Category: " << rr[1] << " | Amount: RM " << rr[2] << " | Date: " << rr[3] << "\n";
            }
            cout << "----------------------------------\n";
            mysql_free_result(resList);

            int id;
            cout << "Enter the Expense ID to update (0 to cancel): ";
            if (!(cin >> id)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input.\n"; continue; }
            if (id == 0) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); continue; }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            // load record
            string fetchQ = "SELECT category, amount, expense_date, notes FROM expenses WHERE id=" + to_string(id) + " AND user_id=" + to_string(userId);
            if (mysql_query(conn, fetchQ.c_str())) { cout << "❌ Failed to fetch expense.\n"; continue; }
            MYSQL_RES *r = mysql_store_result(conn);
            if (!r || mysql_num_rows(r) == 0) { cout << "❌ No expense found with this ID.\n"; if (r) mysql_free_result(r); continue; }
            MYSQL_ROW row = mysql_fetch_row(r);
            string oldCategory = row[0] ? row[0] : "";
            string oldAmount = row[1] ? row[1] : "0.00";
            string oldDate = row[2] ? row[2] : getToday();
            string oldNotes = row[3] ? row[3] : "";
            mysql_free_result(r);

            cout << "\n--- Current Expense ---\n";
            cout << "Category: " << oldCategory << "\nAmount: RM " << oldAmount << "\nDate: " << oldDate << "\nNotes: " << oldNotes << "\n";

            string newCategory = oldCategory, newAmount = oldAmount, newDate = oldDate, newNotes = oldNotes;
            bool done = false;
            while (!done) {
                cout << "\nUpdate menu:\n1. Category\n2. Amount\n3. Date\n4. Notes\n5. Cancel and return\nChoose: ";
                int uc;
                if (!(cin >> uc)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input.\n"; continue; }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                switch (uc) {
                    case 1: {
                        cout << "Enter new category (or 0 to keep current): ";
                        string tmp; getline(cin, tmp);
                        if (tmp != "0" && !trim(tmp).empty()) newCategory = tmp;
                        break;
                    }
                    case 2: {
                        while (true) {
                            cout << "Enter new amount (or 0 to keep current): ";
                            string amt; getline(cin, amt);
                            if (amt == "0") { newAmount = oldAmount; break; }
                            if (!isNumber(amt) || stod(amt) <= 0) { cout << "❌ Invalid amount.\n"; continue; }
                            newAmount = amt; break;
                        }
                        break;
                    }
                    case 3: {
                        cout << "Enter new date (ENTER = today, 0 = keep current): ";
                        string tmp; getline(cin, tmp);
                        if (tmp == "0") newDate = oldDate;
                        else if (tmp.empty()) newDate = getToday();
                        else {
                            string nat = parseNaturalDate(tmp);
                            string num = parseNumericDate(tmp);
                            if (!nat.empty()) newDate = nat; else if (!num.empty()) newDate = num; else { cout << "❌ Invalid format. Keeping current.\n"; }
                        }
                        break;
                    }
                    case 4: {
                        cout << "Enter new notes (or press ENTER to clear): ";
                        string tmp; getline(cin, tmp);
                        newNotes = tmp;
                        break;
                    }
                    case 5:
                        cout << "Update cancelled.\n";
                        done = true;
                        break;
                    default:
                        cout << "Invalid option.\n";
                }
            }
        }

        // -------- 4. Delete Expense (display then multiple delete) --------
        else if (choice == 4) {
            // display records first
            string listQ = "SELECT id, category, amount, expense_date FROM expenses WHERE user_id=" + to_string(userId) + " ORDER BY expense_date DESC";
            if (mysql_query(conn, listQ.c_str())) { cout << "❌ Failed to load records.\n"; continue; }
            MYSQL_RES *resList = mysql_store_result(conn);
            if (!resList || mysql_num_rows(resList) == 0) { cout << "No expenses to delete.\n"; if (resList) mysql_free_result(resList); continue; }
            cout << "\n------ Your Expense Records ------\n";
            MYSQL_ROW rr;
            while ((rr = mysql_fetch_row(resList))) {
                cout << "ID: " << rr[0] << " | Category: " << rr[1] << " | Amount: RM " << rr[2] << " | Date: " << rr[3] << "\n";
            }
            cout << "----------------------------------\n";
            mysql_free_result(resList);

            cout << "Enter Expense ID(s) to delete separated by commas or 0 to cancel: ";
            string idsInput; getline(cin, idsInput); idsInput = trim(idsInput);
            if (idsInput.empty() || idsInput == "0") { cout << "Delete cancelled.\n"; continue; }

            // parse and validate numeric tokens
            vector<string> tokens;
            string tmp; for (char c: idsInput) { if (c==',') { if(!tmp.empty()){ tokens.push_back(trim(tmp)); tmp.clear(); } } else tmp.push_back(c); } if (!tmp.empty()) tokens.push_back(trim(tmp));
            vector<int> ids; bool bad=false;
            for (auto &t: tokens) { if (!regex_match(t, regex("^[0-9]+$"))) { bad=true; break; } ids.push_back(stoi(t)); }
            if (bad || ids.empty()) { cout << "❌ Invalid ID list.\n"; continue; }

            // check ownership and show to-delete
            string csv; for (size_t i=0;i<ids.size();++i){ if (i) csv += ","; csv += to_string(ids[i]); }
            string fetchQ = "SELECT id, category, amount, expense_date FROM expenses WHERE id IN (" + csv + ") AND user_id=" + to_string(userId);
            if (mysql_query(conn, fetchQ.c_str())) { cout << "❌ Failed to check IDs.\n"; continue; }
            MYSQL_RES *res = mysql_store_result(conn);
            if (!res || mysql_num_rows(res) == 0) { cout << "❌ No matching records found for the provided IDs.\n"; if (res) mysql_free_result(res); continue; }
            cout << "\nYou will delete:\n";
            vector<int> found;
            MYSQL_ROW r2;
            while ((r2 = mysql_fetch_row(res))) {
                cout << "ID: " << r2[0] << " | Category: " << r2[1] << " | Amount: RM " << r2[2] << " | Date: " << r2[3] << "\n";
                found.push_back(stoi(r2[0]));
            }
            mysql_free_result(res);
            if (found.size() != ids.size()) { cout << "⚠️ Some provided IDs were not found or not yours. No deletions performed.\n"; continue; }

            cout << "\nType DELETE to confirm deletion, or 0 to cancel: ";
            string confirm; getline(cin, confirm); confirm = trim(confirm);
            if (confirm == "0") { cout << "Delete cancelled.\n"; continue; }
            if (confirm != "DELETE") { cout << "❌ Incorrect confirmation. Delete cancelled.\n"; continue; }

            string delQ = "DELETE FROM expenses WHERE id IN (" + csv + ") AND user_id=" + to_string(userId);
            if (!mysql_query(conn, delQ.c_str())) cout << "🗑️ Successfully deleted " << ids.size() << " record(s).\n";
            else cout << "❌ Failed to delete: " << mysql_error(conn) << endl;
        }

        // -------- 5. Report: totals by category and month --------
        else if (choice == 5) {
            cout << "\nReport options:\n1. Total by category (all time)\n2. Monthly summary (choose month)\nChoose: ";
            int rc; if (!(cin >> rc)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input.\n"; continue; }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (rc == 1) {
                string q = "SELECT category, SUM(amount) as total FROM expenses WHERE user_id=" + to_string(userId) + " GROUP BY category ORDER BY total DESC";
                if (mysql_query(conn, q.c_str())) { cout << "❌ Query failed: " << mysql_error(conn) << endl; continue; }
                MYSQL_RES *r = mysql_store_result(conn);
                if (!r || mysql_num_rows(r) == 0) { cout << "No expenses to report.\n"; if (r) mysql_free_result(r); continue; }
                cout << "\nTotal Expenses by Category:\n";
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(r))) {
                    cout << row[0] << " : RM " << row[1] << "\n";
                }
                mysql_free_result(r);
            } else if (rc == 2) {
                cout << "Enter month to report (YYYY-MM) or press ENTER for current month: ";
                string in; getline(cin, in);
                string monthFilter;
                if (trim(in).empty()) {
                    // current month
                    time_t now = time(0); tm *t = localtime(&now);
                    char buf[8]; snprintf(buf,sizeof(buf), "%04d-%02d", 1900+t->tm_year, 1+t->tm_mon);
                    monthFilter = string(buf);
                } else {
                    // basic validation YYYY-MM
                    if (!regex_match(trim(in), regex("^[0-9]{4}-[0-9]{2}$"))) { cout << "❌ Invalid month format.\n"; continue; }
                    monthFilter = trim(in);
                }
                string q = "SELECT SUM(amount) FROM expenses WHERE user_id=" + to_string(userId) + " AND DATE_FORMAT(expense_date, '%Y-%m')='" + monthFilter + "'";
                if (mysql_query(conn, q.c_str())) { cout << "❌ Query failed: " << mysql_error(conn) << endl; continue; }
                MYSQL_RES *r = mysql_store_result(conn);
                MYSQL_ROW row = mysql_fetch_row(r);
                string total = (row && row[0]) ? row[0] : "0.00";
                cout << "\nTotal expenses for " << monthFilter << " : RM " << total << "\n";
                mysql_free_result(r);
            } else cout << "Invalid report choice.\n";
        }

        // -------- 6. Back --------
        else if (choice == 6) {
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