#include "tcp-connection.h"

//  lib
#include "protocol.h"

//  stl
#include <chrono>

//  boost
#include <boost/bind/bind.hpp>

TCPConnection::TCPConnection(boost::asio::io_context& io_context, ProtocolPtr protocol)
    : m_socket(io_context)
    , m_listenThread(&TCPConnection::listenFunc, this)
    , m_receiveThread(&TCPConnection::senderFunc, this)
    , m_timer(io_context, boost::asio::chrono::seconds(5))
    , m_protocol(protocol)
{
    //  we need to give the io context something to do or it will shutdown so lets use a timer
    m_timer.async_wait(boost::bind(&TCPConnection::keepAlive, this));
}

TCPConnection::TCPConnection(boost::asio::io_context& io_context, tcp::socket& soc, ProtocolPtr protocol)
    : m_socket(std::move(soc))
    , m_started(true)
    , m_listenThread(&TCPConnection::listenFunc, this)
    , m_receiveThread(&TCPConnection::senderFunc, this)
    , m_timer(io_context, boost::asio::chrono::seconds(5))
    , m_protocol(protocol)
{
    //  we need to give the io context something to do or it will shutdown so lets use a timer
    m_timer.async_wait(boost::bind(&TCPConnection::keepAlive, this));
}

TCPConnection::~TCPConnection()
{
    std::cout << "destroy tcp connection\n";

    if (m_listenThread.joinable())
    {
        m_listenThread.join();
    }

    std::cout << "destroy m_listenThread \n";

    if (m_receiveThread.joinable())
    {
        m_receiveThread.join();
    }

    std::cout << "destroy m_receiveThread \n";
}

TCPConnectionPtr TCPConnection::create(boost::asio::io_context& io_context, ProtocolPtr protocol)
{
    return TCPConnectionPtr(new TCPConnection(io_context, protocol));
}

TCPConnectionPtr TCPConnection::create(boost::asio::io_context& io_context, tcp::socket& soc, ProtocolPtr protocol)
{
    return TCPConnectionPtr(new TCPConnection(io_context, soc, protocol));
}

tcp::socket& TCPConnection::getSocket()
{
    return m_socket;
}

void TCPConnection::start()
{
    m_started = true;

    send("start");
}

void TCPConnection::stop()
{
    m_stop = true;
    m_timer.cancel();
}

void TCPConnection::handleWrite(const boost::system::error_code& error, size_t bytes_transferred)
{
    //  The send operation may not transmit all of the data to the peer.
    //  Consider using the write function if you need to ensure that all data 
    //  is written before the blocking operation completes.

    std::cout << "\n";
    std::cout << "writing - bytes_transferred: " << bytes_transferred << "\n";

    m_writing = false;
}

void TCPConnection::handleRead(const boost::system::error_code& error, size_t bytes_transferred)
{
    std::cout << "\n";
    std::cout << "handleRead - bytes_transferred: " << bytes_transferred << "\n";
    
    if (error == boost::asio::error::eof)
    {
        std::cout << "no error from read\n";
    }
    else if (error)
    {
        m_reading = false;
        m_connectionResetByPeer = true;
        std::cout << "connection reset by peer value = " << error.value() << "\n";
        return;
    }

    //  check for header
    int numHeaderFound = 0;
    for (int i = 0; i < m_messageHeader.size() && m_readBuffer.size() >= m_messageHeader.size(); ++i)
    {
        if (m_readBuffer[i] == m_messageHeader[i])
        {
            ++numHeaderFound;
        }
    }

    bool foundHeader = numHeaderFound == m_messageHeader.size();

    std::string data;
    std::string otherData;
    std::for_each(m_readBuffer.begin(), m_readBuffer.begin() + bytes_transferred, [&data, &otherData, this](const char c) {
        if (c != '\n')
        {
            data.push_back(c);
            m_currentMessage.push_back(c);
        }
        else
        {
            otherData.push_back(c);
        }
    });

    std::string buff;
    for (int i = 0; i < READ_BUFFER_SIZE; ++i)
    {
        buff.push_back(m_readBuffer[i]);
    }

    std::cout << "buff: " << buff << "\n";

    //  check for footer in the total data recieved so far, this is because the footer could be sent in two packets and be split.
    //  the header will always be in the first packet as it is the first data sent, but the footer is the last data sent anc
    //  could push past the tcp buffer size of 128 bytes and some of it sent in the next packet/message
    int numFooterFound = 0;
    for (int i = 0; i < m_messageFooter.size() && m_currentMessage.size() >= m_messageFooter.size(); ++i)
    {
        if (m_currentMessage[(m_currentMessage.size() - m_messageFooter.size()) + i] == m_messageFooter[i])
        {
            ++numFooterFound;
        }
    }

    bool foundFooter = numFooterFound == m_messageFooter.size();

    if (foundHeader)
    {
        //  clear and prepare for next message
        std::cout << "header\n";
    }

    if (foundFooter)
    {
        //  clear and prepare for next message
        std::cout << "footer\n";
        m_lastMessage = m_currentMessage;
        m_currentMessage.resize(0);
        
        if (m_protocol)
        {
            m_protocol->receive(m_lastMessage);
        }
    }

    std::cout << "data: " << data << "\n";
    std::cout << "otherData: " << otherData << "\n";
    std::cout << "m_currentMessage: " << m_currentMessage << "\n";
    std::cout << "m_lastMessage: " << m_lastMessage << "\n";

    m_reading = false;
}

