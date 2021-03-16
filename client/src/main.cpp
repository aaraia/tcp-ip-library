#include <iostream>
#include "boost/filesystem.hpp"

//  libTCP
#include "tcp-client.h"

//
#include "protocolA.h"

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        return 0;
    }

    //  service and host name
    std::string host(argv[1]);
    std::string service(argv[2]);

    try
    {
        ProtocolAPtr protocol = std::shared_ptr<ProtocolA>(new ProtocolA{});
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
