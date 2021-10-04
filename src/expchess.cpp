#include <iostream>
#include <thread>
#include <array>
#include "wincontrol.hpp"
#include "asio.hpp"

namespace Game
{
    volatile bool onGameRunning;
    asio::io_context io_context;

    int playerCount=0;

    enum GameType
    {
        INET_HOST,
        INET_GUEST,
        LOCAL
    } gType;
    namespace References
    {
        const size_t MAX_CLIENT_COUNT = 10;
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
        void clientStartup(asio::ip::tcp::endpoint endpoint, bool localHost = 0)
        {
            soc = asio::ip::tcp::socket(io_context, endpoint);
        }
        void serverStartup(int port)
        {
            ac = asio::ip::tcp::acceptor(
                io_context,
                asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
            clientConnected = 0;
        }

    }
    void gameAllInit()
    {
        onGameRunning = true;
        win_control::consoleInit();
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
            break;
        }
        case Game::App::Waiting:
        {
            Game::Network::Client &cl = Game::Network::clients[++Game::Network::clientConnected];
            cl.sock=Game::Network::ac.accept();
            asio::error_code err;
            size_t len = cl.sock.read_some(asio::buffer(cl.username),err);
            if (err && err != asio::error::eof)
                throw asio::system_error(err);


            if(Game::Network::clientConnected==Game::playerCount){
                Game::App::gameState=Game::App::Gaming;
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