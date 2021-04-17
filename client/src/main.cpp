//  stl
#include <iostream>
#include "boost/filesystem.hpp"

//  lib
#include "tcp-client.h"
#include "message.pb.h"
#include "protocolA.h"

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        return 0;
    }

    //  strip out the service and host name
    std::string host(argv[1]);
    std::string service(argv[2]);

    try
    {
        //  create the protocol that will be used for communication
        ProtocolAPtr protocol = std::make_shared<ProtocolA>();
        boost::asio::io_context io_context;
        TCPClient client(io_context, host, service, protocol);
        client.connect();
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
