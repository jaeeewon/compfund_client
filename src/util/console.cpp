#include <util/common.h>

void moveCursorTo(int x, int y)
{
    COORD pos = {static_cast<SHORT>(x), static_cast<SHORT>(y)};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void clearCurrentLine()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
        return;

    COORD startOfLine = {0, csbi.dwCursorPosition.Y};
    DWORD cellsWritten;
    DWORD width = csbi.dwSize.X;

    // 현재 줄을 공백으로 채움
    FillConsoleOutputCharacter(hConsole, ' ', width, startOfLine, &cellsWritten);

    // 커서 x=0으로 이동
    SetConsoleCursorPosition(hConsole, startOfLine);
}

void moveCursorHome()
{
    std::cout << "\033[H"; // 0,0으로 이동
    std::cout.flush();
}

void moveCursorUp(int lines)
{
    std::cout << "\033[" << lines << "A";
}

void moveCursorDown(int lines)
{
    std::cout << "\033[" << lines << "B";
}

void moveCursorToStart()
{
    std::cout << "\r"; // 줄 맨 앞으로 이동
}

void moveCursorToBottom()
{
    std::cout << "\033[999B"; // 가능한 가장 아래로 이동
    std::cout.flush();
}

void clearConsole()
{
    std::cout << "\033[2J\033[H"; // 화면 전체 지우고 커서를 (0,0)으로 이동
    std::cout.flush();
}

bool isCrazyConsole()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        // 실패하면 아예 콘솔 아님
        return true;
    }

    // 커서의 Y 좌표 확인
    SHORT y = csbi.dwCursorPosition.Y;
    return y == 0; // 항상 0이면 이상한 콘솔일 가능성 높음
}