#include "EventLoop.h"
#include "TcpClient.h"
#include "TcpConnection.h"

#include <unistd.h>

#include <iostream>
#include <string>
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
    // char buf[len + 1] = {0};
    // conn->read(buf, len);
    // cout << "客户端(" << conn->remoteIp_ << ":" << conn->remotePort_ << "):" << buf << endl;

    // // 回复
    // conn->write(buf, len);
}

void writeCompleteCb(TcpConnection *conn)
{
    cout << "发送完成回调，客户端(" << conn->remoteIp_ << ":" << conn->remotePort_ << ")" << endl;
}

void cmdMsgCb(int fd, short events, void* arg)
{
    char buf[BUFSIZ];

    if(fgets(buf, BUFSIZ, stdin) == NULL)
    {
        return;
    }

    TcpClient *client = (TcpClient*)arg;

    string msg(buf, strlen(buf) - 1);
    if(msg == "disconnect")
    {
        client->getConnection()->disconnect();
    }
    else if(msg == "connect")
    {
        client->getConnection()->connect();
    }
    else
    {
        client->getConnection()->write(msg);
    }
}


int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        cerr << "usage:" << argv[0] << " remoteIp remotePort" << endl;
        exit(0);
    }

    string remoteIp = argv[1];
    int remotePort = atoi(argv[2]);

    EventLoop loop;
    TcpClient client(loop, remoteIp, remotePort);

    client.setConnectionCb(connectionCb);
    client.setMessageCb(messageCb);

    if (client.connect())
    {
        cout << "连接服务端(" << remoteIp << ")成功" << endl;
    }
    else
    {
        cerr << "连接服务端(" << remoteIp << ")失败，原因：" << client.getErrorString() << endl;
    }

    //监听终端输入事件
    struct event* evCmd = event_new(loop.getEventBase(), STDIN_FILENO,
                                     EV_READ | EV_PERSIST,
                                     cmdMsgCb, &client);

    event_add(evCmd, NULL);

    loop.loop();

    return 0;
}