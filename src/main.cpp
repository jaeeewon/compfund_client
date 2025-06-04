#include "util/common.h"
#include "util/tools.h"
#include "util/arrowselector.h"
#include "util/global.h"
#include "util/console.h"
#include "chat/room.h"
#include "network/websocket.h"

int main(int argc, char *argv[])
{
    if (isCrazyConsole() && argc == 1)
    {
        // 현재 실행 파일 경로 구하기
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);

        std::string command = "start cmd /k \"" + std::string(exePath) + " run\"";
        system(command.c_str());
        return 0;
    }
    std::signal(SIGINT, signalHandler);
    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    SOCKET sock = initializeSocket();

    send_ws_message(sock, "login", json{});

    std::thread recv_thread(receive_loop, sock, std::ref(shared));

    while (!shared.isLoggedIn)
    {
        if (shared.user.ticket != "")
        {
            json ticket_chk;
            ticket_chk["ticket"] = shared.user.ticket;
            send_ws_message(sock, "ticket-check", ticket_chk);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    std::cout << "로그인 완료!" << std::endl;

    std::string message = "/exit 종료     /my 내 정보     /rooms 참여한 방 목록\n/create 방 만들기    /room-list 참여 가능한 방 목록\n/status   모든 참여자의 상태 확인     /status <상태>    상태 메시지 설정\n/nickname <닉네임>  닉네임 설정";

    std::string input;
    while (true)
    {
        std::cout << message << std::endl;
        std::cout << "\n> ";
        if (!std::getline(std::cin, input))
        {
            break;
        }

        if (input == "/exit")
            break;

        if (input == "/my")
        {
            std::cout << "내 정보 확인하기" << std::endl;
            std::cout << "이메일 | " << shared.user.email << std::endl;
            std::cout << "이름 | " << shared.user.name << std::endl;
            std::cout << "닉네임 | " << shared.user.nickname << std::endl;
            std::cout << "프로필사진" << std::endl;
            render_ascii_from_grayscale_raw(base64_decode(shared.user.picture), 80, 80);
        }
        else if (input == "/create")
        {
            // 입력받고

            std::string roomName;
            std::string description;
            std::cout << "생성할 방 이름 > ";
            std::getline(std::cin, roomName);
            std::cout << "생성할 방 설명 > ";
            std::getline(std::cin, description);

            json data;
            data["userId"] = shared.user.id;
            data["detail"]["roomName"] = roomName;
            data["detail"]["description"] = description;
            send_ws_message(sock, "create-room", data);
        }
        else if (input == "/rooms")
        {
            {
                std::lock_guard<std::mutex> lock(shared.mutex);
                if (shared.rooms.size() == 0)
                {
                    std::cout << "아직 참여한 방이 없습니다!" << std::endl;
                    continue;
                }

                std::vector<std::vector<std::string>> v;
                for (const auto &[roomName, room] : shared.rooms)
                {
                    std::string pts;
                    std::vector<std::string> nicknames;
                    for (const auto &p : room.participants)
                    {
                        auto it = ptstate.participants.find(p.first);
                        if (it != ptstate.participants.end())
                            nicknames.push_back(it->second.nickname);
                    }

                    if (!nicknames.empty())
                    {
                        pts += " ";
                        for (auto i = 0; i < nicknames.size(); ++i)
                        {
                            pts += nicknames[i];
                            if (i != nicknames.size() - 1)
                                pts += " | ";
                        }
                    }

                    v.push_back({room.roomName, room.description, room.latestChat, pts});
                }
                auto [header, rooms] = prettier({"방 제목", "방 설명", "최근 채팅 시간", "참여자"}, v);

                int selected = selector("방을 선택하세요", rooms, header);

                if (selected == -1)
                {
                    // input = "/back";
                    continue;
                }

                // std::string selectedRoom = std::next(shared.rooms.begin(), selected)->first;
                shared.currentRoom = &std::next(shared.rooms.begin(), selected)->second;
            }
            loadChats();
            moveToRoom(shared, input, sock);
        }
        else if (input == "/room-list")
        {
            handleRoomList();
        }
        else if (input == "/status")
        {
            std::lock_guard<std::mutex> lock(shared.mutex);
            std::lock_guard<std::mutex> ptlock(ptstate.mutex);
            // unordered_map의 순회는 나쁜 걸 너무 잘 알지만
            // 기말고사 2주 전에 구조 바꾸는 것 보다는,,
            system("cls");
            std::cout << "현재 참여중인 방의 가입자만 보여집니다.\n"
                      << std::endl;
            std::cout << std::format("(나) {} | {}", shared.user.nickname, shared.user.status.size() ? shared.user.status : "<설정되지 않음>") << std::endl;
            for (const auto &[id, pt] : ptstate.participants)
                if (id != shared.user.id)
                    std::cout << std::format("{} | {}", pt.nickname, pt.status.size() ? pt.status : "<설정되지 않음>") << '\n';
            std::cout << std::string(50, '=') << '\n'
                      << std::endl;
        }
        else if (input.starts_with("/status "))
        {
            std::string newStatus = input.substr(8); // "/status " 이후 문자열
            if (newStatus.size() == 0)
                continue;

            std::lock_guard<std::mutex> lock(shared.mutex);
            json data;
            data["userId"] = shared.user.id;
            data["detail"]["status"] = newStatus;
            send_ws_message(sock, "update-status", data);
        }
        else if (input.starts_with("/nickname "))
        {
            std::string nickname = input.substr(10); // "/nickname " 이후 문자열
            if (nickname.size() == 0)
                continue;

            std::lock_guard<std::mutex> lock(shared.mutex);
            json data;
            data["userId"] = shared.user.id;
            data["detail"]["nickname"] = nickname;
            send_ws_message(sock, "update-nickname", data);
        }
        else
        {
            moveCursorUp(1);
            clearCurrentLine();
            moveCursorUp(1);
        }
    }

    shutdown(sock, SD_BOTH); // 소켓에 셧다운 전달하고
    recv_thread.join();      // 스레드 종료시킨 후
    closesocket(sock);       // 소켓 닫기기
    WSACleanup();
    return 0;
}
