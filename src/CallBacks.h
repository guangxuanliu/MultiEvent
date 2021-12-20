#ifndef CALLBACKS_H
#define CALLBACKS_H

class TcpConnection;

typedef void(*MessageCb)(TcpConnection *conn, int len);       // 读回调
typedef void(*WriteCompleteCb)(TcpConnection *conn);          // 写完成回调，当写缓冲区清空的时候调用此回调
typedef void(*ConnectionCb)(TcpConnection *conn);             // 连接、异常回调。连接建立，连接断开时调用此回调

#endif