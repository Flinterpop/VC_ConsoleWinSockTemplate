// ConsoleTemplate.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#define BMG_VERSION 1.0

#include <iostream>
#include <vector>
#include <algorithm> //has sort

#include <conio.h>

#include "BG_Winsock.h"
#include "BG_Utility.h"


bool g_debug = true;

#define BUFLEN 1510  
byte _MSG[1500];
int _MSGLength;
int TxPacketCount = 0;
int RxPacketCount = 0;

char UDPSend_IP[20] = "239.255.1.1";
int UDPSend_Port = 5055;

char TCPListen_IP[20] = "192.168.1.133";
int TCPListen_Port = 30003;

bool bPeriodicList = true;

//forward declarations
void ReadIniFile();
void PrintMenu();
void mainConsoleLoop();

HANDLE ptrToTimerHandle;
void __stdcall MainTimerCallback(PVOID, BOOLEAN);

DWORD WINAPI TCPListenThread(LPVOID lpParam);



int main()
{

    ReadIniFile();

    initialise_winsock();

    const int UPDATE_INTERVAL = 1000; //this is ms 
    CreateTimerQueueTimer(&ptrToTimerHandle, NULL, MainTimerCallback, NULL, 500, UPDATE_INTERVAL, WT_EXECUTEDEFAULT);

    if (EXIT_FAILURE == StartListenThreadOnTCPSocket(TCPListen_IP, TCPListen_Port, TCPListenThread))
    {
        printf("Failed to Start TCP Listen Thread\r\n");
        closeandclean_winsock();
        exit(1);
    }

    OpenUDPSocket(UDPSend_IP, UDPSend_Port);

    PrintMenu();
    mainConsoleLoop();
    
    closeandclean_winsock();
}


DWORD WINAPI TCPListenThread(LPVOID lpParam)
{
    char buffer[1510];
    SOCKET* TCPClientSocket = (SOCKET*)lpParam;

    while (true) {
        int n = ::recv(*TCPClientSocket, buffer, 1510, 0);

        if (SOCKET_ERROR == n) continue;

        if ((n > 0) && (n < 1510))
        {
            buffer[n] = 0;
            
            if (g_debug) printf("%s\r\n", buffer); //doesn't flush without a newline
            fflush(stdout);// Force writing buffer to the stdout
            RxPacketCount++;

            SendUDP(buffer, n);
            TxPacketCount++;
        }
    }
    return 0;
}


void __stdcall MainTimerCallback(PVOID, BOOLEAN)//(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    if (bPeriodicList)
    {
        bgu_clearConsole();
        printf("Periodic Timer\r\n");
        PrintMenu();
    }

    
}

void ReadIniFile()
{
    //expects ini file like
    /*
    [Form]
        TCPListen_Port = 30154
        TCPListen_IP = "192.168.1.133"

        UDPSend_Port = 4000
        UDPSend_IP = "192.168.1.255"
     */


    printf("Attempting to read .ini file\r\n");
    LPCTSTR inipath = L".\\ADSBtoCAT21.ini";
    if (!bgu_FileExists(inipath))
    {
        std::cout << "no ini file ";
        std::wcout << inipath;
        std::cout << " found" << std::endl;
    }
    else
    {
        std::cout << "Found ini file ";
        std::wcout << inipath;
        std::cout << " found" << std::endl;
    }

    wchar_t wide_IP[20];
    GetPrivateProfileString(L"Form", L"TCPListen_IP", L"192.168.1.133", wide_IP, 20, inipath);
    size_t charsConverted = 0;
    wcstombs_s(&charsConverted, TCPListen_IP, 20, wide_IP, 20);

    GetPrivateProfileString(L"Form", L"UDPSend_IP", L"239.255.1.1", wide_IP, 20, inipath);
    wcstombs_s(&charsConverted, UDPSend_IP, 20, wide_IP, 20);

    TCPListen_Port = GetPrivateProfileInt(L"Form", L"TCPListen_PORT", 30154, inipath);
    UDPSend_Port = GetPrivateProfileInt(L"Form", L"UDPSend_PORT", 5055, inipath);
    printf("INI: TCPListen: Port: %d  IP: %s \r\n", TCPListen_Port, TCPListen_IP);
    printf("INI: UDP_Send: Port: %d  IP: %s \r\n", UDPSend_Port, UDPSend_IP);

    printf("Done reading .ini file\r\n----------------------\r\n");


}


void PrintMenu()
{
    bool bMODERN = false; //just an example
    printf("Console Template. V%f Compiled %s\r\n", BMG_VERSION, __DATE__);
    
    printf("Visual Studio version:%f\r\n", _MSC_VER);

    if (bMODERN) printf("Generating Modern CAT21\r\n");
    else printf("Generating SITAWARE CAT21\r\n");
    printf("Receiving TCP via Telnet on %s:%d\r\n", TCPListen_IP, TCPListen_Port);
    printf("Transmitting UDP on %s:%d\r\n", UDPSend_IP, UDPSend_Port);
    printf("Packets Rx: %d  Packets Tx: %d\r\n", RxPacketCount, TxPacketCount);

    printf("Commands:\t\n----------\r\nx - eXit\r\ns - Sort By\r\nt - CAT21 Type: Modern or SITAWARE\r\nl - List\r\np - Periodic List\r\nd - debug\r\n");
    printf("This app is always running even if display is not refreshing\r\n");
}

void mainConsoleLoop()
{
    bool done = false;
    while (!done)
    {
        if (_kbhit() != 0)
        {
            int ch = _getch();
            switch (ch)
            {
            case 'd':
            case 'D':
                g_debug = !g_debug;
                printf("Debug is %d\r\n", g_debug);
                break;
            case 'l':
            case 'L':
                //ShowADSBList();
                break;
            case 'p':
            case 'P':
                bPeriodicList = !bPeriodicList;
                if (bPeriodicList) printf("Periodic List On\r\n");
                else printf("Periodic List Off\r\n");
                break;

            case 'x':
            case 'X':
                done = true;
                break;

            default:
                PrintMenu();
            }
            printf("Press h for help\r\n");
        }
    }
}
