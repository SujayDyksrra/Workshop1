#ifndef USERPROFILE_H
#define USERPROFILE_H

#include <string>

// returns true if account still exists after returning,
// returns false if the user deleted their account (caller should log out / go to main menu)
bool userProfile(const std::string &username);

#endif