void TCPConnection::send(const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_senderQueueMutex);
    m_sendQueue.emplace(message);
}

void TCPConnection::listenFunc()
{
    for (;;)
    {
        if (m_stop) break;
        if (!m_started) continue;
        if (m_connectionResetByPeer) break;
        if (m_reading) continue;

        //  lock the mutex to prevent any sending while listening
        std::lock_guard<std::mutex> lock(m_socketMutex);

        m_reading = true;

        m_socket.async_read_some(boost::asio::buffer(m_readBuffer, READ_BUFFER_SIZE),
                                 boost::bind(&TCPConnection::handleRead, this,
                                 boost::asio::placeholders::error,
                                 boost::asio::placeholders::bytes_transferred));

        //  sleep to allow time for sending on the socket
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void TCPConnection::senderFunc()
{
    for (;;)
    {
        if (m_stop) break;
        if (!m_started) continue;
        if (m_connectionResetByPeer) break;
        if (m_writing) continue;

        //  sleep to allow some time for listening on the socket
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        //  lock the queue so that nothing else is modifying it while we use it
        std::lock_guard<std::mutex> lockQueue(m_senderQueueMutex);

        //  any messages to send
        if (m_sendQueue.empty()) continue;

        //  lock the mutex to prevent any listening while sending
        std::lock_guard<std::mutex> lock(m_socketMutex);

        std::string message = m_sendQueue.front();

        const int actualSize = SEND_BUFFER_SIZE - (m_messageHeader.size() + m_messageFooter.size() + 1); //  the 1 is for null terminator

        if (message.size() >= actualSize)
        {
            //  need a way to tell the client that something went wrong with this message
            //  maybe add a header 
            std::cout << "message size: " << message.size() << " max allowed: " << actualSize;
        }
        
        int index = 0;

        //  add header
        for (int c = 0; c < m_messageHeader.size(); ++c)
        {
            if (index < actualSize)
            {
                m_sendBuffer[index] = m_messageHeader[c];
                ++index;
            }
        }

        std::for_each(message.begin(), message.end(), [&index, &actualSize, this](const char c)
        {
            if (index < actualSize)
            {
                m_sendBuffer[index] = c;
                ++index;
            }        
        });
        
        //  add footer
        for (int c = 0; c < m_messageFooter.size(); ++c)
        {
            if (index < actualSize)
            {
                m_sendBuffer[index] = m_messageFooter[c];
                ++index;
            }
        }

        //  null terminate
        m_sendBuffer[index] = '\n';

        //  remove message
        m_sendQueue.pop();

        m_writing = true;

        std::string buffTemp;
        for (int i = 0; i < index; ++i)
        {
            buffTemp.push_back(m_sendBuffer[i]);
        }
        
        std::cout << buffTemp << "\n";
       
        boost::asio::async_write(m_socket,
                                 boost::asio::buffer(m_sendBuffer, index),
                                 boost::bind(&TCPConnection::handleWrite, shared_from_this(),
                                 boost::asio::placeholders::error,
                                 boost::asio::placeholders::bytes_transferred));
    }
}

void TCPConnection::keepAlive()
{
    if (m_stop) return;

    m_timer.expires_at(m_timer.expiry() + boost::asio::chrono::seconds(5));
    m_timer.async_wait(boost::bind(&TCPConnection::keepAlive, this));
}
