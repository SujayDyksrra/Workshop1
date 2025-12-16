#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

using namespace std;

#include <regex>

bool isNumber(const std::string &str) {
    return std::regex_match(str, std::regex("^[0-9]+(\\.[0-9]{1,2})?$"));
}

// 🔒 Improved hidden password input
string getPasswordWithToggle() {
    string input;
    char ch;
    bool hidden = true;

    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    cout << "(Press TAB to show/hide password)" << endl;

    while (true) {
        ch = getchar();
        if (ch == '\n') break;

        if (ch == '\t') {
            hidden = !hidden;
        }
        else if (ch == 127 || ch == 8) {
            if (!input.empty()) {
                input.pop_back();
            }
        }
        else {
            input += ch;
        }

        cout << "\rPassword: ";
        if (hidden) {
            for (size_t i = 0; i < input.size(); i++) cout << "*";
        } else {
            cout << input;
        }
        cout << "       ";
        fflush(stdout);
    }

    cout << endl;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return input;
}
string getPasswordWithStrength() {
    string input;
    char ch;
    bool hidden = true;

    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    auto strengthText = [&](const string& pw) {
        int len = pw.size();
        bool lower = false, upper = false, digit = false, special = false;

        for (char c : pw) {
            if (islower(c)) lower = true;
            else if (isupper(c)) upper = true;
            else if (isdigit(c)) digit = true;
            else special = true;
        }

        int classes = lower + upper + digit + special;

        if (len >= 10 && classes >= 3) return string("Strong");
        if (len >= 8 && classes >= 2) return string("Medium");
        return string("Weak");
    };

    auto strengthBar = [&](const string& pw) {
        string s = strengthText(pw);

        if (s == "Weak") return string("▓▓░░░░");
        if (s == "Medium") return string("▓▓▓▓░░");
        return string("▓▓▓▓▓▓");
    };

    cout << "(Press TAB to show/hide password)" << endl;

    while (true) {
        ch = getchar();
        if (ch == '\n') break;

        if (ch == '\t') {
            hidden = !hidden;
        }
        else if (ch == 127 || ch == 8) {
            if (!input.empty()) input.pop_back();
        }
        else {
            input += ch;
        }

        cout << "\rPassword: ";
        if (hidden) {
            for (size_t i = 0; i < input.size(); i++) cout << "*";
        } else {
            cout << input;
        }

        string s = strengthText(input);

        cout << "   Strength: " << s;
        cout << "  [" << strengthBar(input) << "]";
        cout << "          ";

        fflush(stdout);
    }

    cout << endl;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return input;
}

// 🔐 SHA-256 hashing
string sha256(const string &str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)str.c_str(), str.size(), hash);

    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}


