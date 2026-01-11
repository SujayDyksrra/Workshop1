// src/report.cpp
#include <iostream>
#include <mysql/mysql.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <ctime>
#include <limits>
#include <thread>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <regex>
#include "report.h"

using namespace std;

/* ===============================
   ANSI COLORS (VISUAL ONLY)
================================ */
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define CYAN    "\033[36m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define RED     "\033[31m"
#define BLUE    "\033[34m"

/* =====================================================
   FORWARD DECLARATION (🔥 FIXES YOUR ERROR)
===================================================== */
static void exportReportToPDF(
    const string &username,
    const string &month,
    double income,
    double expense,
    double balance,
    double monthlyBudget,
    double savingGoal,
    const map<string,double> &categories,
    const string &insights   // ✅ KEEP INSIGHTS
);

/* =====================================================
   Helpers
===================================================== */
static string getCurrentMonth() {
    time_t now = time(0);
    tm *t = localtime(&now);
    stringstream ss;
    ss << (1900 + t->tm_year) << "-" << setw(2) << setfill('0') << (1 + t->tm_mon);
    return ss.str();
}

static int getUserId(MYSQL *conn, const string &username) {
    string q = "SELECT id FROM users WHERE username='" + username + "'";
    if (mysql_query(conn, q.c_str())) return -1;
    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);
    int id = row ? stoi(row[0]) : -1;
    mysql_free_result(res);
    return id;
}

static double getSingleValue(MYSQL *conn, const string &q) {
    if (mysql_query(conn, q.c_str())) return 0.0;
    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);
    double val = (row && row[0]) ? stod(row[0]) : 0.0;
    mysql_free_result(res);
    return val;
}

static string removeEmojis(const string &text) {
    string clean;
    for (unsigned char c : text)
        if (c >= 32 && c <= 126) clean += c;
    return clean;
}

/* =====================================================
   SMART INSIGHTS (UNCHANGED LOGIC)
===================================================== */
static string generateSmartInsights(
    const string &username,
    double income,
    double expense,
    double balance,
    double budget,
    double savingGoal,
    map<string,double> &categories
) {
    stringstream s;

    s << "👤 User: " << username << "\n\n";

    double savingsRate = (income > 0) ? ((income - expense) / income) * 100 : 0;

s << "📊 Financial Summary\n";
s << "------------------------------\n";
s << " Income   : RM " << fixed << setprecision(2) << income << "\n";
s << " Expenses : RM " << fixed << setprecision(2) << expense << "\n";
s << " Balance  : RM " << fixed << setprecision(2) << balance << "\n";
    s << "- Savings Rate: " << fixed << setprecision(1) << savingsRate << "%\n\n";
    s << "------------------------------\n";


    if (budget > 0) {
        double usage = (expense / budget) * 100;
        s << "💳 Budget Status\n";
        s << "- Used: " << usage << "%\n";
        if (usage > 100) s << "🚨 Budget exceeded\n";
        else if (usage > 90) s << "⚠️ Near limit\n";
        else s << "✅ Budget under control\n\n";
    }

    string topCat;
    double topAmt = 0;

    s << "📂 Spending Breakdown\n";
    for (auto &c : categories) {
        s << "- " << c.first << ": RM " << c.second << "\n";
        if (c.second > topAmt) {
            topAmt = c.second;
            topCat = c.first;
        }
    }

    if (!topCat.empty()) {
        s << "\n📌 Highest Category: " << topCat << "\n";
        if (topAmt > expense * 0.35)
            s << "⚠️ High spending detected\n";
        else
            s << "✅ Balanced spending\n";
    }

if (savingGoal > 0) {
    double progress = (balance / savingGoal) * 100;

    // 🔒 cap for readability (terminal + PDF consistency)
    if (progress > 100) progress = 100;

    s << "\n🎯 Saving Goal Progress\n";
    s << "- " << fixed << setprecision(1) << progress << "% achieved\n";
        if (progress >= 100)
            s << "🎉 Goal achieved\n";
        else
            s << "💡 Keep saving consistently\n";
    }

    s << "\n🤖 Smart Tips\n";
    if (savingsRate < 10) s << "- Increase savings rate\n";
    if (!topCat.empty()) s << "- Limit spending on " << topCat << "\n";
    s << "- Track expenses weekly\n";

    return s.str();
}

