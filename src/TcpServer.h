#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "nocopyable.h"

#include "CallBacks.h"
#include "EventLoop.h"

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/util.h>

#include <string.h>

#include <vector>
#include <string>
using std::vector;
using std::string;

struct ThreadInfo;

class TcpServer : private nocopyable
{
public:
    TcpServer(EventLoop *loop, int listenPort);
    ~TcpServer();

    bool setThreadNum(int count);

    event_base *distribute();

    bool start();
    
    string getErrorString()
    {
        return evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
    }

    static void listenCb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *arg);
    static void acceptErrorCb(struct evconnlistener *listener, void *ctx);

    static void *startRoutine(void *arg);

    void setMessageCb(const MessageCb &cb) { messagecb_ = cb; }
    void setWriteCompleteCb(const WriteCompleteCb &cb) { writeCompleteCb_ = cb; }
    void setConnectionCb(const ConnectionCb &cb) { connectionCb_ = cb; }

    MessageCb &getMessageCb() { return messagecb_; }
    WriteCompleteCb &getWriteCompleteCb() { return writeCompleteCb_; }
    ConnectionCb &getConnectionCb() { return connectionCb_; }

private:
    event_base *base_; // 和EventLoop共享同一个base，EventLoop负责释放自己的base
    evconnlistener *listener_;

    int listenPort_;
    int threadNum_;
    vector<ThreadInfo *> basePool_;
    bool isRunning_;

    MessageCb messagecb_;
    WriteCompleteCb writeCompleteCb_;
    ConnectionCb connectionCb_;
};

// 线程的信息
struct ThreadInfo
{
    event_base *base_;
    pthread_t tid_;
};

#endif