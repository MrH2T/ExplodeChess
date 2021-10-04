#include "wincontrol.hpp"
#include <windows.h>

HANDLE hIn=GetStdHandle(STD_INPUT_HANDLE);
HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);


void win_control::setColor(unsigned short fColor,unsigned short bColor){
    SetConsoleTextAttribute(hOut,fColor|bColor<<4);
}
void win_control::goxy(unsigned short x,unsigned short y){
    COORD pos={y,x};
    SetConsoleCursorPosition(hOut,pos);
}
void win_control::hideCursor(){
    CONSOLE_CURSOR_INFO cci;
    GetConsoleCursorInfo(hOut,&cci);
    cci.bVisible=false;
    SetConsoleCursorInfo(hOut,&cci);
}
void win_control::consoleInit(){

    SetConsoleCP(65001);
    SetConsoleTitle(L"Explode Chess");
    win_control::hideCursor();
    
}
void cls(){

}