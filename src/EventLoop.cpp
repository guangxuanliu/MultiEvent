#include "EventLoop.h"

#include <event2/thread.h>

#include <signal.h>
#include <stdlib.h>

#include <iostream>
using std::cerr;
using std::endl;

EventLoop::EventLoop()
    : isRuning_(false)
{
    base_ = event_base_new();
    if (!base_)
    {
        cerr << "EventLoop init event_base error!" << endl;
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);
    evthread_use_pthreads();
};

EventLoop::~EventLoop()
{
    if (base_)
    {
        event_base_free(base_);
        isRuning_ = false;
    }
}

void EventLoop::loop()
{
    if (isRuning_)
    {
        return;
    }

    isRuning_ = true;
    event_base_loop(base_, EVLOOP_NO_EXIT_ON_EMPTY);
}