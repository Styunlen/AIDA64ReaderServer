#pragma once
#include <winsock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <iostream>
#include <fstream>
#include <regex>
using namespace System;
using namespace System::Text;
#pragma comment (lib,"ws2_32")
/*
	前置声明区
*/
extern class StySocket;
/*
	变量声明区
*/
ref struct Globals {

	static String^ AIDA64Data = String::Empty;
	static String^ AIDA64DataForChips = String::Empty;

};
extern std::string G_HTMLTEMPLATE;


/*
	函数声明区
*/
void thFunc_Listen(StySocket * sc);
std::string GetPageTemplate();

/*
	类声明区
*/
class StySocket
{
private:
	SOCKET m_sock = INVALID_SOCKET;
	static bool m_inited;
public:
	//inited the win32 lib of socket
	bool inited();
	bool cleanUp();
	bool startListening(int port);
	friend void thFunc_Listen(StySocket* sc);
	//sendMsg to the client
	int sendMsg(void* msg, int msglen);
	StySocket();
	~StySocket();
};

