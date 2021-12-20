#include "TcpConnection.h"

#include <event2/event.h>
#include <event2/util.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <string.h>
#include <arpa/inet.h>

#include <iostream>
using std::cerr;
using std::endl;

TcpConnection::TcpConnection(event_base *base, evutil_socket_t fd, const string &remoteIp, int remotePort,
                             MessageCb messageCb, WriteCompleteCb writeCompleteCb, ConnectionCb connectionCb)
    : base_(base), fd_(fd), remoteIp_(remoteIp), remotePort_(remotePort), messagecb_(messageCb), writeCompleteCb_(writeCompleteCb), connectionCb_(connectionCb)
{
    type_ = kServer;
    isConnect_ = true;

    bev_ = bufferevent_socket_new(base_, fd_, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
    if (bev_ == NULL)
    {
        cerr << "new bufferevent error!" << endl;
        return;
    }

    bufferevent_setcb(bev_, readCb, writeCb, eventCb, this);

    bufferevent_enable(bev_, EV_READ | EV_WRITE | EV_PERSIST);
}

TcpConnection::TcpConnection(event_base *base, const string &remoteIp, int remotePort,
                             MessageCb messageCb, WriteCompleteCb writeCompleteCb, ConnectionCb connectionCb)
    : base_(base), remoteIp_(remoteIp), remotePort_(remotePort), messagecb_(messageCb), writeCompleteCb_(writeCompleteCb), connectionCb_(connectionCb)
{
    type_ = kClient;

    bev_ = NULL;

    isConnect_ = false;
}

TcpConnection::~TcpConnection()
{
    if (bev_)
    {
        bufferevent_free(bev_);
        bev_ = NULL;
    }
}

int TcpConnection::bytesAvaliable()
{
    evbuffer *input = bufferevent_get_input(bev_);
    return evbuffer_get_length(input);
}

void TcpConnection::write(void *buf, int len)
{
    if (bev_)
        bufferevent_write(bev_, buf, len);
}

void TcpConnection::write(const string &str)
{
    write((void *)(str.c_str()), str.length());
}

int TcpConnection::read(void *buf, int len)
{
    if (bev_)
        return bufferevent_read(bev_, buf, len);

    return 0;
}

bool TcpConnection::connect()
{
    if (isConnect_)
        return true;

    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(remoteIp_.c_str());
    sin.sin_port = htons(remotePort_);

    if (bev_)
    {
        bufferevent_free(bev_);
    }

    bev_ = bufferevent_socket_new(base_, -1, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev_, readCb, writeCb, eventCb, this);

    bufferevent_enable(bev_, EV_READ | EV_WRITE | EV_PERSIST);

    if (bufferevent_socket_connect(bev_, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        bufferevent_free(bev_);
        return false;
    }

    fd_ = bufferevent_getfd(bev_);

    isConnect_ = true;
    return true;
}

void TcpConnection::disconnect()
{
    if (isConnect_)
    {
        bufferevent_free(bev_);
        bev_ = NULL;

        isConnect_ = false;
    }
}

void TcpConnection::readCb(struct bufferevent *bev, void *arg)
{
    TcpConnection *conn = (TcpConnection *)arg;

    if (conn->getMessageCb())
    {
        conn->getMessageCb()(conn, conn->bytesAvaliable());
    }
}

void TcpConnection::writeCb(struct bufferevent *bev, void *arg)
{
    TcpConnection *conn = (TcpConnection *)arg;

    if (conn->getWriteCompleteCb())
    {
        conn->getWriteCompleteCb()(conn);
    }
}

void TcpConnection::eventCb(struct bufferevent *bev, short events, void *arg)
{
    TcpConnection *conn = (TcpConnection *)arg;

    // 建立连接
    if (events & BEV_EVENT_CONNECTED)
    {
        conn->isConnect_ = true;
        if (conn->getConnectionCb())
        {
            conn->getConnectionCb()(conn);
        }
    }
    // 对方主动断开连接
    else if (events & (BEV_EVENT_READING | BEV_EVENT_EOF))
    {
        conn->isConnect_ = false;
        if (conn->getConnectionCb())
        {
            conn->getConnectionCb()(conn);
        }

        delete conn;
    }
    // 异常信息
    else if (events & BEV_EVENT_ERROR)
    {
        conn->isConnect_ = false;
        if (conn->getConnectionCb())
        {
            conn->getConnectionCb()(conn);
        }

        delete conn;
    }
    else
    {
        conn->isConnect_ = false;
        cerr << "connected error, events:" << events << ", reason:" << conn->getErrorString() << endl;
        if (conn->getConnectionCb())
        {
            conn->getConnectionCb()(conn);
        }

        delete conn;
    }
}