#include <string>
#include <optional>
#include <vector>
#include <iostream>
#include "util/global.h"
#include "util/common.h"

std::string base64_encode(const std::string &in)
{
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4)
        out.push_back('=');
    return out;
}

std::vector<unsigned char> base64_decode(const std::string &in)
{
    std::vector<unsigned char> out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
        T[base64_chars[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : in)
    {
        if (T[c] == -1)
            break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0)
        {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

std::string generate_ws_key()
{
    std::string key;
    for (int i = 0; i < 16; ++i)
        key.push_back(static_cast<char>(rand() % 256));
    return base64_encode(key);
}

std::optional<std::string> decode_websocket_payload_safe(const char *buffer, int len)
{
    if (len < 2)
        return std::nullopt;

    const unsigned char *buf = reinterpret_cast<const unsigned char *>(buffer);
    bool mask = (buf[1] & 0x80) != 0;
    uint64_t payload_len = buf[1] & 0x7F;
    int offset = 2;

    if (payload_len == 126)
    {
        if (len < 4)
            return std::nullopt;
        payload_len = (buf[2] << 8) | buf[3];
        offset += 2;
    }
    else if (payload_len == 127)
    {
        if (len < 10)
            return std::nullopt;
        payload_len = 0;
        for (int i = 0; i < 8; ++i)
        {
            payload_len = (payload_len << 8) | buf[2 + i];
        }
        offset += 8;
    }

    unsigned char mask_key[4] = {};
    if (mask)
    {
        if (len < offset + 4)
            return std::nullopt;
        memcpy(mask_key, buf + offset, 4);
        offset += 4;
    }

    if (len < offset + payload_len)
        return std::nullopt;

    std::string result;
    result.reserve(payload_len);
    for (uint64_t i = 0; i < payload_len; ++i)
    {
        char c = buffer[offset + i];
        if (mask)
            c ^= mask_key[i % 4];
        result.push_back(c);
    }

    return result;
}

void signalHandler(int signum = 0)
{
    std::cout << "\n(SIGINT 감지됨, 종료)\n";
    exit(signum);
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    if (fdwCtrlType == CTRL_C_EVENT)
    {
        std::cout << "\nCtrl+C 감지됨. 프로그램을 종료합니다.\n";
        ExitProcess(0);
        return TRUE;
    }
    return FALSE;
}

void render_ascii_from_grayscale_raw(const std::vector<unsigned char> &raw, int width, int height)
{
    const char *ramp = " .:-=+*#%@";
    for (int y = 0; y < height; y += 2)
    {
        for (int x = 0; x < width; x++)
        {
            unsigned char pixel = raw[y * width + x];
            char c = ramp[pixel * 9 / 255];
            std::cout << c;
        }
        std::cout << "\n";
    }
}

void openMessageBox(const std::string &title, const std::string &body)
{
    // 멀티바이트 문자열을 wide string으로 변환
    int lenMsg = MultiByteToWideChar(CP_UTF8, 0, body.c_str(), -1, nullptr, 0);
    std::wstring wMsg(lenMsg, 0);
    MultiByteToWideChar(CP_UTF8, 0, body.c_str(), -1, &wMsg[0], lenMsg);

    int lenTitle = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, nullptr, 0);
    std::wstring wTitle(lenTitle, 0);
    MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, &wTitle[0], lenTitle);

    MessageBoxW(NULL, wMsg.c_str(), wTitle.c_str(), MB_OK | MB_ICONINFORMATION);
}

std::pair<std::string, std::vector<std::string>> prettier(
    const std::vector<std::string> &keys,
    const std::vector<std::vector<std::string>> &values)
{
    std::vector<size_t> widths(keys.size());
    std::vector<std::string> rows;

    for (size_t i = 0; i < keys.size(); ++i)
        widths[i] = keys[i].size();

    for (const auto &row : values)
    {
        for (size_t i = 0; i < row.size(); ++i)
        {
            if (i < widths.size())
                widths[i] = std::max(widths[i], row[i].size());
        }
    }

    std::string header;
    for (size_t i = 0; i < keys.size(); ++i)
    {
        if (i > 0)
            header += " | ";
        header += std::format("{0:<{1}}", keys[i], widths[i]);
    }

    for (const auto &row : values)
    {
        std::string line;
        for (size_t i = 0; i < row.size(); ++i)
        {
            if (i > 0)
                line += " | ";
            line += std::format("{0:<{1}}", row[i], widths[i]);
        }
        rows.push_back(line);
    }

    return {header, rows};
}