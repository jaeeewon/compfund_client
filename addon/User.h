#pragma once
#include <string>
#include <chrono>

struct User
{
    std::string nickname;
    int socketFd;
    bool isAdmin;
    bool isMuted = false;
    std::chrono::steady_clock::time_point muteEndTime;
    bool isBanned = false;
};