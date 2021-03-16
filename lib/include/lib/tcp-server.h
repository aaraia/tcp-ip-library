#pragma once

//  stl
#include <ctime>
#include <iostream>
#include <string>

//  boost
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Protocol;
using ProtocolPtr = std::shared_ptr<Protocol>;

class TCPConnection;
typedef std::shared_ptr<TCPConnection> TCPConnectionPtr;

class TCPServer
{
public:
    TCPServer(boost::asio::io_context& io_context, ProtocolPtr protocol);

private:
    void startAccept();

    void handleAccept(TCPConnectionPtr new_connection, const boost::system::error_code& error);

    void heartBeat();

private:
    ProtocolPtr m_protocol; 
    boost::asio::steady_timer t;
    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    //  eventually it might be good to have multiple protocols, one for each client.
    //  but for now each connection will be using the servers protocol
    //  in the future the servers protocol could be used just for connections
    //  the client uses that protocol to connect to the server then later tells the server 
    //  which protocol it wants to use.
    std::map<int, ProtocolPtr> m_protocols; 
    std::map<int, TCPConnectionPtr> m_connections;
    std::mutex m_mutex;
    int m_connectionID;
};
