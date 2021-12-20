#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <event2/event.h>

#include "nocopyable.h"

class EventLoop : private nocopyable
{
public:
    EventLoop();
    ~EventLoop();

    void loop();

    event_base* getEventBase()
    {
        return base_;
    }

private:
    event_base *base_;
    bool isRuning_;
};

#endif