#include <iostream>
#include <boost/filesystem.hpp>

//  libTCP
#include "tcp-server.h"
#include "get_name_cmd.pb.h"

//  
#include "protocolA.h"

int main()
{
    try
    {
        message::GetNameCMD cmd;
        cmd.set_cmd_id(ProtocolA::GET_NAME_CMD);

        ProtocolPtr protocol = std::make_shared<ProtocolA>();
        protocol->send(cmd);

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
