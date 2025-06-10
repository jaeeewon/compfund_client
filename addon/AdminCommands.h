#pragma once
#include <unordered_map>
#include <string>
#include "User.h"

void handleAdminCommand(std::string commandLine, User &sender,
                        std::unordered_map<std::string, User> &users);

void kickUser(std::string nickname,
              std::unordered_map<std::string, User> &users);

void muteUser(std::string nickname, int seconds,
              std::unordered_map<std::string, User> &users);

void banUser(std::string nickname,
             std::unordered_map<std::string, User> &users);

void announce(std::string message,
              User sender,
              std::unordered_map<std::string, User> &users);