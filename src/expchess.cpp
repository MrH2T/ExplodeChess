#include <iostream>
#include <cmath>
#include <iomanip>
#include <queue>
#include <thread>
#include <array>
#include "wincontrol.hpp"
#include "asio.hpp"

void win_control::cls()
{
    win_control::setColor(win_control::Color::c_BLACK,win_control::Color::c_BLACK);
    win_control::goxy(0, 0);
    for (short i = 0; i < 40; i++)
    {
        std::cout << "                                                                                \n";
    }
    win_control::goxy(0, 0);
}

namespace Game
{
    volatile bool onGameRunning;

    bool painting;

    enum GameType
    {
        INET_HOST,
        INET_GUEST,
        LOCAL
    } gType;

    namespace References
    {
        const size_t MAX_CLIENT_COUNT = 10;
        const int PORT = 14514;
        const int MAX_MAP_SIZE = 15;
        const std::string EMPTYLINE = "                                                              ";
    }

    namespace Debug
    {
        const bool DEBUG = false;
        const short DBG_X[3]={20,25,30}, DBG_Y = 0, DBG_X2 = 30;
        void dprint(char* s,int pos=0){
            if(!DEBUG)return;
            while(painting);
            painting=true;
            win_control::goxy(DBG_X[pos],DBG_Y);
            std::cout<<s;

            painting=false;
        }
    }

    win_control::Color pColor[References::MAX_CLIENT_COUNT + 1] = {
        win_control::Color::c_DGREY,
        win_control::Color::c_RED,
        win_control::Color::c_DARKBLUE,
        win_control::Color::c_LYELLOW,
        win_control::Color::c_DGREEN,
        win_control::Color::c_LIME,
        win_control::Color::c_SKYBLUE,
        win_control::Color::c_LIGHTBLUE,
        win_control::Color::c_DPURPLE,
        win_control::Color::c_PURPLE,
        win_control::Color::c_DRED,
    };
    int playerCount = 0, playerAlive = 0;
    int mapSize = 0, map[References::MAX_MAP_SIZE][References::MAX_MAP_SIZE],
        cmap[References::MAX_MAP_SIZE][References::MAX_MAP_SIZE];
    bool playerDied[References::MAX_CLIENT_COUNT], initialed[References::MAX_CLIENT_COUNT];
    int blocks[References::MAX_CLIENT_COUNT];
    std::string usrNames[References::MAX_CLIENT_COUNT];

    COORD choosingPosition = {0, 0};
    int nowTurn, usercol;

    void nextPlayer()
    {
        nowTurn = nowTurn % playerCount + 1;
        while (playerDied[nowTurn])
        {
            nowTurn = nowTurn % playerCount + 1;
        }
    }

    int fullAtPos(int x, int y)
    {
        int res = 4;
        if (x == 0 || x == mapSize - 1)
            res--;
        if (y == 0 || y == mapSize - 1)
            res--;
        return res;
    }

    namespace Network
    {
        asio::io_context io_context;

        asio::ip::tcp::socket soc(io_context);

        asio::ip::tcp::acceptor ac(io_context);
        struct Client
        {
            asio::ip::tcp::socket sock;
            std::string username;
            Client() : sock(io_context), username("") {}
        } clients[Game::References::MAX_CLIENT_COUNT];
        int clientConnected = 0;

