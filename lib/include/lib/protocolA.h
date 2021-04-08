#pragma once

#include "protocol.h"

//  stl
#include <memory>

//  forwards
class ProtocolA;
using ProtocolAPtr = std::shared_ptr<ProtocolA>;

class Observer;
using ObserverPtr = std::shared_ptr<Observer>;

using ObserverArray = std::vector<ObserverPtr>;
using ObserverHashTable = std::unordered_map<uint64_t, ObserverArray>;

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
    ~ProtocolA();

    void receive(message::Message&& msg) override;

private:
    void notifyObserversThread();

private:
    std::queue<message::Message> m_queueOne;
    std::queue<message::Message> m_queueTwo;
    std::queue<message::Message>* m_inQueue = &m_queueOne;
    std::queue<message::Message>* m_outQueue = &m_queueTwo;
    //TODO: next step is to for observers for each type of command
    ObserverHashTable m_observers;
    std::mutex m_mutex;
    std::thread m_notifyThread;
};