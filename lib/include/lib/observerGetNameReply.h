#pragma once


//  lib
#include "observer.h"
#include "get_name_reply.pb.h"

//  forwards
class ObserverGetNameReply;
using ObserverGetNameReplyPtr = std::shared_ptr<ObserverGetNameReply>;

class ObserverGetNameReply : public Observer
{
public:
    ObserverGetNameReply() {}
    virtual ~ObserverGetNameReply() {}

    //  leave it to the client/server to write its own implementation
    virtual void notify(const message::GetNameReply& cmd) = 0;
};