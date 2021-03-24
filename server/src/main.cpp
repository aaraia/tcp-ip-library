#include <iostream>
#include <boost/filesystem.hpp>

//  libTCP
#include "tcp-server.h"
#include "message.pb.h"

//  
#include "protocolA.h"

int main()
{
    try
    {
        message::Message m;
        ProtocolPtr protocol = std::make_shared<ProtocolA>();
        boost::asio::io_context io_context;
        TCPServer server(io_context, protocol);
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
