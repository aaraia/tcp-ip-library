#pragma once

//  lib
#include "observer.h"
#include "get_name_cmd.pb.h"

//  forwards
class ObserverGetNameCmd;
using ObserverGetNameCmdPtr = std::shared_ptr<ObserverGetNameCmd>;

class ObserverGetNameCmd : public Observer
{
public:
    ObserverGetNameCmd() {}
    virtual ~ObserverGetNameCmd() {}

    //  leave it to the client/server to write its own implementation
    virtual void notify(const message::GetNameCMD& cmd) = 0;
};