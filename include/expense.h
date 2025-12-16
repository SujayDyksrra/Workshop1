#ifndef EXPENSE_H
#define EXPENSE_H

#include <string>

// Opens the expense management UI for given username (blocks until user returns)
void expenseManagement(const std::string &username);

#endif // EXPENSE_H