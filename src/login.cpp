#include <iostream>
#include <thread>
#include <chrono>
#include <mysql/mysql.h>
#include <sstream>
#include <iomanip>
#include "utils.h"
#include "login.h"

using namespace std;

// SHA-256 is in utils (sha256)
// getPasswordWithToggle() in utils

// loginPage returns the username string if login successful, otherwise empty string.
string loginPage() {
    string username, password;

    cout << "\n========== LOGIN PAGE ==========\n";

    int attempts = 0;

    while (attempts < 2) {   // allow 2 total attempts (1 retry)
        cout << "Enter username: ";
        cin >> username;

        // clear leftover newline before calling getPasswordWithToggle
        cin.ignore(numeric_limits<std::streamsize>::max(), '\n');

        cout << "Enter password: ";
        password = getPasswordWithToggle();

        string hashedPassword = sha256(password);

        MYSQL *conn = mysql_init(NULL);

        if (!mysql_real_connect(conn, "127.0.0.1", "root", "", "finance_db", 3306, NULL, 0)) {
            cout << "❌ Database connection failed: " << mysql_error(conn) << endl;
            this_thread::sleep_for(chrono::seconds(2));
            return "";
        }

        // Compare hashed password
        string query =
            "SELECT id FROM users WHERE username='" + username +
            "' AND password='" + hashedPassword + "'";

        if (mysql_query(conn, query.c_str())) {
            cout << "❌ Query failed: " << mysql_error(conn) << endl;
            mysql_close(conn);
            return "";
        }

        MYSQL_RES *result = mysql_store_result(conn);
        int rows = mysql_num_rows(result);

        if (rows > 0) {
            cout << "\n✅ Login successful! Welcome, " << username << "!\n";
            mysql_free_result(result);
            mysql_close(conn);
            this_thread::sleep_for(chrono::seconds(1));
            return username;  // success
        }

        // login failed
        attempts++;
        mysql_free_result(result);
        mysql_close(conn);

        if (attempts < 2) {
            cout << "\n❌ Invalid username or password.\n";
            cout << "🔁 Please try again.\n\n";
            continue;   // retry once
        } else {
            cout << "\n❌ Too many login attempts.\n";
            cout << "Returning to main menu...\n";
            this_thread::sleep_for(chrono::seconds(2));
            return "";  // return to home
        }
    }

    return "";
}