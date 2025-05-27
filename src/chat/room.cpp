#include "util/global.h"
#include "util/console.h"
#include "util/tools.h"
#include "util/arrowselector.h"
#include "network/websocket.h"

Participant getParticipant(const std::string &id)
{
    // mutex assertion
    return ptstate.participants[id];
}

void moveToRoom(SharedState &state, std::string &input, SOCKET sock)
{
    int scrollIndex = 0;
    int prevScrollIndex = -1;
    int lastPrinted = -1;

    std::vector<Chat> chatsCopy;
    std::string roomName, roomId, userId;
    std::string inputBuffer = "";
    std::string prevInputBuffer = " ";
    int baseY = 1;

    {
        std::lock_guard<std::mutex> lock(state.mutex);
        roomName = state.currentRoom->roomName;
        chatsCopy = state.currentRoom->chats;
        userId = state.user.id;
        roomId = state.currentRoom->roomId;
    }

    clearConsole();
    moveCursorTo(0, 0);
    std::cout << "채팅방 이름: " << roomName << std::string(50, ' ') << "\n";

    int guideY = baseY + (int)chatsCopy.size();

    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            chatsCopy = state.currentRoom->chats;
        }

        // 새로운 채팅이 생긴 경우에만 출력
        if ((int)chatsCopy.size() > lastPrinted)
        {
            std::lock_guard<std::mutex> lock(ptstate.mutex);
            for (int i = std::max(0, lastPrinted); i < chatsCopy.size(); ++i)
            {
                const Participant &usr = ptstate.participants[chatsCopy[i].userId];
                moveCursorTo(0, baseY + i);
                std::cout << usr.nickname << " | " << chatsCopy[i].text
                          << std::string(50, ' ') << "\n";
            }

            guideY = baseY + (int)chatsCopy.size();
            moveCursorTo(0, guideY);
            std::cout << "=== /back '뒤로 가기'   /send <메시지> '메시지 보내기'    ctrl '정보 보기' ==="
                      << std::string(20, ' ') << "\n"
                      << std::string(50, ' ');

            lastPrinted = (int)chatsCopy.size();
        }

        // 하이라이트 위치 변경 감지
        if (scrollIndex != prevScrollIndex)
        {
            std::lock_guard<std::mutex> lock(ptstate.mutex);
            // 이전 줄 배경색 제거
            if (prevScrollIndex >= 0 && prevScrollIndex < chatsCopy.size())
            {
                moveCursorTo(0, baseY + prevScrollIndex);
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                std::cout << getParticipant(chatsCopy[prevScrollIndex].userId).nickname << " | " << chatsCopy[prevScrollIndex].text
                          << std::string(50, ' ') << "\n";
            }

            // 현재 줄 하이라이트
            if (scrollIndex >= 0 && scrollIndex < chatsCopy.size())
            {
                moveCursorTo(0, baseY + scrollIndex);
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                        BACKGROUND_BLUE | BACKGROUND_INTENSITY |
                                            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                std::cout << getParticipant(chatsCopy[scrollIndex].userId).nickname << " | " << chatsCopy[scrollIndex].text
                          << std::string(50, ' ') << "\n";
            }

            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            prevScrollIndex = scrollIndex;
        }

        // Ctrl 눌렀을 때 디테일 출력
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && scrollIndex < chatsCopy.size())
        {
            const auto &chat = chatsCopy[scrollIndex];
            moveCursorTo(0, guideY + 1);
            std::lock_guard<std::mutex> lock(ptstate.mutex);
            const Participant &usr = ptstate.participants[chat.userId];

            std::time_t unix_timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            std::cout << "[채팅 디테일] 닉네임: " << usr.nickname
                      << " | 최근 활동: " << (unix_timestamp - usr.latest_access) / 1000 / 60 << "분 전"
                      << " | 전송: " << chat.createdAt
                      << " | 내용: " << chat.text
                      << std::string(30, ' ') << "\n";
        }

        // // 입력창 출력
        // moveCursorTo(0, guideY + 2);
        // std::cout << "> " << inputBuffer << std::flush;

        std::string prompt = "> " + inputBuffer;

        if (inputBuffer != prevInputBuffer)
        {
            // 줄 전체 지우고 다시 출력
            moveCursorTo(0, guideY + 2);
            std::cout << prompt << std::string(50 - prompt.size(), ' ') << std::flush;

            prevInputBuffer = inputBuffer;
        }
        // 커서를 문자열 끝으로 이동
        moveCursorTo((int)prompt.size(), guideY + 2);

        if (inputBuffer.starts_with("/send "))
        {
            std::string message;
            if (std::getline(std::cin, message))
            {
                json data;
                data["userId"] = userId;
                data["detail"]["roomId"] = roomId;
                data["detail"]["message"] = message;
                send_ws_message(sock, "send-message", data);
            }
            inputBuffer.clear();
            continue;
        }

        // 키 입력 처리
        if (_kbhit())
        {
            int ch = _getch();
            if (ch == 224 || ch == 0)
            {
                int key = _getch();
                if (key == 72 && scrollIndex > 0) // ↑
                    scrollIndex--;
                else if (key == 80 && scrollIndex < (int)chatsCopy.size() - 1) // ↓
                    scrollIndex++;
            }
            else if (ch == 13) // Enter
            {
                input = inputBuffer;

                if (input == "/back")
                {
                    system("cls");
                    break;
                }

                inputBuffer.clear();
            }
            else if (ch == 8 && !inputBuffer.empty()) // Backspace
            {
                inputBuffer.pop_back();
            }
            else if (ch >= 32 && ch <= 126) // 일반 문자
            {
                inputBuffer += static_cast<char>(ch);
            }
            else if (ch == 27)
            {
                // 27: esc
                // input = "/back";
                system("cls");
                break;
            }
            else if (ch == 3)
            {
                signalHandler();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::lock_guard<std::mutex> lock(shared.mutex);
    state.currentRoom = nullptr;
}

void handleRoomList()
{
    json data;
    {
        std::lock_guard<std::mutex> lock(shared.mutex);
        data["userId"] = shared.user.id;
    }
    send_ws_message(sock, "get-room-list", data);
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(visiting.mutex);
            if (visiting.updated)
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    std::vector<std::vector<std::string>> v;
    for (const auto &room : visiting.rooms)
    {
        v.push_back({room.roomName, room.description, room.latestChat});
    }
    auto [header, rooms] = prettier({"방 제목", "방 설명", "최근 채팅 시간"}, v);

    {
        std::lock_guard<std::mutex> lock(visiting.mutex);
        visiting.updated = false;
        visiting.rooms.clear();
    }

    int selected = selector("등록할 방을 선택해주세요", rooms, header);
    if (selected == -1)
        return;

    data.clear();
    data["userId"] = shared.user.id;
    data["detail"]["roomId"] = visiting.rooms[selected].roomId;
    send_ws_message(sock, "join-room", data);

    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(join_room.mutex);
            if (join_room.updated)
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    std::cout << "해당 채팅방에 참여했습니다." << std::endl;

    {
        std::lock_guard<std::mutex> lock(join_room.mutex);
        join_room.updated = false;
    }
}

void loadChats()
{
    system("cls");
    std::cout << "채팅 기록을 불러오는 중..." << std::endl;
    json data;
    {
        std::lock_guard<std::mutex> lock(shared.mutex);
        data["roomId"] = shared.currentRoom->roomId;
    }
    send_ws_message(sock, "get-chat-list", data);
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(load_chat.mutex);
            if (load_chat.updated)
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    std::lock_guard<std::mutex> lock(load_chat.mutex);
    load_chat.updated = false;
}