#include "tcp-connection.h"

//  lib
#include "protocol.h"
#include "message.pb.h"

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

void TCPConnection::registerProtocol(ProtocolPtr protocol)
{
    if (!protocol) return;

    m_protocols.emplace(protocol->getID(), protocol);
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
    
    //  check for errors
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

    auto fndHeader = m_currentMessage.find_last_of(m_messageHeader);
    if (fndHeader == std::string::npos) {
        //  no header yet
        m_reading = false;
        return;
    }

    auto fndFooter = m_currentMessage.find_last_of(m_messageFooter);
    if (fndFooter == std::string::npos)
    {
        //  no footer yet
        m_reading = false;
        return;
    }

    if (fndFooter <= fndHeader)
    {
        //  could be multiple messages
        //  decide what to do here! TODO
        //  for no print message
        std::cout << "footer is before header " << m_currentMessage;
        m_reading = false;
        return;
    }

    //  valid packet of data
    std::string packet = m_currentMessage.substr(fndHeader + m_messageHeader.size(), fndFooter);    

    //  convert to protocol buffer
    message::Message message;
    message.ParseFromString(packet);

    //  find the protocol
    uint64_t id = message.protocol_id();
    auto protocolIter = m_protocols.find(id);
    if (protocolIter == m_protocols.end())
    {
        //  no protocol registered!
        std::cout << "protocol: " << id << " not registered";
        m_reading = false;
        return;
    }

    //  send the message to the correct protocol for further processing
    protocolIter->second->receive(std::move(message));

    //  print some debug and clear the message 
    std::string remainderFront = m_currentMessage.substr(0, fndHeader);
    std::string remainderBack = m_currentMessage.substr(fndFooter);
    std::cout << "remainderFront " << remainderFront;
    std::cout << "remainderBack " << remainderBack;
    m_currentMessage.resize(0);

    m_reading = false;
}

void TCPConnection::send(std::string&& message)
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
        std::lock_guard<std::mutex> lock(m_shareSocketMutex);

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
        std::lock_guard<std::mutex> lock(m_shareSocketMutex);

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