        void networkInit()
        {
        }
        void clientStartup(std::string ipAddress, bool localHost = 0)
        {
            asio::connect(
                soc,
                asio::ip::tcp::resolver(io_context).resolve(ipAddress, std::to_string(References::PORT)));
        }
        void serverStartup(int port)
        {
            ac = asio::ip::tcp::acceptor(
                io_context,
                asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
            clientConnected = 0;
        }
        void sendMessage(asio::ip::tcp::socket &soc, const std::string &msg)
        {
            // win_control::goxy(20, 0);
            // std::cout << "Message as argument : " << msg << std::endl; //1
            try
            {
                asio::write(soc, asio::buffer(msg));
            }
            catch (std::exception &e)
            {
                std::cout << e.what() << std::endl;
            }
        }
        void sendToAll(std::string msg)
        {
            if (gType != INET_HOST)
                return;
            // win_control::goxy(23, 0);
            // std::cout << "Message in sendToAll() : " << msg << "   " << std::endl;
            for (int i = 1; i <= clientConnected; i++)
            {
                sendMessage(clients[i].sock, msg);
            }
        }
    }
    namespace Thread
    {
        std::thread receiveThread, toClient[References::MAX_CLIENT_COUNT];
        void joinClientThreads(){
            return;
            if (Game::gType == Game::INET_HOST)
            {
                try{
                for (int i = 1; i <= Game::Network::clientConnected; i++)
                    if (Game::Thread::toClient[i].joinable())
                        Game::Thread::toClient[i].join();
                }catch(...){
                    std::cout<<"Error on joining toClient Thread!\n";
                }
            }
        }
        void joinServerThread()
        {
            return;
            try{
            if (Game::Thread::receiveThread.joinable())
                Game::Thread::receiveThread.join();
            }catch(...){
                std::cout<<"Cannot join receiveThread!\n";
            }
            
        }
    }
    void beGuest()
    {
        //not final version
        win_control::goxy(0,0);
        gType = INET_GUEST;
        std::cout << "IP Address: ";
        std::string ipAddress;
        std::cin >> ipAddress;
        std::cout << "Your user name: ";
        std::string username;
        std::cin >> username;
        try
        {
            Network::clientStartup(ipAddress);
            Network::sendMessage(Network::soc, username);
        }
        catch (...)
        {
            exit(1);
        }

        win_control::setTitle(L"Explode Chess - Waiting");
    }
    void beHost()
    {
        gType = INET_HOST;
        win_control::goxy(0,0);
        Network::serverStartup(References::PORT);
        while (Game::playerCount < 2 || Game::playerCount > 10)
        {
            win_control::goxy(0, 0);
            std::cout << Game::References::EMPTYLINE;
            win_control::goxy(0, 0);
            std::cout << "Player Count (2-10): ";
            std::cin >> Game::playerCount;
        }
        while (Game::mapSize < 5 || Game::mapSize > 15)
        {
            win_control::goxy(1, 0);
            std::cout << Game::References::EMPTYLINE;
            win_control::goxy(1, 0);
            std::cout << "Map Size (5-15): ";
            std::cin >> Game::mapSize;
        }

        std::string username = "";
        std::cout << "Your user name: ";
        std::cin >> username;
        Network::clientStartup("127.0.0.1", 1);
        Network::sendMessage(Network::soc, username);

        win_control::goxy(0, 0);
        std::cout
            << Game::References::EMPTYLINE << std::endl
            << Game::References::EMPTYLINE << std::endl
            << Game::References::EMPTYLINE << std::endl
            << Game::References::EMPTYLINE << std::endl
            << Game::References::EMPTYLINE << std::endl
            << Game::References::EMPTYLINE << std::endl;

        win_control::setTitle(L"Explode Chess - Waiting");
    }
    namespace App
    {
        enum State
        {
            MainFrame,
            Waiting,
            Gaming
        } gameState;

    }
    //maybe will be unused
    void onHostAccept(asio::error_code err, asio::ip::tcp::socket sc)
    {
        Network::Client &cl = Network::clients[++Network::clientConnected];
        cl.sock = std::move(sc);
        std::array<char, 65536> tmp;
        size_t len = cl.sock.read_some(asio::buffer(tmp), err);
        cl.username = tmp.data();
        cl.username.resize(len);
        if (err && err != asio::error::eof)
            throw asio::system_error(err);

        win_control::goxy(2, 0);
        std::cout << "Now players: " << Game::Network::clientConnected << std::endl;
        for (int i = 1; i <= Game::Network::clientConnected; i++)
            std::cout << "User #" << i << ": " << Game::Network::clients[i].username << std::endl;

        if (Game::Network::clientConnected == Game::playerCount)
        {

            std::cout << "All players get ready. Game starts in 5 seconds!" << std::endl;
            win_control::sleep(5000);

            Game::Network::sendToAll("ec$gamestart");
            Game::App::gameState = Game::App::Gaming;
            win_control::cls();
            win_control::setTitle(L"Explode Chess - Gaming");
        }
    }

