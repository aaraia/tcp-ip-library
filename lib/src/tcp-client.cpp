#include "tcp-client.h"

//  stl
#include <iostream>

//  boost
#include <boost/array.hpp>
#include <boost/asio.hpp>

//
#include "tcp-connection.h"


using boost::asio::ip::tcp;

TCPClient::TCPClient(boost::asio::io_context& io, const std::string& host, const std::string& service, ProtocolPtr protocol)
    : io_context(io)
    , resolver(io)
    , socket(nullptr)
    , m_host(host)
    , m_service(service)
    , m_protocol(protocol)
{

}

void TCPClient::connect()
{   
    socket = std::make_shared<tcp::socket>(io_context);

    tcp::resolver::results_type endpoints = resolver.resolve(m_host, m_service, tcp::resolver::passive);

    try {
        auto result = boost::asio::connect(*socket, endpoints);
        m_connection = TCPConnection::create(io_context, *socket, m_protocol);
    }
    catch (std::exception& e)
    {
        std::cout << "could not connect " << e.what() << " \n";
    }
}

void TCPClient::disconnect()
{
    socket->close();
    socket.reset();
}
