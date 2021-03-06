#include "tcp-server.h"

//  lib
#include "tcp-connection.h"
#include "protocol.h"

//  boost
#include <boost/bind/bind.hpp>

namespace {
    const int SERVER_CONNECTION_ID = 1;
}

TCPServer::TCPServer(boost::asio::io_context& io_context, ProtocolPtr protocol)
    : io_context_(io_context)
    , acceptor_(io_context, tcp::endpoint(tcp::v4(), 13))
    , t(io_context, boost::asio::chrono::seconds(5))
    , m_connectionID(SERVER_CONNECTION_ID)
    , m_protocol(protocol)
{
    t.async_wait(boost::bind(&TCPServer::heartBeat, this));

    startAccept();
}

void TCPServer::startAccept()
{
    TCPConnectionPtr new_connection = TCPConnection::create(io_context_, m_protocol);
    m_protocol->setConnection(new_connection);

    acceptor_.async_accept(new_connection->getSocket(),
                            boost::bind(&TCPServer::handleAccept, 
                            this, 
                            new_connection,
                            boost::asio::placeholders::error));
}

void TCPServer::handleAccept(TCPConnectionPtr new_connection, const boost::system::error_code& error)
{
    std::cout << "handleAccept: error: " << error.value() << "\n";

    if (!error)
    {
        new_connection->start();

        std::lock_guard<std::mutex> lock(m_mutex);
        m_connections.emplace(m_connectionID, new_connection);
        ++m_connectionID;

        std::cout << "handleAccept: connection started\n";
    }

    startAccept();
}

void TCPServer::heartBeat()
{
    std::lock_guard<std::mutex> lock(m_mutex);    
    for (auto iter = m_connections.begin(); iter != m_connections.end();)
    {
        if ((*iter).second->hasError())
        {
            //  something went wrong with a connection that was connected to a client
            (*iter).second->stop();
            iter = m_connections.erase(iter);
            std::cout << "erased connection\n";
        }
        else
        {
            //  no one to send heart beat to
            //  has not connected with a client
            //  just listening
            if ((*iter).second->isStarted())
            {
                (*iter).second->send("123");
            }
            else
            {
                std::cout << "hearbeat else\n";
            }

            ++iter;
        }
    }

    std::cout << "timer added\n";
    t.expires_at(t.expiry() + boost::asio::chrono::seconds(5));
    t.async_wait(boost::bind(&TCPServer::heartBeat, this));
}

