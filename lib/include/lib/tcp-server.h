#pragma once

//  stl
#include <ctime>
#include <iostream>
#include <string>

//  boost
#include <boost/asio.hpp>

//  forwards
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
    std::map<int, TCPConnectionPtr> m_connections;
    std::mutex m_mutex;
    int m_connectionID;
};
