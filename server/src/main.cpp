#include <iostream>
#include <boost/filesystem.hpp>

//  libTCP
#include "tcp-server.h"

//  
#include "protocol-a.h"

ProtocolPtr addProtocol()
{
    return std::make_shared<ProtocolA>();
}

int main()
{
    try
    {

        boost::asio::io_context io_context;
        TCPServer server(io_context, std::bind(addProtocol));
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
