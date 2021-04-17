#pragma once

//  stl
#include <iostream>

//  boost
#include <boost/array.hpp>
#include <boost/asio.hpp>

//  forwards
using boost::asio::ip::tcp;

class Protocol;
using ProtocolPtr = std::shared_ptr<Protocol>;

class TCPConnection;
typedef std::shared_ptr<TCPConnection> TCPConnectionPtr;

class TCPClient
{
public:
    TCPClient(boost::asio::io_context& io, const std::string& host, const std::string& service, ProtocolPtr protocol);

    void connect();
    void disconnect();
private:
    boost::asio::io_context& io_context;
    tcp::resolver resolver;
    TCPConnectionPtr m_connection;
    std::shared_ptr<tcp::socket> socket;
    std::string m_host;
    std::string m_service;
    ProtocolPtr m_protocol;
};
