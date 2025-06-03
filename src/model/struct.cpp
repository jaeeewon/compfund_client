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
    std::string status;
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
    time_t createdAt;
    std::set<std::string> readers; // made by EUNHYE(2025-05-29): This stores the list of user IDs who have read the message.
};

struct RoomState
{
    std::vector<Chat> chats;
    std::unordered_map<std::string, time_t> participants;
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
    bool updated = false;
    std::mutex mutex;
};

struct DataRaceState
{
    bool updated = false;
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
    bool isLoggedIn = false;
    UserState user;
    RoomState *currentRoom = nullptr;
    std::map<std::string, RoomState> rooms;
    std::mutex mutex;
};