    void guestSolveGameInfo(std::string info)
    {

        int num[3] = {0};
        int i, id;
        for (i = 0, id = 0; i < info.size() && id <= 2; i++)
        {
            if (info[i] == '$')
            {
                id++;
                continue;
            }
            num[id] = num[id] * 10 + info[i] - '0';
        }
        //    win_control::goxy(15, 0);
        //     std::cout << num[0] << " " << num[1] << std::endl;
        Game::playerCount = num[0];
        Game::mapSize = num[1];
        Game::usercol = num[2];

        for (id = 1; id <= playerCount && i < info.size(); i++)
        {
            if (info[i] == '$')
            {
                usrNames[id] += '\000';
                id++;
                continue;
            }
            usrNames[id] += info[i];
        }
    }
    void drawBlockBorder(int x, int y, win_control::Color col)
    {
        while (painting)
            ;
        painting = true;
        int fx = x * 2, fy = y * 4;

        win_control::setColor(col, win_control::Color::c_GREY);
        win_control::goxy(fx, fy);
        std::cout << "+---+";
        win_control::goxy(fx + 1, fy);
        std::cout << "|";
        win_control::goxy(fx + 1, fy + 4);
        std::cout << "|";
        win_control::goxy(fx + 2, fy);
        std::cout << "+---+";
        win_control::setColor(win_control::Color::c_BLACK, win_control::Color::c_GREY);
        painting = false;
    }
    void drawChoosingBlock()
    {
        drawBlockBorder(choosingPosition.X, choosingPosition.Y, pColor[usercol]);
    }
    void drawMapItem(int x, int y)
    {
        while (painting)
            ;
        painting = true;
        win_control::goxy(2 * x + 1, 4 * y + 1);
        win_control::setColor(pColor[cmap[x][y]], win_control::Color::c_GREY);
        std::cout << map[x][y] << "/" << fullAtPos(x, y);
        win_control::setColor(win_control::Color::c_BLACK, win_control::Color::c_GREY);
        painting = false;
    }
    void drawMap()
    {
        while (painting)
            ;
        painting = true;
        win_control::setColor(win_control::Color::c_BLACK, win_control::Color::c_GREY);
        win_control::goxy(0, 0);
        std::cout << "+";
        for (short i = 0; i < mapSize; i++)
            std::cout << "---+";
        std::cout << std::endl;
        for (short i = 0; i < mapSize; i++)
        {
            win_control::setColor(win_control::Color::c_BLACK, win_control::Color::c_GREY);
            std::cout << "|";
            for (short j = 0; j < mapSize; j++)
            {
                win_control::setColor(pColor[cmap[i][j]], win_control::Color::c_GREY);
                std::cout << map[i][j] << "/" << fullAtPos(i, j);
                win_control::setColor(win_control::Color::c_BLACK, win_control::Color::c_GREY);
                std::cout << "|";
            }
            std::cout << std::endl;
            win_control::setColor(win_control::Color::c_BLACK, win_control::Color::c_GREY);
            std::cout << "+";
            for (short j = 0; j < mapSize; j++)
            {
                std::cout << "---+";
            }
            std::cout << std::endl;
        }
        painting = false;
    }

