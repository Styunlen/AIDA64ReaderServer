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
	ǰ��������
*/
extern class StySocket;
/*
	����������
*/
ref struct Globals {

	static String^ AIDA64Data = String::Empty;
	static String^ AIDA64DataForChips = String::Empty;

};
extern std::string G_HTMLTEMPLATE;


/*
	����������
*/
void thFunc_Listen(StySocket * sc);
std::string GetPageTemplate();

/*
	��������
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

