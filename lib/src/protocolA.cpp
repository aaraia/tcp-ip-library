#include "protocolA.h"

//  stl
#include <iostream>

//  lib
#include "tcp-connection.h"
#include "get_name_cmd.pb.h"
#include "get_name_reply.pb.h"
#include "observer.h"


ProtocolA::ProtocolA()
    : Protocol(ProtocolA::PROTOCOL_A)
    , m_notifyThread(&ProtocolA::notifyObserversThread, this)
{

}

ProtocolA::~ProtocolA()
{
    if (m_notifyThread.joinable())
    {
        m_notifyThread.join();
    }
}

void ProtocolA::receive(message::Message&& msg)
{
    //  TODO: need some sore of rate limiting here
    // 
    //  acquire lock
    std::lock_guard<std::mutex> l{m_mutex};

    //  add new message to list
    if (m_inQueue)
    {
        m_inQueue->push(msg);
    }
}

void ProtocolA::notifyObserversThread()
{
    //  acquire mutex that can be unlocked
    std::unique_lock<std::mutex> ul{m_mutex};
    //  swap pointers to our queues
    std::swap(m_inQueue,m_outQueue);
    //  now we have done the swap we no longer need the lock
    //  no other thread will operate on m_outQueue
    ul.unlock();

    while (!m_outQueue->empty())
    {
        auto msg = m_outQueue->front();

        //  no observers for this cmd
        auto fnd = m_observers.find(msg.cmd_id());
        if (fnd != m_observers.end())
        {
            m_outQueue->pop();
            continue;
        }

        //  no observers
        if (fnd->second.empty())
        {
            m_outQueue->pop();
            continue;
        }
         
        //  convert the message into a protocol command
        //  and send to observers
        for (auto obs : fnd->second)
        {
            switch (msg.cmd_id())
            {
            case GET_NAME_CMD:
            {
                message::GetNameCMD cmd{};
                cmd.set_protocol_id(PROTOCOL_A);
                cmd.set_cmd_id(GET_NAME_CMD);
                // TODO: will need an observer for each type of command
            }
            break;
            case GET_NAME_REPLY:
            {
                message::GetNameReply cmd{};
                cmd.set_protocol_id(PROTOCOL_A);
                cmd.set_cmd_id(GET_NAME_REPLY);
                // TODO: will need an observer for each type of command
            }
            break;
            default:
                //  logging here
                break;
            }
        }

        //  this message has been processed
        m_outQueue->pop();
    }
}