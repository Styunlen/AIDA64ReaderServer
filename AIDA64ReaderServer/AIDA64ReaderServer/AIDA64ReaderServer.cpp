// AIDA64ReaderServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#define _WINSOCKAPI_ 
#include <windows.h>
#include <thread>
#include <msclr/marshal_cppstd.h>
#include "StySocket.h"
using namespace msclr::interop;
using namespace System;
using namespace Microsoft::Win32;
using namespace System::IO;
#using "System.core.dll"
#using "System.Xml.Linq.dll"
using namespace System::IO::MemoryMappedFiles;
using namespace System::Xml::Linq;

//move window to the right bottom corner of the screen
void moveWindowToRB()
{
    WINDOWINFO wi;
    HWND hwnd = GetConsoleWindow();
    GetWindowInfo(hwnd, &wi);
    SIZE windowSize, screenSize, pos;
    windowSize.cx = wi.rcWindow.right - wi.rcWindow.left;
    windowSize.cy = wi.rcWindow.bottom - wi.rcWindow.top;
    screenSize.cx = GetSystemMetrics(SM_CXSCREEN);
    screenSize.cy = GetSystemMetrics(SM_CYSCREEN);
    pos.cx = (screenSize.cx - windowSize.cx);
    pos.cy = (screenSize.cy - windowSize.cy - 50);
    /*std::cout << "width: \t" << windowSize.cx << "\theight: \t" << windowSize.cy << std::endl;
    std::cout << "SWidth: \t" << screenSize.cx << "\tSHeight: \t" << screenSize.cy << std::endl;
    std::cout << "x: \t" << wi.rcWindow.left << "\ty: \t" << wi.rcWindow.top << std::endl;
    std::cout << "sx: \t" << pos.cx << "\tsy: \t" << pos.cy << std::endl;*/
    MoveWindow(hwnd,pos.cx, pos.cy, windowSize.cx, windowSize.cy, true);
}

void moveChild(HWND child, POINT pos)
{
    WINDOWINFO wi;
    ZeroMemory(&wi, sizeof wi);
    GetWindowInfo(child, &wi);
    SIZE windowSize, parentPos;
    windowSize.cx = wi.rcWindow.right - wi.rcWindow.left;
    windowSize.cy = wi.rcWindow.bottom - wi.rcWindow.top;
    parentPos.cx = wi.rcWindow.left;
    parentPos.cy = wi.rcWindow.top;
    /*std::cout << "width: \t" << windowSize.cx << "\theight: \t" << windowSize.cy << std::endl;
    std::cout << "SWidth: \t" << screenSize.cx << "\tSHeight: \t" << screenSize.cy << std::endl;
    std::cout << "x: \t" << wi.rcWindow.left << "\ty: \t" << wi.rcWindow.top << std::endl;
    std::cout << "sx: \t" << pos.cx << "\tsy: \t" << pos.cy << std::endl;*/
    MoveWindow(child, pos.x, pos.y, windowSize.cx, windowSize.cy, true);
}

void thFunc_aida64Reader() {
    Console::WriteLine("Hello,World!");
    Console::Write("Thread Aida64Reader is working! @");
    std::cout << std::this_thread::get_id() << std::endl;
    //等待AIDA64初始化完毕后再启动服务器
    while (true)
    {
        try {
            MemoryMappedFile::OpenExisting("AIDA64_SensorValues");
            break;
        }
        catch (FileNotFoundException^ ex)
        {
            std::cout << "AIDA64_SensorValues might be loading..." << std::endl;
            std::cout << "Wait for another try after 5s" << std::endl;
            Sleep(5000);
        }
    }
    StySocket server = StySocket();
    server.startListening(50123);
    while (true)
    {
        //system("cls");
        String^ tempString = String::Empty;
        tempString += "<AIDA64>";
        MemoryMappedFile^ mmf;
        while (true)
        {
            try {
                mmf = MemoryMappedFile::OpenExisting("AIDA64_SensorValues");
                break;
            }
            catch (FileNotFoundException^ ex)
            {
                std::cout << "AIDA64_SensorValues might be loading..." << std::endl;
                std::cout << "Wait for another try after 5s" << std::endl;
                Sleep(5000);
            }
        }
        MemoryMappedViewAccessor^ accessor = mmf->CreateViewAccessor();
        tempString = tempString + "";
        for (int i = 0; i < accessor->Capacity; i++)
        {
            tempString = tempString + (Char)accessor->ReadByte(i);
        }
        tempString = tempString->Replace("\0", "");
        tempString = tempString + "";

        tempString += "</AIDA64>";
        //Console::WriteLine(tempString);
        /*

        <AIDA64><sys><id>SBATTLVL</id><label>Battery Level</label><value>53</value></sys><sys><id>SBATTWEARLVL</id><label>Battery Wear Level</label><value>3</value></sys><sys><id>SBATT</id><label>Battery</label><value>1:41:42</value></sys><sys><id>SPWRSTATE</id><label>Power State</label><value>Discharging</value></sys><pwr><id>PBATTCHR</id><label>Battery Charge Rate</label><value>-29.89</value></pwr><pwr><id>PGPU1</id><label>GPU1</label><value>9.72</value></pwr></AIDA64>
         */
        XDocument^ aidaXML;
        try {
            aidaXML = XDocument::Parse(tempString);
        }
        catch (Exception^ ex) {
            /*Console::WriteLine("————————");
            Console::WriteLine(tempString);
            Console::WriteLine("————————");*/
            String^ xmlTestStr = gcnew String("");
            xmlTestStr += "<AIDA64><sys><id>Loading</id><value>Loading.</value></sys>";
            xmlTestStr += "<pwr><id>Loading</id><value>Loading..</value></pwr></AIDA64>";
            aidaXML = XDocument::Parse(xmlTestStr);
        }
        auto sysElements = aidaXML->Element("AIDA64")->Elements("sys");
        auto pwrElements = aidaXML->Element("AIDA64")->Elements("pwr");
        auto i = sysElements->GetEnumerator();
        i->MoveNext();
        String^ retChips = gcnew String(""); //为单片机显示提供支持
        String^ ret = gcnew String("");
        String^ valueID = i->Current->Element("id")->Value;
        String^ valueVal = i->Current->Element("value")->Value;
        do {
            valueID = i->Current->Element("id")->Value;
            valueVal = i->Current->Element("value")->Value;
            ret += String::Format("{0}: {1}\r\n", valueID, valueVal);
            if (valueID == "SBATTLVL")
            {
                retChips += "Percentage: " + valueVal + " %\r\n";
            }
            else if (valueID == "SBATT") {
                if (valueVal == "Discharging")
                {
                    valueVal = "Powering";
                }
                retChips += "Remain: " + valueVal + "\r\n";
            }
            //String("SBATT"):
        } while (i->MoveNext());
        i = pwrElements->GetEnumerator();
        i->MoveNext();
        do {
            valueID = i->Current->Element("id")->Value;
            valueVal = i->Current->Element("value")->Value;
            ret += String::Format("{0}: {1}\r\n", valueID, valueVal);
            if (valueID == "PBATTCHR")
            {
                retChips += "Power: " + valueVal + " W/h\r\n";
            }
            else {
                //"PGPU1"
                retChips += "GPU: " + valueVal + " W/h\r\n";
            }
            //String("SBATT"):
        } while (i->MoveNext());
        //Console::WriteLine(ret);
        Globals::AIDA64DataForChips = retChips;
        Globals::AIDA64Data = ret;
        Sleep(1000);
    }
}

