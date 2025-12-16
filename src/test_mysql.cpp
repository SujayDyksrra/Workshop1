#include <iostream>
#include <mysql/mysql.h>

using namespace std;

int main() {
    MYSQL *conn;
    conn = mysql_init(NULL);

    if (!conn) {
        cout << "MySQL init failed!" << endl;
        return 1;
    }

    cout << "🔌 Connecting to MySQL (XAMPP 3306)..." << endl;

    if (!mysql_real_connect(conn,
                            "127.0.0.1",  // host
                            "root",       // username
                            "",           // password (empty for XAMPP)
                            "finance_db", // your db
                            3306,         // port
                            NULL, 0)) {

        cout << "❌ Connection failed: " << mysql_error(conn) << endl;
        return 1;
    }

    cout << "✅ Successfully connected to MySQL!" << endl;

    mysql_close(conn);
    return 0;
}