    void drawUserTable()
    {
        while (painting)
            ;
        painting = true;
        for (int i = 1; i <= Game::playerCount; i++)
        {
            win_control::goxy(4 + i, 63);
            if (i == nowTurn)
            {
                win_control::setColor(pColor[nowTurn], win_control::Color::c_BLACK);
                std::cout << ">";
                win_control::setColor(win_control::Color::c_BLACK, win_control::Color::c_BLACK);
                std::cout << " ";
            }
            else
            {
                win_control::setColor(win_control::Color::c_BLACK, win_control::Color::c_BLACK);
                std::cout << "  ";
            }
            if (playerDied[i])
                win_control::setColor(win_control::Color::c_GREY, win_control::Color::c_BLACK);
            else
                win_control::setColor(pColor[i], win_control::Color::c_BLACK);
            std::cout << std::setw(3) << blocks[i] << " " << Game::usrNames[i];
        }
        painting = false;
    }
    void drawOneUser(int usr)
    {
        while (painting)
            ;
        painting = true;
        win_control::goxy(4 + usr, 65);
        if (playerDied[usr])
            win_control::setColor(win_control::Color::c_GREY, win_control::Color::c_BLACK);
        else
            win_control::setColor(pColor[usr], win_control::Color::c_BLACK);
        std::cout << blocks[usr] << " " << Network::clients[usr].username;
        painting = false;
    }
    void drawCombo(int combo, win_control::Color col)
    {
        while (painting)
            ;

        painting = true;
        win_control::goxy(2, 63);
        win_control::setColor(col, win_control::Color::c_BLACK);
        if (combo == -1)
            std::cout << "            ";
        else
            std::cout << "Combo: " << combo;
        painting = false;
    }

    namespace Explosion
    {
        const int dir[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
        std::queue<std::pair<int, int>> qu;
    }

    void gameEnd(){
        Debug::dprint("Client Gameend\n",2);
        Network::sendMessage(Network::soc, "ec$gameend");
        drawUserTable();
        if(Debug::DEBUG){
            win_control::sleep(1000);
            // Debug::dprint(std::string(Thread::receiveThread.));
            while(1);
        }
        win_control::sleep(2000);
        win_control::cls();
        win_control::setColor(win_control::Color::c_WHITE,win_control::Color::c_BLACK);
        onGameRunning = false;
        Network::io_context.stop();
        try{
            Thread::joinServerThread();
            Thread::joinClientThreads();
            exit(0);
        }
        catch(std::exception &e){
            std::cout<<e.what()<<std::endl;
            while(1);
            exit(0);
        }
    }

    void checkExplode(int targetColor, int x, int y)
    {
        while (!Explosion::qu.empty())
            Explosion::qu.pop();
        if (map[x][y] == fullAtPos(x, y))
            Explosion::qu.push(std::make_pair(x, y));
        int combo = 0;

        while (!Explosion::qu.empty())
        {
            std::pair<int, int> hd = Explosion::qu.front();
            Explosion::qu.pop();
            combo++;
            map[hd.first][hd.second] = 0;
            cmap[hd.first][hd.second] = 0;
            blocks[nowTurn]--;
            for (int di = 0; di < 4; di++)
            {
                int dx = hd.first + Explosion::dir[di][0], dy = hd.second + Explosion::dir[di][1];
                if (dx < 0 || dx >= mapSize || dy < 0 || dy >= mapSize)
                    continue;
                if (cmap[dx][dy] != targetColor)
                    blocks[nowTurn]++;
                if (cmap[dx][dy] && targetColor != cmap[dx][dy])
                {
                    blocks[cmap[dx][dy]]--;
                    if (!blocks[cmap[dx][dy]])
                    {
                        playerDied[cmap[dx][dy]] = 1;
                        playerAlive--;
                        if (playerAlive == 1)
                        {
                            cmap[dx][dy] = targetColor;
                            map[dx][dy]++;
                            drawMapItem(hd.first, hd.second);
                            for (int di = 0; di < 4; di++)
                            {
                                int dx = hd.first + Explosion::dir[di][0], dy = hd.second + Explosion::dir[di][1];
                                if (dx < 0 || dx >= mapSize || dy < 0 || dy >= mapSize)
                                    continue;
                                drawMapItem(dx, dy);
                            }
                            gameEnd();
                        }
                    }
                }
                cmap[dx][dy] = targetColor;
                map[dx][dy]++;
                if (map[dx][dy] == fullAtPos(dx, dy))
                    Explosion::qu.push(std::make_pair(dx, dy));
            }

            drawMapItem(hd.first, hd.second);
            for (int di = 0; di < 4; di++)
            {
                int dx = hd.first + Explosion::dir[di][0], dy = hd.second + Explosion::dir[di][1];
                if (dx < 0 || dx >= mapSize || dy < 0 || dy >= mapSize)
                    continue;
                drawMapItem(dx, dy);
            }

            drawUserTable();
            drawCombo(combo,pColor[nowTurn]);
            if (combo < 20)
                Sleep(1000);
            else
                Sleep(500);
        }
        Sleep(100);
        drawCombo(-1,win_control::Color::c_BLACK);
    }

