#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <mutex>
#include <thread>
#include <random>
#include <chrono>
#include <format>
#include <optional>
#include <algorithm>
#include <csignal>
#include <ctime>

#include <conio.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#undef max

#include "util/json.hpp"
using json = nlohmann::json;