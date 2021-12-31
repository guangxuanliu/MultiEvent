#include "TcpServer.h"
#include "TcpConnection.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>

#include <iostream>
using std::cerr;
using std::endl;

#include <string>
using std::string;

TcpServer::TcpServer(EventLoop *loop, int listenPort)
    : base_(loop->getEventBase()), listenPort_(listenPort)
{
    srand(time(NULL));

    listener_ = NULL;
    threadNum_ = 0;
    basePool_.clear();
    isRunning_ = false;

    messagecb_ = NULL;
    writeCompleteCb_ = NULL;
    connectionCb_ = NULL;
}

TcpServer::~TcpServer()
{
    if (listener_)
    {
        evconnlistener_free(listener_);
    }

    for (int i = 0; i < basePool_.size(); ++i)
    {
        ThreadInfo *tInfo = basePool_[i];

        if (event_base_loopbreak(tInfo[i].base_) == 0)
        {
            event_base_free(tInfo[i].base_);
            pthread_detach(tInfo[i].tid_);
        }
        else
        {
            cerr << "loop break error!" << endl;
        }
    }

    basePool_.clear();
}

bool TcpServer::setThreadNum(int count)
{

    // 目前仅仅允许设置一次，后期可以改进为允许动态的扩展线程个数
    if (basePool_.size() > 0)
    {
        return false;
    }

    for (int i = 0; i < count; ++i)
    {
        event_base *base = event_base_new();
        if (base)
        {
            ThreadInfo *info = new ThreadInfo();
            info->base_ = base;
            basePool_.push_back(info);
        }
    }

    for (int i = 0; i < basePool_.size(); ++i)
    {
        int ret = pthread_create(&(basePool_[i]->tid_), NULL, startRoutine, basePool_[i]);
        if (ret != 0)
        {
            cerr << "create thread error, ret value:" << ret << endl;
            continue;
        }
    }
    return true;
}

void *TcpServer::startRoutine(void *arg)
{
    ThreadInfo *info = (ThreadInfo*)arg;
    if(info == NULL)
        return NULL;

    event_base_loop(info->base_, EVLOOP_NO_EXIT_ON_EMPTY);

    return NULL;
}

event_base *TcpServer::distribute()
{
    // 如果线程个数为0， 则和主线程共享同一个base
    if (basePool_.size() == 0)
    {
        return base_;
    }

    int index = rand() % basePool_.size();
    return basePool_[index]->base_;
}

bool TcpServer::start()
{
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    /* Listen on 0.0.0.0 */
    sin.sin_addr.s_addr = htonl(0);
    sin.sin_port = htons(listenPort_);

    // If backlog is negative, Libevent tries to pick a good value for the backlog
    listener_ = evconnlistener_new_bind(base_, listenCb, this,
                                        LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                        (struct sockaddr *)&sin, sizeof(sin));
    if (!listener_)
    {
        cerr << "can not create listener!" << endl;
        return false;
    }
    evconnlistener_set_error_cb(listener_, acceptErrorCb);

    return true;
}

void TcpServer::listenCb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *arg)
{

    // 获取对方的IP和端口
    char ip[INET_ADDRSTRLEN];
    evutil_inet_ntop(AF_INET, &(((sockaddr_in*)address)->sin_addr), ip, sizeof(ip));
    string ipStr = ip;

    int port = ntohs(((sockaddr_in*)address)->sin_port);

    // 分配一个event_base
    TcpServer *server = (TcpServer *)arg;
    event_base *base = server->distribute();

    // 创建TcpConnection
    // todo: 应该把所有的客户端连接都管理起来
    TcpConnection *conn = new TcpConnection(base, fd, ipStr, port,
                                            server->getMessageCb(), server->getWriteCompleteCb(), server->getConnectionCb());

    // 触发用户的连接回调
    if (server->getConnectionCb())
    {
        server->getConnectionCb()(conn);
    }
}

void TcpServer::acceptErrorCb(struct evconnlistener *listener, void *ctx)
{
    cerr << "accept error:" << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << endl;
}