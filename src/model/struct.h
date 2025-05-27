#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <set>
#include <mutex>
#include <vector>

struct Participant
{
    std::string id;
    std::string name;
    std::string nickname;
    std::string email;
    std::string picture;
    time_t latest_access;
};

struct ParticipantState
{
    std::unordered_map<std::string, Participant> participants;
    std::mutex mutex;
};

struct Chat
{
    std::string userId;
    std::string text;
    std::string createdAt;
};

struct RoomState
{
    std::vector<Chat> chats;
    std::set<std::string> participants;
    std::string roomName;
    std::string roomId;
    std::string description;
    std::string latestChat;
};

struct VisitingRoom
{
    std::string roomName;
    std::string roomId;
    std::string description;
    std::string latestChat;
};

struct VisitingRoomState
{
    std::vector<VisitingRoom> rooms;
    bool updated;
    std::mutex mutex;
};

struct DataRaceState
{
    bool updated;
    std::mutex mutex;
};

struct UserState
{
    std::string email;
    std::string name;
    std::string id;
    std::string ticket;
    std::string nickname;
    std::string picture;
};

struct SharedState
{
    bool isLoggedIn;
    UserState user;
    RoomState *currentRoom;
    std::map<std::string, RoomState> rooms;
    std::mutex mutex;
};