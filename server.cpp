#include "EventLoop.h"
#include "TcpServer.h"
#include "TcpConnection.h"

#include <stdlib.h>

#include <iostream>
using namespace std;

void connectionCb(TcpConnection *conn)
{
    if (conn->isServer())
    {
        if (conn->isConnect_)
        {
            cout << "来自(" << conn->remoteIp_ << ":" << conn->remotePort_ << ")的新连接..." << endl;
        }
        else
        {
            cout << "客户端(" << conn->remoteIp_ << ":" << conn->remotePort_ << ")断开连接" << endl;
        }
    }

    if (conn->isClient())
    {
        if (conn->isConnect_)
        {
            cout << "连接上服务端(" << conn->remoteIp_ << ":" << conn->remotePort_ << ")" << endl;
        }
        else
        {
            cout << "与服务端(" << conn->remoteIp_ << ":" << conn->remotePort_ << ")断开连接" << endl;
        }
    }
}

void messageCb(TcpConnection *conn, int len)
{
    char buf[len + 1] = {0};
    conn->read(buf, len);
    cout << "客户端(" << conn->remoteIp_ << ":" << conn->remotePort_ << ")， 长度:" << len << endl;

    // 回复
    conn->write(buf, len);
}

void writeCompleteCb(TcpConnection *conn)
{
    cout << "发送完成回调，客户端(" << conn->remoteIp_ << ":" << conn->remotePort_ << ")" << endl;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "usage:" << argv[0] << " listenPort threadCount" << endl;
        exit(0);
    }

    int listenPort = atoi(argv[1]);
    int threadCount = atoi(argv[2]);

    EventLoop loop;
    TcpServer server(&loop, listenPort);

    server.setConnectionCb(connectionCb);
    server.setMessageCb(messageCb);
    server.setThreadNum(threadCount);

    if (!server.start())
    {
        cerr << "服务端监听端口" << listenPort << "失败，原因：" << server.getErrorString() << endl;
        exit(0);
    }
    else
    {
        cout << "服务端正在监听端口：" << listenPort << endl;
    }

    loop.loop();

    exit(0);
}