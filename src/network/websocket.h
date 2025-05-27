#pragma once

#include "util/common.h"

void send_ws_message(SOCKET sock, const std::string &type, const json &data);
SOCKET initializeSocket();
void receive_loop(SOCKET sock, SharedState &state);