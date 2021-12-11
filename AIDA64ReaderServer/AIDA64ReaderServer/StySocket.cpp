#include "StySocket.h"
#include <iostream>
#include <msclr/marshal_cppstd.h>
using namespace msclr::interop;
/*
	变量定义区
*/
bool StySocket::m_inited = false;
std::string G_HTMLTEMPLATE = GetPageTemplate();


/*
	函数定义区
*/
std::string getFormattedTime(std::string format="%Y-%m-%d %H:%M:%S")
{
	time_t rawtime;
	struct tm info;
	char buffer[80];
	time(&rawtime);
	localtime_s(&info, &rawtime);
	strftime(buffer, 80, format.c_str(), &info);
	return buffer;
}

//子线程工作函数
void thFunc_Work(SOCKET*client,SOCKADDR_IN* clientAddr)
{
	std::cout << getFormattedTime() << "[INFO] Work Thread @" << std::this_thread::get_id() << " start" << std::endl;
	int ret = 1;
	while (ret != SOCKET_ERROR) {
		char recvBuf[1025]{0};
		char sendBuf[1025]{0};
		char clientIP[INET_ADDRSTRLEN] = "";
		inet_ntop(AF_INET, &clientAddr->sin_addr, clientIP, INET_ADDRSTRLEN);
		ret = recv(*client, recvBuf, 1024, 0);
		while (ret > 0)
		{
			//std::cout << getFormattedTime() << "[INFO] Client @" << clientIP << ":" << ntohs(clientAddr->sin_port) << " sent:" << std::endl;
			//for (int i = 0; i < ret; i++) std::cout << recvBuf[i];
			//std::cout << std::endl;
			//UTF8Encoding^ utf8 = gcnew UTF8Encoding();
			//Encoding^ gb2312 = Encoding::GetEncoding("gb2312");
			//auto gbkB = gb2312->GetBytes(Globals::AIDA64Data);
			//String^ u8Str = utf8->GetString(Encoding::Convert(gb2312, utf8, gbkB));
			//std::string data = marshal_as<std::string>( u8Str );
			std::string temp = recvBuf;
			std::string data = marshal_as<std::string>(Globals::AIDA64Data->ToString());
			std::string httpResp = "HTTP/1.1 200 OK\n";
			httpResp += "Server: AIDA64ReaderServer\n";
			//httpResp += "Refresh: 1";
			httpResp += "\n\n";
			std::regex e("(\\w+):\\s(.+)\r\n",std::regex_constants::icase);
			std::sregex_token_iterator varName(data.cbegin(), data.cend(), e, 1);// 0表示匹配结果的整个字符串，正则表达式中还有两个分组，如果参数为1表示第一个分组，以此类推
			std::sregex_token_iterator varVal(data.cbegin(), data.cend(), e, 2);
			std::sregex_token_iterator end;
			std::string renderedHtml = G_HTMLTEMPLATE;
			for (; varName != end; ++varName,++varVal)
			{
				std::string varTemplate = "{{";
				varTemplate += varName->str();
				varTemplate += "}}";
				std::string::size_type indexStart; //模板可替换变量的索引
				unsigned int lenVarTemplate;
				indexStart = renderedHtml.find(varTemplate);
				lenVarTemplate = varTemplate.length();
				while(indexStart != renderedHtml.npos)
				{
					renderedHtml.replace(indexStart, lenVarTemplate, varVal->str());
					indexStart = renderedHtml.find(varTemplate);
				}
			}
			httpResp += renderedHtml;
			//添加对浏览器请求的支持
			if (temp.length() > 3) {
				send(*client, httpResp.c_str(), httpResp.size(), 0);
				ret = SOCKET_ERROR; //disconnected with the http client
				break;
			}
			data = marshal_as<std::string>(Globals::AIDA64DataForChips->ToString());
			sprintf_s(sendBuf, "%s", data.c_str());
			send(*client, sendBuf, data.size(), 0);
			ret = recv(*client, recvBuf, 1024, 0);
		}
		std::cout << getFormattedTime() << "[INFO] Client @" << clientIP << ":" << ntohs(clientAddr->sin_port) << " disconnected." << std::endl;
		closesocket(*client);
	}
	std::cout << getFormattedTime() << "[INFO] Work Thread @" << std::this_thread::get_id() << " exit" << std::endl;
}
//子线程监听函数
void thFunc_Listen(StySocket* sc)
{
	std::cout << getFormattedTime() << "[INFO] Listening Thread @" << std::this_thread::get_id() << " start" << std::endl;
	SOCKADDR_IN clientAddr;
	unsigned long long cliaddr_len = sizeof(clientAddr);
	while (sc->m_sock!=INVALID_SOCKET)
	{
		SOCKET clientSock = accept(sc->m_sock, (SOCKADDR*)&clientAddr, (int*)&cliaddr_len);
		char clientIP[INET_ADDRSTRLEN] = "";
		inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
		std::cout << getFormattedTime() << "[INFO] Client @" << clientIP << ":" << ntohs(clientAddr.sin_port) << " connected!" << std::endl;
		std::thread tTask = std::thread(thFunc_Work, &clientSock,&clientAddr);
		tTask.detach();
	}
}

std::string GetPageTemplate()
{
	static std::string ret;
	ret.clear();
	std::ifstream ifs("template.html");
	char c;
	while (ifs.get(c))
	{
		ret += c;
	}
	ifs.close();
	return ret;
}

/*
	类方法定义区
*/
bool StySocket::inited()
{
	if (m_inited == false)
	{
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);//用于检测函数状态
		if (iResult != 0) {
			return false;
		}
		m_inited = true;
	}
	return true;
}

bool StySocket::cleanUp()
{
	if (m_inited == true)
	{
		int iResult = WSACleanup();
		if (iResult != 0) {
			return false;
		}
		m_inited = false;
	}
	return true;
}

bool StySocket::startListening(int port)
{
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	//绑定
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//ip地址
	addr.sin_port = htons(port);//绑定端口
	//将套接字绑定到指定的网络地址
	int ret = bind(m_sock, (SOCKADDR*)&addr, sizeof(SOCKADDR));
	if (ret)
	{
		std::cout << "Bind Failed!" << std::endl;
		return false;
	}
	ret = listen(m_sock, 10);
	if (ret)
	{
		std::cout << "Listen Failed!" << std::endl;
		return false;
	}
	std::thread tListening = std::thread(thFunc_Listen,this);
	tListening.detach();
	return true;
}

int StySocket::sendMsg(void* msg, int msglen)
{
	return 0;
}

StySocket::StySocket()
{
	this->inited();
}

StySocket::~StySocket()
{
	this->cleanUp();
}
