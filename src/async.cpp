#include "asio.hpp"
#include <iostream>
using namespace std;
int main()
{
    asio::io_context ioc;
    asio::ip::tcp::socket sock(ioc);
    std::array<char, 114514> buf;


    sock.async_read_some(asio::buffer(buf), [&](const asio::error_code &err, std::size_t byteTransfered) -> void{
        std::cout.write(buf.data(), byteTransfered);
    });


    std::thread t([]()->void{
        ioc.run();
        
    });


    return 0;
}