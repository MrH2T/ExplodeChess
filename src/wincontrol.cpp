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
    void sleep(int tm)
    {
        Sleep(tm);
    }

    void cls()
    {
    }
    namespace input_record
    {
        CONSOLE_SCREEN_BUFFER_INFO bInfo;
        INPUT_RECORD ms_rec;
        DWORD ms_res;
        void getInput()
        {
            ReadConsoleInput(hIn, &ms_rec, 1, &ms_res);
            if (ms_rec.EventType == KEY_EVENT)
            {
                if (ms_rec.Event.KeyEvent.bKeyDown)
                {
                    win_control::input_record::keyHandler(ms_rec.Event.KeyEvent.wVirtualKeyCode);
                }
            }
        }
    }
}