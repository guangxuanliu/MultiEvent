#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "EventLoop.h"
#include "TcpConnection.h"

#include <event2/event.h>

#include <string>
using std::string;

class TcpClient
{
public:
    TcpClient(EventLoop &loop, const string &remoteIp, int remotePort);
    ~TcpClient();

    bool connect();
    void disconnect();

    TcpConnection* getConnection()
    {
        if(conn)
        {
            return conn;
        }
        return NULL;
    }

    void setMessageCb(const MessageCb &cb)
    {
        messagecb_ = cb;
        if (conn)
        {
            conn->setMessageCb(messagecb_);
        }
    }
    void setWriteCompleteCb(const WriteCompleteCb &cb)
    {
        writeCompleteCb_ = cb;
        if (conn)
        {
            conn->setWriteCompleteCb(writeCompleteCb_);
        }
    }
    void setConnectionCb(const ConnectionCb &cb)
    {
        connectionCb_ = cb;
        if (conn)
        {
            conn->setConnectionCb(connectionCb_);
        }
    }

    MessageCb &getMessageCb() { return messagecb_; }
    WriteCompleteCb &getWriteCompleteCb() { return writeCompleteCb_; }
    ConnectionCb &getConnectionCb() { return connectionCb_; }

    string getErrorString()
    {
        return evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
    }

private:
    event_base *base_;
    TcpConnection *conn;

    string remoteIp_;
    int remotePort_;

    MessageCb messagecb_;
    WriteCompleteCb writeCompleteCb_;
    ConnectionCb connectionCb_;
};

#endif