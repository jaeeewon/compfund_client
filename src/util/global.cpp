#include <winsock2.h>
#include "model/struct.h"

std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

SOCKET sock;
SharedState shared;
VisitingRoomState visiting;
ParticipantState ptstate;
DataRaceState join_room;
DataRaceState load_chat;
LeaderboardState load_leaderboard;