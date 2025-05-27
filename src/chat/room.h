#pragma once

#include "util/global.h"
#include "util/console.h"
#include "util/tools.h"
#include "util/arrowselector.h"
#include "network/websocket.h"

Participant getParticipant(const std::string &id);

void moveToRoom(SharedState &state, std::string &input, SOCKET sock);

void handleRoomList();

void loadChats();