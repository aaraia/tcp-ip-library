#pragma once

#include "protocol.h"

//  stl
#include <memory>

class ProtocolA;
using ProtocolAPtr = std::shared_ptr<ProtocolA>;

class ProtocolA : public Protocol
{
public:
    ProtocolA();
    ~ProtocolA();

    void send(const std::string& msg) override;
    void receive(const std::string& msg) override;
};