#include "protocolA.h"

//  stl
#include <iostream>

//  lib
#include "tcp-connection.h"


ProtocolA::ProtocolA()
    : Protocol()
{

}

ProtocolA::~ProtocolA()
{

}

void ProtocolA::send(const std::string& msg)
{
    std::cout << "ProtocolA::send " << msg << std::endl;

    if (m_tcpConnection)
    {
        m_tcpConnection->send(msg);
    }
}

void ProtocolA::receive(const std::string& msg)
{
    std::cout << "ProtocolA::receive " << msg << std::endl;

    //  find out which command.
    //  maybe first two bytes in strinf store protocol number
    int cmd = 14;

    switch (cmd)
    {
    case 14:
        //  convert to protocol buffer command
        //  send to handler that is registered
        break;
    }

}