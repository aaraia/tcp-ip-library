#include "protocolA.h"

//  stl
#include <iostream>

//  lib
#include "tcp-connection.h"
#include "get_name_cmd.pb.h"
#include "get_name_reply.pb.h"
#include "observerGetNameCmd.h"
#include "observerGetNameReply.h"


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
    //  TODO: need some sort of rate limiting here
    //  what to do when full? log then drop messages?

    //  acquire lock
    std::lock_guard<std::mutex> l{m_mutex};

    //  add new message to list
    if (m_inQueue)
    {
        m_inQueue->push(std::move(msg));
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

        //  there were observers, but they were removed
        if (fnd->second.empty())
        {
            m_outQueue->pop();
            continue;
        }

        //  find out which message it is
        switch (msg.cmd_id())
        {
        case GET_NAME_CMD:
        {
            message::GetNameCMD cmd{};
            cmd.set_protocol_id(PROTOCOL_A);
            cmd.set_cmd_id(GET_NAME_CMD);

            for (auto obs : fnd->second)
            {
                //  cast to the right type of observer
                auto obsGetNameCmd = std::dynamic_pointer_cast<ObserverGetNameCmd>(obs);
                if (obsGetNameCmd)
                {
                    obsGetNameCmd->notify(cmd);
                }
            }
        }
        break;
        case GET_NAME_REPLY:
        {
            //  convert the message into a protocol command
            //  and send to observers
            message::GetNameReply cmd{};
            cmd.set_protocol_id(PROTOCOL_A);
            cmd.set_cmd_id(GET_NAME_REPLY);

            for (auto obs : fnd->second)
            {
                //  cast to the right type of observer
                auto obsGetNameReply = std::dynamic_pointer_cast<ObserverGetNameReply>(obs);
                if (obsGetNameReply)
                {
                    obsGetNameReply->notify(cmd);
                }
            }
        }
        break;
        default:
            //  logging here
            break;
        }

        //  this message has been processed
        //  remove it
        m_outQueue->pop();
    }
}