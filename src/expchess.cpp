#include <iostream>
#include <thread>
#include <array>
#include "wincontrol.hpp"
#include "asio.hpp"

namespace Game
{
    volatile bool onGameRunning;
    asio::io_context io_context;

    int playerCount = 0;

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
        std::cout<<"IP Address: ";
        std::string ipAddress;
        std::cin >> ipAddress;
        std::cout<<"Your user name: ";
        std::string username;
        std::cin>>username;
        try
        {
            Network::clientStartup(ipAddress);
            Network::sendMessage(Network::soc,username);
        }
        catch (std::exception &e)
        {
            exit(1);
        }
    }
    void beHost()
    {
        gType = INET_HOST;
        Network::serverStartup(References::PORT);
        std::string username="";
        std::cout<<"Your user name: ";
        std::cin>>username;
        Network::clientStartup("127.0.0.1", 1);
        Network::sendMessage(Network::soc,username);
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
    void gameAllInit()
    {
        onGameRunning = true;
        App::gameState = App::MainFrame;
        win_control::consoleInit();
    }
}

namespace key_handling
{
    void onUp()
    {
    }
    void onDown()
    {
    }
    void onLeft()
    {
    }
    void onRight()
    {
    }
    void onConfirm()
    {
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
    Game::gameAllInit();
    while (Game::onGameRunning)
    {
        win_control::input_record::getInput();
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
                    win_control::goxy(0,0);
                    std::cout<<"Waiting for game start."<<std::endl;
                }
                if (gameMode == '2')
                { //be host
                    Game::beHost();
                    win_control::goxy(0,0);
                    std::cout<<"Waiting for players to connect."<<std::endl;
                    win_control::goxy(1,0);
                    std::cout<<"                                   ";
                }
            }
            Game::App::gameState=Game::App::Waiting;
            break;
        }
        case Game::App::Waiting:
        {
            if (Game::gType == Game::INET_HOST)
            {
                Game::Network::Client &cl = Game::Network::clients[++Game::Network::clientConnected];
                cl.sock = Game::Network::ac.accept();
                win_control::goxy(10,0);std::cout<<"fccf";
                asio::error_code err;
                size_t len = cl.sock.read_some(asio::buffer(cl.username), err);
                if (err && err != asio::error::eof)
                    throw asio::system_error(err);

                win_control::goxy(1,0);
                std::cout<<"Now players: "<<Game::Network::clientConnected<<std::endl;
                if (Game::Network::clientConnected == Game::playerCount)
                {
                    Game::Network::sendToAll("ec$gamestart");
                    Game::App::gameState = Game::App::Gaming;
                }
            }
            else
            {
                std::string buf;
                asio::error_code err;
                Game::Network::soc.read_some(asio::buffer(buf), err);
                if (buf == "ecc$gamestart")
                {
                    Game::App::gameState = Game::App::Gaming;
                }
            }
            break;
        }
        case Game::App::Gaming:
        {
            
            break;
        }
        }
        win_control::sleep(50);
    }
}