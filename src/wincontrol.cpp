#include "wincontrol.hpp"
#include <windows.h>

HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

namespace win_control
{
    void setColor(unsigned short fColor, unsigned short bColor)
    {
        SetConsoleTextAttribute(hOut, fColor | bColor << 4);
    }
    void goxy(unsigned short x, unsigned short y)
    {
        COORD pos = {y, x};
        SetConsoleCursorPosition(hOut, pos);
    }
    void hideCursor()
    {
        CONSOLE_CURSOR_INFO cci;
        GetConsoleCursorInfo(hOut, &cci);
        cci.bVisible = false;
        SetConsoleCursorInfo(hOut, &cci);
    }
    void consoleInit()
    {

        SetConsoleCP(65001);
        SetConsoleTitle(L"Explode Chess");
        hideCursor();
    }
    void cls()
    {
    }
}