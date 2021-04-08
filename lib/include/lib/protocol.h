#pragma once

//  stl
#include <string>
#include <memory>

//  lib
#include "message.pb.h"
#include "tcp-connection.h"


class TCPConnection;
using TCPConnectionPtr = std::shared_ptr<TCPConnection>;

class Protocol
{
public:
    Protocol(const uint64_t id)
        : m_tcpConnection(nullptr)
        , m_id(id)
    {}

    virtual ~Protocol() {}

    void setConnection(TCPConnectionPtr connection) { m_tcpConnection = connection; }

    uint64_t getID() const { return m_id; }

    template<typename CMD>
    void send(CMD&& cmd)
    {
        if (!m_tcpConnection) return;

        assert(cmd.cmd_id() == m_id);

        //  convert cmd to a message
        message::Message m;
        m.set_protocol_id(cmd.protocol_id());
        m.set_cmd_id(cmd.cmd_id());
        m.set_body(cmd.SerializeAsString());

        //  send message down the tcp connection
        m_tcpConnection->send(std::move(m.SerializeAsString()));
    }

    //virtual void send(const std::string& msg) = 0;
    virtual void receive(message::Message&& msg) = 0;

protected:
    TCPConnectionPtr m_tcpConnection;
    uint64_t m_id;
};
