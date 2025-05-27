#include "util/common.h"
#include "util/tools.h"

int selector(const std::string &title, const std::vector<std::string> &msgs, const std::string &header = "")
{
    int currentIndex = 0;
    int prevIndex = -1;
    while (true)
    {
        if (prevIndex != currentIndex)
        {
            system("cls"); // 콘솔 클리어
            std::cout << rand() << title << " (↑/↓, Enter | ESC):\n\n";

            if (header.size())
            {
                std::cout << header << std::endl;
            }

            for (int i = 0; i < msgs.size(); ++i)
            {
                if (i == currentIndex)
                {
                    // 선택 항목에 색 입히기
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_BLUE | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                }

                std::cout << msgs[i] << std::endl;

                if (i == currentIndex)
                {
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // 초기화
                }
            }
            prevIndex = currentIndex;
        }

        int ch = _getch();
        if (ch == 224 || ch == 0)
        { // 방향키
            ch = _getch();
            if (ch == 72 && currentIndex > 0)
            { // ↑
                prevIndex = currentIndex--;
            }
            else if (ch == 80 && currentIndex < msgs.size() - 1)
            { // ↓
                prevIndex = currentIndex++;
            }
        }
        else if (ch == 13)
        { // Enter
            std::cout << std::flush;
            return currentIndex;
        }
        else if (ch == 8 || ch == 27)
        { // 8: backspace, 27: esc
            std::cout << "사용자가 취소했습니다." << std::endl;
            return -1;
        }
        else if (ch == 3)
        {
            signalHandler();
        }
    }
}