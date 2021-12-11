#include <WinSock2.h>
#include <iostream>

/*
    前置声明区
*/
extern class StySocket;

/*
    全局变量声明区
*/
extern StySocket server;

/*
    类声明区
*/
class StySocket{
private:
    static bool m_inited;
    SOCKET m_sock;
    bool SocketInit();
    bool SocketRelease();
public:
    StySocket();
    ~StySocket();

};