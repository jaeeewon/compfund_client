#include "AdminCommands.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <unistd.h>

void handleAdminCommand(std::string commandLine, User &sender,
                        std::unordered_map<std::string, User> &users)
{
    if (sender.isAdmin == false)
    {
        send(sender.socketFd, "관리자 권한이 없습니다.\n", 28, 0);
        return;
    }

    std::stringstream ss(commandLine);
    std::string cmd;
    ss >> cmd;

    std::string arg;
    ss >> arg;

    if (cmd.compare("/kick") == 0)
    {
        kickUser(arg, users);
    }
    else if (cmd.compare("/mute") == 0)
    {
        std::string timeStr;
        ss >> timeStr;
        int sec = atoi(timeStr.c_str());
        muteUser(arg, sec, users);
    }
    else if (cmd.compare("/ban") == 0)
    {
        banUser(arg, users);
    }
    else if (cmd.compare("/announce") == 0)
    {
        std::string remain;
        getline(ss, remain);
        announce(remain, sender, users);
    }
}

void kickUser(std::string nickname,
              std::unordered_map<std::string, User> &users)
{
    if (users.count(nickname) > 0)
    {
        close(users[nickname].socketFd);
        users.erase(nickname);
        std::cout << nickname << " has been kicked." << std::endl;
    }
}

void muteUser(std::string nickname, int seconds,
              std::unordered_map<std::string, User> &users)
{
    if (users.find(nickname) != users.end())
    {
        User &u = users[nickname];
        u.isMuted = true;
        u.muteEndTime = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    }
}

void banUser(std::string nickname,
             std::unordered_map<std::string, User> &users)
{
    auto it = users.find(nickname);
    if (it != users.end())
    {
        it->second.isBanned = true;
        close(it->second.socketFd);
        users.erase(it);
    }
}

void announce(std::string message,
              User sender,
              std::unordered_map<std::string, User> &users)
{
    std::string msg = "[공지]" + message + "\n";
    for (auto it = users.begin(); it != users.end(); ++it)
    {
        send(it->second.socketFd, msg.c_str(), msg.length(), 0);
    }
}
