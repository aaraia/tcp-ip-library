//  stl
#include <iostream>
#include <boost/filesystem.hpp>

//  lib
#include "tcp-server.h"
#include "get_name_cmd.pb.h"
#include "protocolA.h"

int main()
{
    try
    {
        //  create the protocol that will be used for communication
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
