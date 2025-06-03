// 콘솔 기반 채팅 프로그램 - 멀티 유저 입력 및 리더보드 지원
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

// 사용자 정보 구조체
struct User
{
    std::string nickname;
    int level = 1;
    int xp = 0;
};

std::unordered_map<std::string, User> users; // 닉네임 -> User 매핑

void addXP(User &user)
{
    user.xp++;
    int xpRequired = user.level * 10;
    if (user.xp >= xpRequired)
    {
        user.level++;
        user.xp = 0;
        std::cout << "[시스템] " << user.nickname << "님이 레벨업했습니다! 현재 레벨: " << user.level << "\n";
    }
}

void printMessage(const std::string &nickname, const std::string &message)
{
    const User &user = users[nickname];
    std::cout << "[Lv." << user.level << "] " << nickname << ": " << message << "\n";
}

void showLeaderboard()
{
    std::vector<User> ranking;
    for (const auto &[_, user] : users)
    {
        ranking.push_back(user);
    }
    std::sort(ranking.begin(), ranking.end(), [](const User &a, const User &b)
              {
        if (a.level != b.level) return a.level > b.level;
        if (a.xp != b.xp) return a.xp > b.xp;
        return a.nickname < b.nickname; });

    std::cout << "[리더보드 TOP 3]\n";
    for (int i = 0; i < std::min(3, (int)ranking.size()); ++i)
    {
        const User &user = ranking[i];
        std::cout << i + 1 << "위 - " << user.nickname << " (Lv." << user.level << ", " << user.xp << "XP)\n";
    }
}

int main()
{
    std::cout << "채팅을 시작합니다. 사용자는 '닉네임>메시지' 형식으로 입력하세요.\n";
    std::cout << "리더보드를 보려면 '/leaderboard'를 입력하세요. 종료하려면 '/exit'.\n";
    std::string line;
    while (true)
    {
        std::getline(std::cin, line);
        if (line == "/exit")
            break;
        if (line == "/leaderboard")
        {
            showLeaderboard();
            continue;
        }
        size_t sep = line.find('>');
        if (sep == std::string::npos)
        {
            std::cout << "입력 형식 오류: '닉네임>메시지' 형식으로 입력하세요.\n";
            continue;
        }
        std::string nickname = line.substr(0, sep);
        std::string message = line.substr(sep + 1);

        if (users.count(nickname) == 0)
        {
            users[nickname] = User{nickname};
        }
        addXP(users[nickname]);
        printMessage(nickname, message);
    }

    std::cout << "채팅을 종료합니다.\n";
    return 0;
}