    void renderScreen(){
        win_control::cls();
        drawMap();
        drawUserTable();
        drawChoosingBlock();
    }

    void readNetworkInfo(std::string info)
    {
        if (info.substr(0, 6) == "ec$op$")
        {
            int num[2] = {0}, x, y;
            // win_control::goxy(16, 0);
            // std::cout << "In readNetworkInfo() : " << info << std::endl;
            for (int i = 6, id = 0; i < info.size(); i++)
            {
                if (info[i] == '$')
                {
                    id++;
                    continue;
                }
                num[id] = num[id] * 10 + info[i] - '0';
            }

            x = num[0], y = num[1];
            //        std::cout << "Pos in readNetworkInfo() : " << x << " " << y << std::endl;

            if (map[x][y] == 0)
            {
                if (!initialed[nowTurn])
                {
                    initialed[nowTurn] = 1;
                }
                blocks[nowTurn]++;
            }
            map[x][y]++;
            cmap[x][y] = nowTurn;
            drawMapItem(x, y);
            checkExplode(nowTurn, x, y);
            //    drawMap();
            // printChoosingBlock();
            nextPlayer();
            drawUserTable();
        }
    }
    void serverSolveInfo(std::string info, size_t len)
    {
        info.resize(len);
        // win_control::goxy(28, 0);
        // std::cout << "In serversolveinfo: " << info << std::endl;
        if (info.substr(0, 6) == "ec$op$")
        {
            Network::sendToAll(info);
        }
    }

    void startServerReceiving()
    {
        std::array<char, 65536> buf;
        for (int i = 1; i <= Game::Network::clientConnected; i++)
        {
            Network::Client &cl = Network::clients[i];
            Thread::toClient[i] = std::thread([&]() -> void {
                std::array<char, 65536> buf;
                asio::error_code err;
                size_t len;
                while (Game::onGameRunning)
                {
                    len = cl.sock.read_some(asio::buffer(buf), err);
                    Debug::dprint(buf.data());
                    if (std::string(buf.data()) == "ec$gameend")
                    {
                        Debug::dprint("Server go into ec$gameend\n");
                        Network::sendMessage(cl.sock,"ec$gameend");
                        Debug::dprint("Server message sent\n");
                        return;
                    }
                    serverSolveInfo(buf.data(), len);
                    if (Game::Network::io_context.stopped())
                        return;
                }
            });
            Thread::toClient[i].detach();
        }
    }

    void startGuestListening()
    {
        Thread::receiveThread = std::thread([&]() -> void {
            std::array<char, 65536> buf;
            asio::error_code err;
            size_t len;
            while (Game::onGameRunning)
            {
                len = Network::soc.read_some(asio::buffer(buf), err);
                Debug::dprint("Recv",1);
                Debug::dprint(buf.data(),1);
                if (std::string(buf.data()) == "ec$gameend")
                {
                    Debug::dprint("receiveThread Ended!\n",1);
                    return;
                }
                readNetworkInfo(buf.data());
                if (Game::Network::io_context.stopped()){
                    Debug::dprint("receiveThread Ended!\n",1);
                    return;
                }
            }
            Debug::dprint("receiveThread Ended!\n",1);
        });
        Thread::receiveThread.detach();
    }

    void gameAllInit()
    {
        onGameRunning = true;
        App::gameState = App::MainFrame;
        win_control::consoleInit();
    }

