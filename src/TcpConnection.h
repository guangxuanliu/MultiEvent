#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "nocopyable.h"
#include "CallBacks.h"

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/util.h>

#include <string.h>

#include <string>
using std::string;

class TcpConnection : private nocopyable
{
public:
    enum TypeE
    {
        kServer,
        kClient
    }; // 服务端，客户端

    // 做为服务端
    TcpConnection(event_base *base, evutil_socket_t fd, const string &remoteIp, int remotePort,
                  MessageCb messageCb, WriteCompleteCb writeCompleteCb, ConnectionCb connectionCb);

    // 做为客户端
    TcpConnection(event_base *base, const string &remoteIp, int remotePort,
                  MessageCb messageCb, WriteCompleteCb writeCompleteCb, ConnectionCb connectionCb);

    ~TcpConnection();

    int bytesAvaliable();

    void write(void *buf, int len);
    void write(const string &str);
    int read(void *buf, int len);

    bool isServer() { return type_ == kServer; }
    bool isClient() { return type_ == kClient; }

    // 客户端接口
    bool connect();
    void disconnect();

    string getErrorString()
    {
        return evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
    }

    void setMessageCb(const MessageCb &cb) { messagecb_ = cb; }
    void setWriteCompleteCb(const WriteCompleteCb &cb) { writeCompleteCb_ = cb; }
    void setConnectionCb(const ConnectionCb &cb) { connectionCb_ = cb; }

    MessageCb &getMessageCb() { return messagecb_; }
    WriteCompleteCb &getWriteCompleteCb() { return writeCompleteCb_; }
    ConnectionCb &getConnectionCb() { return connectionCb_; }

    static void readCb(struct bufferevent *bev, void *arg);
    static void writeCb(struct bufferevent *bev, void *arg);
    static void eventCb(struct bufferevent *bev, short events, void *arg);

    bool isConnect_;
    string remoteIp_;
    int remotePort_;

private:
    event_base *base_; // base是传进来的，所以此处不负责释放。
    evutil_socket_t fd_;
    bufferevent *bev_;

    MessageCb messagecb_;
    WriteCompleteCb writeCompleteCb_;
    ConnectionCb connectionCb_;

    TypeE type_;
};

#endif