/* =====================================================
   PDF EXPORT (UNCHANGED, SAFE)
===================================================== */
static void exportReportToPDF(
    const string &username,
    const string &month,
    double income,
    double expense,
    double balance,
    double monthlyBudget,
    double savingGoal,
    const map<string,double> &categories,
    const string &insights
) {
    ofstream html("temp_report.html");

    /* ================= CALCULATIONS ================= */
    double budgetUsage = (monthlyBudget > 0) ? (expense / monthlyBudget) * 100 : 0;
    double savingProgress = 0;

if (savingGoal > 0) {
    savingProgress = (balance / savingGoal) * 100;
    if (savingProgress > 100) savingProgress = 100; // 🔒 cap at 100%
}

    string topCategory = "None";
    double topAmount = 0;

    for (auto &c : categories) {
        if (c.second > topAmount) {
            topAmount = c.second;
            topCategory = c.first;
        }
    }

    /* ================= HTML + CSS ================= */
    html <<
    "<html><head><meta charset='UTF-8'>"
    "<style>"
    "body{font-family:Arial;margin:40px;color:#333;background:#fff;}"
    "h1{color:#2c3e50;font-size:32px;margin-bottom:5px;}"
    "h2{color:#34495e;border-bottom:2px solid #eee;padding-bottom:6px;margin-top:35px;}"
    ".meta{color:#666;font-size:14px;margin-bottom:30px;}"
    ".card{margin-top:25px;padding:22px;border:1px solid #ddd;border-radius:10px;background:#fafafa;}"
    ".bar-bg{background:#eaeaea;border-radius:20px;height:26px;width:100%;margin-top:12px;}"
    ".bar{height:26px;border-radius:20px;color:#fff;font-size:13px;"
    "line-height:26px;text-align:right;padding-right:12px;font-weight:bold;}"
    ".green{background:#27ae60;}"
    ".orange{background:#f39c12;}"
    ".red{background:#e74c3c;}"
    ".info{margin-top:10px;font-size:14px;color:#555;}"
    ".list ul{margin-top:15px;padding-left:22px;}"
    ".list li{margin-bottom:10px;font-size:14px;}"
    ".highlight{background:#eef6ff;border-left:6px solid #3498db;"
    "padding:18px;border-radius:6px;margin-top:15px;}"
    ".footer{text-align:center;color:#777;margin-top:45px;font-size:13px;}"
    "</style></head><body>";

    /* ================= HEADER ================= */
    html << "<h1>Smart Personal Finance Report</h1>";
    html << "<div class='meta'><b>User:</b> " << username
         << " &nbsp;|&nbsp; <b>Month:</b> " << month << "</div>";

    /* ================= FINANCIAL SUMMARY ================= */
    html << "<div class='card'><h2>Financial Summary</h2>";
    html << "<p><b>Total Income:</b> RM " << income << "</p>";
    html << "<p><b>Total Expenses:</b> RM " << expense << "</p>";
    html << "<p><b>Balance:</b> RM " << balance << "</p>";
    html << "</div>";

    /* ================= BUDGET PERFORMANCE ================= */
    html << "<div class='card'><h2>Monthly Budget Performance</h2>";
    html << "<p><b>Monthly Budget:</b> RM " << monthlyBudget << "</p>";

    html << "<div class='bar-bg'><div class='bar "
         << (budgetUsage > 100 ? "red" : budgetUsage > 80 ? "orange" : "green")
         << "' style='width:" << min(100.0, budgetUsage) << "%;'>"
         << fixed << setprecision(1) << budgetUsage << "%</div></div>";

    html << "<div class='info'>";
    if (budgetUsage <= 80)
        html << "Status: Budget is well controlled.";
    else if (budgetUsage <= 100)
        html << "Status: Budget nearly exceeded.";
    else
        html << "Status: Budget exceeded. Spending adjustment required.";
    html << "</div></div>";

    /* ================= SAVING GOAL ================= */
    html << "<div class='card'><h2>Saving Goal Progress</h2>";
    html << "<p><b>Saving Goal:</b> RM " << savingGoal << "</p>";

    html << "<div class='bar-bg'><div class='bar green' style='width:"
         << min(100.0, savingProgress) << "%;'>"
         << fixed << setprecision(1) << savingProgress << "%</div></div>";

    html << "<div class='info'>";
    if (savingProgress >= 100)
        html << "Status: Saving goal achieved. Excellent financial discipline.";
    else
        html << "Status: Goal not yet achieved. Continue saving consistently.";
    html << "</div></div>";

    /* ================= HIGHEST SPENDING CATEGORY ================= */
    html << "<div class='card'><h2>Highest Spending Category</h2>";
    html << "<div class='highlight'>";
    html << "<b>Category:</b> " << topCategory << "<br/>";
    html << "<b>Total Spent:</b> RM " << topAmount << "<br/>";
    html << "</div></div>";

    /* ================= EXPENSE BREAKDOWN ================= */
    html << "<div class='card'><h2>Expense Breakdown</h2><div class='list'><ul>";
    for (auto &c : categories) {
    double pct = (expense > 0) ? (c.second / expense) * 100 : 0;

    if (pct > 100) pct = 100;   // 🔒 prevent >100%
        html << "<li><b>" << c.first << ":</b> RM " << c.second
             << " (" << fixed << setprecision(1) << pct << "%)</li>";
    }
    html << "</ul></div></div>";

    /* ================= SMART RECOMMENDATIONS ================= */
    html << "<div class='card'><h2>Smart Recommendations</h2><div class='list'><ul>";

    if (budgetUsage > 90)
        html << "<li>Reduce spending to avoid exceeding your monthly budget.</li>";

    if (savingProgress < 50)
        html << "<li>Increase monthly savings to reach your saving goal faster.</li>";

    if (topCategory != "None")
        html << "<li>Consider limiting expenses in <b>" << topCategory << "</b>.</li>";

    html << "<li>Track expenses weekly for better financial control.</li>";
    html << "</ul></div></div>";

    /* ================= FOOTER ================= */
    html << "<div class='footer'>Generated by Smart Personal Finance Manager (SPFM)</div>";

    html << "</body></html>";
    html.close();

    system("wkhtmltopdf --margin-top 15mm --margin-bottom 15mm temp_report.html financial_report.pdf");
    remove("temp_report.html");

    cout << "📄 PDF exported: financial_report.pdf\n";
}
/* =====================================================
   MAIN MODULE (VISUALS ONLY ADDED)
===================================================== */
void reportManagement(const string &username) {

    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "127.0.0.1", "root", "", "finance_db", 3306, NULL, 0)) {
        cout << RED << "❌ Database connection failed\n" << RESET;
        return;
    }

    int userId = getUserId(conn, username);
    if (userId < 0) { mysql_close(conn); return; }

    while (true) {
        cout << BLUE << BOLD
             << "\n========================================\n"
             << "      REPORT & SMART INSIGHTS (SPFM)\n"
             << "========================================\n"
             << RESET;
        cout << "1. Generate Monthly Report\n";
        cout << "2. View Saved Reports\n";
        cout << "3. Back to Dashboard\n";
        cout << "Choice: ";

        int choice;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        /* ================= GENERATE ================= */
        if (choice == 1) {

            cout << "Enter month (YYYY-MM) or press ENTER for current month: ";
            string month;
            getline(cin, month);
            if (month.empty()) month = getCurrentMonth();
            else if (!regex_match(month, regex("^[0-9]{4}-[0-9]{2}$"))) {
                cout << RED << "❌ Invalid format\n" << RESET;
                continue;
            }

            double income = getSingleValue(conn,
                "SELECT SUM(amount) FROM income WHERE user_id=" + to_string(userId) +
                " AND DATE_FORMAT(income_date,'%Y-%m')='" + month + "'");

            double expense = getSingleValue(conn,
                "SELECT SUM(amount) FROM expenses WHERE user_id=" + to_string(userId) +
                " AND DATE_FORMAT(expense_date,'%Y-%m')='" + month + "'");

            double budget = getSingleValue(conn,
                "SELECT monthly_budget FROM users WHERE id=" + to_string(userId));

            double savingGoal = getSingleValue(conn,
                "SELECT saving_goal FROM users WHERE id=" + to_string(userId));

            double balance = income - expense;

            map<string,double> categories;
            string q = "SELECT category, SUM(amount) FROM expenses WHERE user_id=" +
                       to_string(userId) + " GROUP BY category";

            mysql_query(conn, q.c_str());
            MYSQL_RES *res = mysql_store_result(conn);
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)))
                categories[row[0]] = stod(row[1]);
            mysql_free_result(res);

            string insights = generateSmartInsights(
                username, income, expense, balance, budget, savingGoal, categories
            );

            cout << CYAN << "\n========== REPORT ==========\n" << RESET;
            cout << insights;
            cout << CYAN << "============================\n" << RESET;

            cout << "Save & export PDF? (y/n): ";
            char save; cin >> save; cin.ignore();

            if (tolower(save) == 'y') {
exportReportToPDF(
    username,
    month,
    income,
    expense,
    balance,
    budget,
    savingGoal,
    categories,
    insights      // ✅ now included
);

                string insertQ =
                    "INSERT INTO reports (user_id, month, total_income, total_expense, balance, insights) VALUES (" +
                    to_string(userId) + ",'" + month + "'," +
                    to_string(income) + "," + to_string(expense) + "," +
                    to_string(balance) + ",'" + insights + "')";

                if (!mysql_query(conn, insertQ.c_str()))
                    cout << GREEN << "✅ Report saved\n" << RESET;
                else
                    cout << RED << "❌ Save failed\n" << RESET;
            }
        }
        /* ================= VIEW / DELETE SAVED REPORTS ================= */
