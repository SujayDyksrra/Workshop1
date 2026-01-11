#include <iostream>
#include <thread>
#include <chrono>
#include <mysql/mysql.h>
#include <sstream>
#include <iomanip>
#include <limits>
#include "utils.h"
#include "login.h"

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

/*
 loginPage returns username if login successful,
 otherwise returns empty string.
*/
string loginPage() {

    string username, password;
    int attempts = 0;

    cout << BLUE << BOLD;
    cout << "\n============================================================\n";
    cout << "                     SPFM LOGIN\n";
    cout << "============================================================\n";
    cout << RESET;

    while (attempts < 2) {   // max 2 attempts

        cout << CYAN << "\nUsername: " << RESET;
        cin >> username;

        // clear newline before password input
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << CYAN << "Password: " << RESET;
        password = getPasswordWithToggle();

        string hashedPassword = sha256(password);

        MYSQL *conn = mysql_init(NULL);

        if (!mysql_real_connect(conn, "127.0.0.1", "root", "", "finance_db", 3306, NULL, 0)) {
            cout << RED << "\n❌ Database connection failed: "
                 << mysql_error(conn) << RESET << endl;
            this_thread::sleep_for(chrono::seconds(2));
            return "";
        }

        string query =
            "SELECT id FROM users WHERE username='" + username +
            "' AND password='" + hashedPassword + "'";

        if (mysql_query(conn, query.c_str())) {
            cout << RED << "\n❌ Query failed: "
                 << mysql_error(conn) << RESET << endl;
            mysql_close(conn);
            return "";
        }

        MYSQL_RES *result = mysql_store_result(conn);
        int rows = mysql_num_rows(result);

        if (rows > 0) {
            cout << GREEN << BOLD
                 << "\n✅ Login successful!\n"
                 << "Welcome back, " << username << " 👋\n"
                 << RESET;

            mysql_free_result(result);
            mysql_close(conn);
            this_thread::sleep_for(chrono::seconds(1));
            return username;
        }

        // Login failed
        attempts++;
        mysql_free_result(result);
        mysql_close(conn);

        if (attempts < 2) {
            cout << RED << "\n❌ Invalid username or password.\n" << RESET;
            cout << YELLOW << "🔁 Please try again.\n" << RESET;
        } else {
            cout << RED << BOLD
                 << "\n❌ Too many login attempts.\n"
                 << RESET;
            cout << YELLOW << "Returning to main menu...\n" << RESET;
            this_thread::sleep_for(chrono::seconds(2));
            return "";
        }
    }

    return "";
}