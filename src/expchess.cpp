#include <iostream>
#include <thread>
#include <array>
#include "wincontrol.hpp"
#include "asio.hpp"

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
        }

    }

    void gameAllInit()
    {
        onGameRunning = true;
        win_control::consoleInit();
    }
}
int main()
{
    Game::gameAllInit();
    while (Game::onGameRunning)
    {
    }
}