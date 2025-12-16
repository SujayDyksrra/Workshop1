#ifndef UTILS_H
#define UTILS_H

#include <string>

std::string getPasswordWithToggle();
std::string getPasswordWithStrength();
std::string sha256(const std::string &str);

// shared numeric validator
bool isNumber(const std::string &str);

#endif