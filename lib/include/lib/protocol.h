#pragma once

//  stl
#include <string>
#include <memory>

//  lib


class TCPConnection;
using TCPConnectionPtr = std::shared_ptr<TCPConnection>;

class Protocol
{
public:
    Protocol()
        : m_tcpConnection(nullptr)
    {}

    virtual ~Protocol() {}

    void setConnection(TCPConnectionPtr connection) { m_tcpConnection = connection; }

    virtual void send(const std::string& msg) = 0;
    virtual void receive(const std::string& msh) = 0;

protected:
    TCPConnectionPtr m_tcpConnection;
};
