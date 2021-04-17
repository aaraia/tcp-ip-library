#pragma once

//  stl
#include <memory>

class Observer : std::enable_shared_from_this<Observer>
{
public:
    Observer() {}
    virtual ~Observer() = 0 {}
};