    void gameStart()
    {
        nowTurn = 1;
        painting=false;
        drawMap();
        drawUserTable();
        drawChoosingBlock();
        playerAlive = playerCount;
    }

}

namespace key_handling
{
    void onUp()
    {
        if (Game::choosingPosition.X == 0)
            return;
        Game::drawBlockBorder(Game::choosingPosition.X, Game::choosingPosition.Y, win_control::Color::c_BLACK);
        Game::choosingPosition.X--;
        Game::drawChoosingBlock();
    }
    void onDown()
    {
        if (Game::choosingPosition.X == Game::mapSize - 1)
            return;
        Game::drawBlockBorder(Game::choosingPosition.X, Game::choosingPosition.Y, win_control::Color::c_BLACK);
        Game::choosingPosition.X++;
        Game::drawChoosingBlock();
    }
    void onLeft()
    {
        if (Game::choosingPosition.Y == 0)
            return;
        Game::drawBlockBorder(Game::choosingPosition.X, Game::choosingPosition.Y, win_control::Color::c_BLACK);
        Game::choosingPosition.Y--;
        Game::drawChoosingBlock();
    }
    void onRight()
    {
        if (Game::choosingPosition.Y == Game::mapSize - 1)
            return;
        Game::drawBlockBorder(Game::choosingPosition.X, Game::choosingPosition.Y, win_control::Color::c_BLACK);
        Game::choosingPosition.Y++;
        Game::drawChoosingBlock();
    }
    void onConfirm()
    {
        if (Game::nowTurn != Game::usercol)
            return;
        int x = Game::choosingPosition.X, y = Game::choosingPosition.Y;
        if (!Game::cmap[x][y] || Game::cmap[x][y] == Game::usercol)
        {
            //enable to place

            // win_control::goxy(25,0);
            // std::cout<<x<<" "<<y<<std::endl;
            // std::cout<<"ec$op$" + std::to_string(x) + '$' + std::to_string(y)<<std::endl;
            std::string info = "ec$op$" + std::to_string(x) + '$' + std::to_string(y) + '\000';
            // info.resize(9+int(log10(x))+int(log10(y)));
            //    win_control::goxy(25,0);std::cout<<info<<std::endl;
            Game::Network::sendMessage(Game::Network::soc, info);
        }
    }

}
void ::win_control::input_record::keyHandler(int keyCode)
{
    switch (keyCode)
    {

    case VK_UP:
    {
        key_handling::onUp();
        break;
    }
    case 'W':
    {
        key_handling::onUp();
        break;
    }
    case VK_DOWN:
    {
        key_handling::onDown();
        break;
    }
    case 'S':
    {
        key_handling::onDown();
        break;
    }
    case VK_LEFT:
    {
        key_handling::onLeft();
        break;
    }
    case 'A':
    {
        key_handling::onLeft();
        break;
    }
    case VK_RIGHT:
    {
        key_handling::onRight();
        break;
    }
    case 'D':
    {
        key_handling::onRight();
        break;
    }
    case VK_SPACE:
    {
        key_handling::onConfirm();
        break;
    }
    case 'J':
    {
        key_handling::onConfirm();
        break;
    }
    case VK_ESCAPE:
    {
        Game::Thread::joinServerThread();
        Game::Thread::joinClientThreads();
        Game::Network::io_context.stop();
        exit(0);
    }
    case 'R':{
        Game::renderScreen();
    }
    default:
        break;
    }
}
int main()
{
    try
    {
        Game::gameAllInit();
        while (Game::onGameRunning)
        {
            switch (Game::App::gameState)
            {
            case Game::App::MainFrame:
            {
                //not final from here on
                char gameMode = 0;
                std::cout << "Input game mode (1: Client; 2: Server) :" << std::endl;
                while (1)
                {

                    gameMode = std::getchar();
                    if (gameMode == '1')
                    { //be guest
                        win_control::goxy(0,0);
                        std::cout << Game::References::EMPTYLINE << std::endl
                                  << Game::References::EMPTYLINE << std::endl
                                  << Game::References::EMPTYLINE << std::endl
                                  << Game::References::EMPTYLINE << std::endl;
                        Game::beGuest();
                        win_control::goxy(0, 0);
                        std::cout << "Waiting for game start." << std::endl
                                  << Game::References::EMPTYLINE << std::endl
                                  << Game::References::EMPTYLINE << std::endl;
                        break;
                    }
                    if (gameMode == '2')
                    { //be host
                        win_control::goxy(0,0);
                        std::cout << Game::References::EMPTYLINE << std::endl 
                                  << Game::References::EMPTYLINE << std::endl 
                                  << Game::References::EMPTYLINE << std::endl 
                                  << Game::References::EMPTYLINE << std::endl;
                        Game::beHost();
                        win_control::goxy(0, 0);
                        std::cout << "Game started at IP : Help yourself." << std::endl;
                        win_control::goxy(1, 0);
                        std::cout << "Waiting for players to connect." << std::endl;
                        win_control::goxy(2, 0);
                        std::cout << Game::References::EMPTYLINE;
                        break;
                    }
                }
                Game::App::gameState = Game::App::Waiting;
                break;
            }
            case Game::App::Waiting:
            {
                if (Game::gType == Game::INET_HOST)
                {
                    Game::Network::Client &cl = Game::Network::clients[++Game::Network::clientConnected];
                    cl.sock = Game::Network::ac.accept(/* Game::onHostAccept */);

                    //maybe will be ...
                    std::array<char, 65536> tmp;
                    asio::error_code err;
                    size_t len = cl.sock.read_some(asio::buffer(tmp), err);
                    cl.username = tmp.data();
                    cl.username.resize(len);
                    if (err && err != asio::error::eof)
                        throw asio::system_error(err);

                    win_control::goxy(2, 0);
                    std::cout << "Now players: " << Game::Network::clientConnected << std::endl;
                    for (int i = 1; i <= Game::Network::clientConnected; i++)
                        std::cout << "User #" << i << ": " << Game::Network::clients[i].username << std::endl;

                    if (Game::Network::clientConnected == Game::playerCount)
                    {

                        std::cout << "All players get ready. Game starts in 5 seconds!" << std::endl;
                        win_control::sleep(1000);
                        std::cout << "All players get ready. Game starts in 4 seconds!" << std::endl;
                        win_control::sleep(1000);
                        std::cout << "All players get ready. Game starts in 3 seconds!" << std::endl;
                        win_control::sleep(1000);
                        std::cout << "All players get ready. Game starts in 2 seconds!" << std::endl;
                        win_control::sleep(1000);
                        std::cout << "All players get ready. Game starts in 1 seconds!" << std::endl;
                        win_control::sleep(1000);

                        std::string gameInfo = std::to_string(Game::playerCount) + '$' + std::to_string(Game::mapSize);
                        std::string users = "";
                        for (int i = 1; i <= Game::playerCount; i++)
                        {
                            users += Game::Network::clients[i].username + '$';
                            Game::usrNames[i] = Game::Network::clients[i].username;
                        }
                        for (int i = 2; i <= Game::Network::clientConnected; i++)
                        {
                            Game::Network::sendMessage(Game::Network::clients[i].sock, "ec$gamestart$" + gameInfo + '$' + std::to_string(i) + '$' + users);
                        }
                        Game::usercol = 1;
                        Game::App::gameState = Game::App::Gaming;
                        win_control::cls();
                        win_control::setTitle(L"Explode Chess - Gaming");
                        Game::startServerReceiving();
                        Game::startGuestListening();
                        Game::gameStart();
                    }
                }
                else
                {
                    std::array<char, 65536> buf;
                    asio::error_code err;
                    size_t len = Game::Network::soc.read_some(asio::buffer(buf), err);
                    std::string tmp = buf.data();
                    tmp.resize(len);
                    if (tmp.substr(0, 13) == "ec$gamestart$")
                    {
                        Game::App::gameState = Game::App::Gaming;
                        win_control::cls();
                        win_control::setTitle(L"Explode Chess - Gaming");
                        Game::guestSolveGameInfo(tmp.substr(13, len - 13));

                        Game::startGuestListening();
                        Game::gameStart();
                    }
                }
                break;
            }
            case Game::App::Gaming:
            {
                win_control::input_record::getInput();
                break;
            }
            }
            //    win_control::sleep(50);
        }
    }
    catch (...)
    {
        win_control::goxy(10, 0);
        std::cout << "fuckccf";
    }
    Game::Thread::joinServerThread();
    Game::Thread::joinClientThreads();
    Game::Network::io_context.stop();
    return 0;
}