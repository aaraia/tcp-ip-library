#pragma once

//  lib
#include "protocol.h"

//  stl
#include <memory>

//  forwards
class ProtocolA;
using ProtocolAPtr = std::shared_ptr<ProtocolA>;

class Observer;
using ObserverPtr = std::shared_ptr<Observer>;

using ObserverArray = std::vector<ObserverPtr>;
using ObserverHashTable = std::unordered_map<uint64_t, ObserverArray>;  //  id of command, list of observers for that command

class ProtocolA : public Protocol
{
    static const uint64_t PROTOCOL_A = 1;

public:
    enum CMD_ID
    {
        GET_NAME_CMD,
        GET_NAME_REPLY
    };


public:
    ProtocolA();
    virtual ~ProtocolA();

    void receive(message::Message&& msg) override;

private:
    void notifyObserversThread();

private:
    std::queue<message::Message> m_queueOne;
    std::queue<message::Message> m_queueTwo;
    std::queue<message::Message>* m_inQueue = &m_queueOne;
    std::queue<message::Message>* m_outQueue = &m_queueTwo;
    //  NOTE: perhaps better to store concrete type of observer
    //  as the commands of the protocol will be known at compile time
    ObserverHashTable m_observers;
    std::mutex m_mutex;
    std::thread m_notifyThread;
};