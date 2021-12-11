#include "StySocket.h"
using std::cout;
using std::cin;
using std::endl;

/*
    全局变量定义区
*/
StySocket server = StySocket();


/*
    类方法定义区
*/
bool StySocket::SocketInit()
{

}

bool StySocket::SocketRelease()
{

}

StySocket::StySocket()
{
    if(!m_inited) this->SocketInit();
    cout << "[INFO]Server inited!" << endl;
}

StySocket::~StySocket()
{
    if(m_inited) this->SocketRelease();
}