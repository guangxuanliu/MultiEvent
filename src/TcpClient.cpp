#include "TcpClient.h"

TcpClient::TcpClient(EventLoop &loop, const string &remoteIp, int remotePort)
    :base_(loop.getEventBase()), remoteIp_(remoteIp), remotePort_(remotePort)
{
    messagecb_ = NULL;
    writeCompleteCb_ = NULL;
    connectionCb_ = NULL;
    
    conn = new TcpConnection(base_, remoteIp_, remotePort_, messagecb_, writeCompleteCb_, connectionCb_);
}

TcpClient::~TcpClient()
{
    if(conn)
    {
        delete conn;
    }
}

bool TcpClient::connect()
{
    return conn->connect();
}

void TcpClient::disconnect()
{
    return conn->disconnect();
}