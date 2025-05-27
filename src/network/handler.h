#pragma once

#include "util/common.h"
#include "model/struct.h"
#include "util/global.h"
#include "util/tools.h"

// std::string latest_cout;

// void cout_dupl(std::string str, bool endl);

void handleWebsocket(std::string response, SharedState &state);