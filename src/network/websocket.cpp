#include "util/common.h"
#include "util/tools.h"
#include "util/global.h"
#include "network/handler.h"

void send_ws_message(SOCKET sock, const std::string &type, const json &data)
{
    json message;
    message["type"] = type;
    message["data"] = data;

    std::string payload = message.dump();
    std::vector<unsigned char> frame;
    frame.push_back(0x81);

    if (payload.size() <= 125)
    {
        frame.push_back(0x80 | payload.size());
    }
    else if (payload.size() <= 65535)
    {
        frame.push_back(0x80 | 126); // 126을 명시
        frame.push_back((payload.size() >> 8) & 0xFF);
        frame.push_back(payload.size() & 0xFF);
    }
    else
    {
        frame.push_back(0x80 | 127);
        for (int i = 7; i >= 0; --i)
        {
            frame.push_back((payload.size() >> (8 * i)) & 0xFF);
        }
    }

    unsigned char mask[4] = {0x12, 0x34, 0x56, 0x78};
    frame.insert(frame.end(), mask, mask + 4);

    for (size_t i = 0; i < payload.size(); ++i)
        frame.push_back(payload[i] ^ mask[i % 4]);

    send(sock, (const char *)frame.data(), frame.size(), 0);
}

void wsAssertionThread()
{
    while (true)
    {
        // websocket assertion
        json data;
        data["type"] = "ping";
        send_ws_message(sock, "ping", data);
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }
}

void readHandlingThread()
{
    while (true)
    {
        std::string roomId;
        std::string userId;
        bool send = false;

        {
            std::lock_guard<std::mutex> lock(shared.mutex);
            if (shared.currentRoom != nullptr)
            {
                roomId = shared.currentRoom->roomId;
                userId = shared.user.id;
                send = true;
            }
        }

        if (send)
        {
            json data;
            data["userId"] = userId;
            data["detail"]["roomId"] = roomId;

            send_ws_message(sock, "read-chat", data);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

SOCKET initializeSocket()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    addrinfo hints = {}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    bool isDev = false;
    std::string HOST = isDev ? "chuncheon" : "comp-fund.jae.one";
    std::string PORT = isDev ? "2101" : "80";
    getaddrinfo(HOST.c_str(), PORT.c_str(), &hints, &res);

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(sock, res->ai_addr, (int)res->ai_addrlen);

    freeaddrinfo(res);

    std::string ws_key = generate_ws_key();
    std::stringstream req;
    req << "GET /api/ws HTTP/1.1\r\n"
        << "Host: "
        << HOST
        << "\r\n"
        << "Upgrade: websocket\r\n"
        << "Connection: Upgrade\r\n"
        << "Sec-WebSocket-Key: " << ws_key << "\r\n"
        << "Sec-WebSocket-Version: 13\r\n\r\n";

    send(sock, req.str().c_str(), (int)req.str().size(), 0);
    char buffer[2048];
    int len = recv(sock, buffer, sizeof(buffer) - 1, 0);
    buffer[len] = 0;
    // std::cout << "[Handshake 응답]\n"
    //           << buffer << "\n";

    std::thread(wsAssertionThread)
        .detach();

    std::thread(readHandlingThread).detach();

    return sock;
}

void receive_loop(SOCKET sock, SharedState &state)
{
    std::string recv_buffer;
    char buffer[4096]; // 더 크게

    while (true)
    {
        int len = recv(sock, buffer, sizeof(buffer), 0);
        if (len <= 0)
            break;

        recv_buffer.append(buffer, len);

        // 반복적으로 프레임 파싱 시도
        while (true)
        {
            auto result = decode_websocket_payload_safe(recv_buffer.data(), recv_buffer.size());
            if (!result.has_value())
                break;

            // 한 프레임 성공적으로 파싱되었으면
            size_t frame_len = 0;

            // payload 길이 계산
            const unsigned char *buf = (const unsigned char *)recv_buffer.data();
            uint64_t payload_len = buf[1] & 0x7F;
            int offset = 2;
            if (payload_len == 126)
            {
                payload_len = (buf[2] << 8) | buf[3];
                offset += 2;
            }
            else if (payload_len == 127)
            {
                payload_len = 0;
                for (int i = 0; i < 8; ++i)
                {
                    payload_len = (payload_len << 8) | buf[2 + i];
                }
                offset += 8;
            }

            bool mask = (buf[1] & 0x80);
            if (mask)
                offset += 4;

            frame_len = offset + payload_len;

            std::string raw = result.value();
            recv_buffer.erase(0, frame_len); // consume

            handleWebsocket(raw, state);
        }
    }
}