else if (choice == 2) {

    cout << BLUE << BOLD
         << "\n========================================\n"
         << "           SAVED REPORTS (SPFM)\n"
         << "========================================\n"
         << RESET;

    string q =
        "SELECT id, month, total_income, total_expense, balance, insights "
        "FROM reports WHERE user_id=" + to_string(userId);

    if (mysql_query(conn, q.c_str())) {
        cout << RED << "❌ Failed to load reports\n" << RESET;
        continue;
    }

    MYSQL_RES *res = mysql_store_result(conn);

    if (!res || mysql_num_rows(res) == 0) {
        cout << YELLOW << "No saved reports found.\n" << RESET;
        if (res) mysql_free_result(res);
        continue;
    }

    vector<vector<string>> rows;
    MYSQL_ROW r;

    cout << CYAN << "\nAvailable Reports:\n" << RESET;
    while ((r = mysql_fetch_row(res))) {
        rows.push_back({r[0], r[1], r[2], r[3], r[4], r[5]});
        cout << "ID: " << r[0]
             << " | Month: " << r[1]
             << " | Balance: RM " << r[4] << "\n";
    }
    mysql_free_result(res);

    cout << "\nEnter Report ID (0 to go back): ";
    int rid;
    cin >> rid;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (rid == 0) continue;

    auto it = find_if(rows.begin(), rows.end(),
        [&](auto &v){ return stoi(v[0]) == rid; });

    if (it == rows.end()) {
        cout << RED << "❌ Invalid Report ID\n" << RESET;
        continue;
    }

    cout << BLUE << "\n----------------------------------------\n" << RESET;
    cout << "1. View Report\n";
    cout << "2. Delete Report\n";
    cout << "3. Back\n";
    cout << "Choice: ";

    int sub;
    cin >> sub;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    /* ---------- VIEW REPORT ---------- */
    if (sub == 1) {
        cout << CYAN << "\n========== SAVED REPORT ==========\n" << RESET;
        cout << it->at(5);
        cout << CYAN << "=================================\n" << RESET;

        cout << "Export PDF? (y/n): ";
        char ex;
        cin >> ex;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (tolower(ex) == 'y') {

            // ✅ FETCH USER SETTINGS ONCE (NO DUPLICATES)
            double monthlyBudget = getSingleValue(
                conn,
                "SELECT monthly_budget FROM users WHERE id=" + to_string(userId)
            );

            double savingGoal = getSingleValue(
                conn,
                "SELECT saving_goal FROM users WHERE id=" + to_string(userId)
            );

            // ✅ FETCH CATEGORY BREAKDOWN FOR THIS REPORT MONTH
            map<string,double> categories;
            string cq =
                "SELECT category, SUM(amount) FROM expenses WHERE user_id=" +
                to_string(userId) +
                " AND DATE_FORMAT(expense_date,'%Y-%m')='" + it->at(1) + "'";

            mysql_query(conn, cq.c_str());
            MYSQL_RES *cres = mysql_store_result(conn);
            MYSQL_ROW crow;

            while ((crow = mysql_fetch_row(cres))) {
                categories[crow[0]] = stod(crow[1]);
            }
            mysql_free_result(cres);

            // ✅ EXPORT PDF WITH REAL DATA
            exportReportToPDF(
                username,
                it->at(1),
                stod(it->at(2)), // income
                stod(it->at(3)), // expense
                stod(it->at(4)), // balance
                monthlyBudget,
                savingGoal,
                categories,
                it->at(5)        // stored insights
            );
        }
    }

    /* ---------- DELETE REPORT ---------- */
    else if (sub == 2) {
        cout << RED << "\n⚠️  This will permanently delete the report.\n" << RESET;
        cout << "Type YES to confirm deletion: ";

        string confirm;
        getline(cin, confirm);

        if (confirm == "YES") {
            string delQ =
                "DELETE FROM reports WHERE id=" + to_string(rid) +
                " AND user_id=" + to_string(userId);

            if (!mysql_query(conn, delQ.c_str()))
                cout << GREEN << "🗑️ Report deleted successfully\n" << RESET;
            else
                cout << RED << "❌ Failed to delete report\n" << RESET;
        }
        else {
            cout << YELLOW << "Deletion cancelled\n" << RESET;
        }
    }

    else {
        cout << YELLOW << "Returning to report menu...\n" << RESET;
    }
}

        else if (choice == 3) break;
        else cout << RED << "Invalid choice\n" << RESET;
    }

    mysql_close(conn);
}