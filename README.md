# MultiEvent
## 介绍
libevent的简单封装，既可以作为客户端，又可以作为服务端。作为服务端时可以设置工作线程的个数。

## 使用方法
- 先源码编译libevent。
- 客户端使用方法参考[client.cpp](client.cpp)，服务端使用方法参考[server.cpp](server.cpp)

## 注意
- 粘包情况：需要由用户自行处理。