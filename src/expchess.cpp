#include <iostream>
#include <thread>
#include <array>
#include "wincontrol.hpp"
#include "asio.hpp"

void win_control::cls()
{
    win_control::goxy(0, 0);
    for (short i = 0; i < 40; i++)
    {
        std::cout << "                                                                                \n";
    }
}

namespace Game
{
    volatile bool onGameRunning;
    asio::io_context io_context;

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

    win_control::Color pColor[References::MAX_CLIENT_COUNT + 1] = {
        win_control::Color::c_DGREY,
        win_control::Color::c_RED,
        win_control::Color::c_LIME,
        win_control::Color::c_LYELLOW,
        win_control::Color::c_SKYBLUE,
        win_control::Color::c_PURPLE,
        win_control::Color::c_LIGHTBLUE,
        win_control::Color::c_DPURPLE,
        win_control::Color::c_DARKBLUE,
        win_control::Color::c_DGREEN,
        win_control::Color::c_DRED,
    };
    int playerCount = 0;
    int mapSize = 0, map[References::MAX_MAP_SIZE][References::MAX_MAP_SIZE],
        cmap[References::MAX_MAP_SIZE][References::MAX_MAP_SIZE];

    COORD choosingPosition = {0, 0};
    int nowTurn, usercol;

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
        void sendMessage(asio::ip::tcp::socket &soc, std::string msg)
        {
            asio::write(soc, asio::buffer(msg));
        }
        void sendToAll(std::string msg)
        {
            if (gType != INET_HOST)
                return;
            for (int i = 1; i <= clientConnected; i++)
            {
                sendMessage(clients[i].sock, msg);
            }
        }
    }

    void beGuest()
    {
        //not final version

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
        for (int i = 0, id = 0; i < info.size(); i++)
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
    }
    void printBlockBorder(int x, int y, win_control::Color col)
    {
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
    }
    void printChoosingBlock()
    {
        printBlockBorder(choosingPosition.X, choosingPosition.Y, pColor[usercol]);
    }
    void drawMap(int fx = 0, int fy = 0, int tx = mapSize - 1, int ty = mapSize - 1)
    {
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
    }

    void gameAllInit()
    {
        onGameRunning = true;
        App::gameState = App::MainFrame;
        win_control::consoleInit();
    }

    void gameStart()
    {
        drawMap();
        printChoosingBlock();
        nowTurn = 1;
    }
}

namespace key_handling
{
    void onUp()
    {
        if (Game::choosingPosition.X == 0)
            return;
        Game::printBlockBorder(Game::choosingPosition.X, Game::choosingPosition.Y, win_control::Color::c_BLACK);
        Game::choosingPosition.X--;
        Game::printChoosingBlock();
    }
    void onDown()
    {
        if (Game::choosingPosition.X == Game::mapSize - 1)
            return;
        Game::printBlockBorder(Game::choosingPosition.X, Game::choosingPosition.Y, win_control::Color::c_BLACK);
        Game::choosingPosition.X++;
        Game::printChoosingBlock();
    }
    void onLeft()
    {
        if (Game::choosingPosition.Y == 0)
            return;
        Game::printBlockBorder(Game::choosingPosition.X, Game::choosingPosition.Y, win_control::Color::c_BLACK);
        Game::choosingPosition.Y--;
        Game::printChoosingBlock();
    }
    void onRight()
    {
        if (Game::choosingPosition.Y == Game::mapSize - 1)
            return;
        Game::printBlockBorder(Game::choosingPosition.X, Game::choosingPosition.Y, win_control::Color::c_BLACK);
        Game::choosingPosition.Y++;
        Game::printChoosingBlock();
    }
    void onConfirm()
    {
        if (Game::nowTurn != Game::usercol)
            return;
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

            //    win_control::goxy(2,0);std::cout<<Game::App::gameState;
            switch (Game::App::gameState)
            {
            case Game::App::MainFrame:
            {
                //not final from here on
                char gameMode = 0;
                while (1)
                {
                    gameMode = std::getchar();
                    if (gameMode == '1')
                    { //be guest
                        Game::beGuest();
                        win_control::goxy(0, 0);
                        std::cout << "Waiting for game start." << std::endl
                                  << Game::References::EMPTYLINE << std::endl
                                  << Game::References::EMPTYLINE << std::endl;
                        break;
                    }
                    if (gameMode == '2')
                    { //be host
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

                        std::string gameInfo = std::to_string(Game::playerCount) + '$' + std::to_string(Game::mapSize) + '$';
                        for (int i = 2; i <= Game::Network::clientConnected; i++)
                        {
                            Game::Network::sendMessage(Game::Network::clients[i].sock, "ec$gamestart$" + gameInfo + std::to_string(i));
                        }
                        Game::usercol=1;
                        Game::App::gameState = Game::App::Gaming;
                        win_control::cls();
                        win_control::setTitle(L"Explode Chess - Gaming");
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
            win_control::sleep(50);
        }
    }
    catch (...)
    {
        win_control::goxy(10, 0);
        std::cout << "cy";
    }
}