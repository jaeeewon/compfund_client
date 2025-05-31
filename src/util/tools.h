#pragma once

#include <string>
#include <vector>
#include <optional>

std::string base64_encode(const std::string &in);
std::vector<unsigned char> base64_decode(const std::string &in);
std::string generate_ws_key();
std::optional<std::string> decode_websocket_payload_safe(const char *buffer, int len);
void signalHandler(int signum = 0);
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);
void render_ascii_from_grayscale_raw(const std::vector<unsigned char> &raw, int width, int height);
void openMessageBox(const std::string &title, const std::string &body);
std::pair<std::string, std::vector<std::string>> prettier(
    const std::vector<std::string> &keys,
    const std::vector<std::vector<std::string>> &values);
std::string timeAgo(time_t past);