#pragma once

//  stl
#include <iostream>
#include <queue>
#include <unordered_map>

//  boost
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>

//  forwards
using boost::asio::ip::tcp;

class Protocol;
using ProtocolPtr = std::shared_ptr<Protocol>;

class TCPConnection;
typedef std::shared_ptr<TCPConnection> TCPConnectionPtr;

class TCPConnection : public std::enable_shared_from_this<TCPConnection>
{
public:
    ~TCPConnection();

    static TCPConnectionPtr create(boost::asio::io_context& io_context, ProtocolPtr protocol);
    static TCPConnectionPtr create(boost::asio::io_context& io_context, tcp::socket& soc, ProtocolPtr protocol);

    tcp::socket& getSocket();

    void send(std::string&& message);

    //  start/stop the connection
    void start();
    void stop();

    //  register a protocol
    void registerProtocol(ProtocolPtr protocol);

    //  define the packet header
    void setHeader(const std::string& str) { m_messageHeader = str; }
    void setFooter(const std::string& str) { m_messageFooter = str; }

    //  query the state of the connection
    bool isStarted() const { return m_started; }
    bool hasError() const { return m_connectionResetByPeer; }

private:
    TCPConnection(boost::asio::io_context& io_context, ProtocolPtr protocol);
    TCPConnection(boost::asio::io_context& io_context, tcp::socket& soc, ProtocolPtr protocol);

    void listenFunc();
    void senderFunc();

    void handleWrite(const boost::system::error_code& error, size_t bytes_transferred);
    void handleRead(const boost::system::error_code& error, size_t bytes_transferred);

    void keepAlive();

private:
    static const int SEND_BUFFER_SIZE = 1024;
    static const int READ_BUFFER_SIZE = 128;
    std::unordered_map<uint64_t, ProtocolPtr> m_protocols;
    std::queue<std::string> m_sendQueue;
    boost::array<char, READ_BUFFER_SIZE> m_readBuffer;
    boost::array<char, SEND_BUFFER_SIZE> m_sendBuffer;
    tcp::socket m_socket;
    boost::asio::steady_timer m_timer;
    std::thread m_listenThread;
    std::thread m_receiveThread;
    std::mutex m_shareSocketMutex;
    std::mutex m_senderQueueMutex;
    ProtocolPtr m_protocol;
    std::string m_lastMessage;
    std::string m_currentMessage;
    std::string m_messageHeader = "¬¬";
    std::string m_messageFooter = "^^";
    std::atomic_bool m_started = false;
    std::atomic_bool m_connectionResetByPeer = false;
    std::atomic_bool m_reading = false;
    std::atomic_bool m_writing = false;
    std::atomic_bool m_stop = false;
};

