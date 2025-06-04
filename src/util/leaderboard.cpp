#include "util/common.h"
#include "util/global.h"
#include "network/websocket.h"

void loadLeaderboard()
{
    system("cls");
    std::cout << "리더보드를 불러오는 중..." << std::endl;
    send_ws_message(sock, "get-leaderboard", json{});
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(load_leaderboard.mutex);
            if (load_leaderboard.updated)
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    std::lock_guard<std::mutex> lock(load_leaderboard.mutex);
    load_leaderboard.updated = false;
}