int main()
{
    system("title AIDA64ReaderServer");
    system("mode con Cols=50");
    system("mode con Lines=10");
    //设置控制台，禁用编辑
    auto hStdin = ::GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    ::GetConsoleMode(hStdin, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    ::SetConsoleMode(hStdin, mode);
    moveWindowToRB();
    std::thread thReader = std::thread(thFunc_aida64Reader);
    thReader.detach();
    HWND hWnd = GetConsoleWindow();
    HFONT font = CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"微软雅黑");
    HWND b1 = CreateWindowW(
        L"BUTTON",   // 类
        L"重载模板页",       // 文字
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,
        0,         // x
        0,         // y
        100,        // 长
        60,        // 宽
        hWnd,       // 上级
        0,       // ID
        (HINSTANCE)GetWindowLongW(hWnd, GWLP_HINSTANCE),
        NULL);
    HWND b2 = CreateWindowW(
        L"BUTTON",   // 类
        L"关闭服务器",       // 文字
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,
        0,         // x
        0,         // y
        100,        // 长
        60,        // 宽
        hWnd,       // 上级
        0,       // ID
        (HINSTANCE)GetWindowLongW(hWnd, GWLP_HINSTANCE),
        NULL);
    SendMessage(b1, WM_SETFONT, (WPARAM)font, 1);
    ShowWindow(b1, 1);
    SendMessage(b2, WM_SETFONT, (WPARAM)font, 1);
    ShowWindow(b2, 1);
    WINDOWINFO wi;
    ZeroMemory(&wi, sizeof wi);
    GetWindowInfo(hWnd, &wi);
    POINT pos = { wi.rcWindow.right,wi.rcWindow.bottom };
    pos.y -= 120;
    ScreenToClient(hWnd, &pos);
    moveChild(b1, pos);
    pos = { wi.rcWindow.right,wi.rcWindow.bottom };
    pos.y -= 60;
    ScreenToClient(hWnd, &pos);
    moveChild(b2, pos);
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.hwnd == b1) {
            switch (msg.message)
            {
            case WM_LBUTTONDOWN:
                G_HTMLTEMPLATE = GetPageTemplate();
                std::cout << "重载成功！" << std::endl;
                MessageBoxW(hWnd, L"重载成功！", L"信息", MB_ICONINFORMATION);
                break;
            case WM_PAINT:
                DefWindowProc(b1, msg.message, msg.wParam, msg.lParam);
                break;
            default:
                DefWindowProc(b1, msg.message, msg.wParam, msg.lParam);
                break;
            }

        }
        else if (msg.hwnd == b2) {
            if (msg.message == WM_LBUTTONDOWN) 
            { 
                MessageBoxW(hWnd, L"单击确定，服务器将在三秒后退出！", L"信息", MB_ICONINFORMATION);
                break; 
            }
        }else {
            std::cout << msg.hwnd << ' ' << std::hex << msg.message << std::endl;
        }
    }
    /*std::string command;
    std::getline(std::cin, command);
    while (_stricmp(command.c_str(),"quit"))
    {
        if (_stricmp(command.c_str(), "reload"))
        {
            GetPageTemplate();
            std::cout << "重载成功！" << std::endl;
        }
        else {
            std::cout << "未知指令！" << std::endl;
        }
        std::getline(std::cin, command);
    }*/
    std::cout << "See you World!\n";
    Sleep(3000);
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
