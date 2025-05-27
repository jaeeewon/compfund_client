#pragma once

#include <winsock2.h>
#include "model/struct.h"

extern std::string base64_chars;

extern SOCKET sock;
extern SharedState shared;
extern VisitingRoomState visiting;
extern ParticipantState ptstate;
extern DataRaceState join_room;
extern DataRaceState load_chat;