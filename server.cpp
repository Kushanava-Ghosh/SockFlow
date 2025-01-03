#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <chrono>
#include <thread>
#include <string>
#include <mutex>
#include <iomanip>
#include <cstdio>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
string IP = "";

void ShowConsoleCursor(bool flag)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = flag;
    SetConsoleCursorInfo(out, &cursorInfo);
}

void progbar(double progress, int seq) 
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    int barWidth = 50;
    SetConsoleTextAttribute(out, 0x0B);
    char spinner[] = {'\\', '|', '/', '-', '*'};
    cout << "[" << spinner[seq] << "] [";
    int pos = barWidth * progress;
    for(int i = 0; i < barWidth; i++) 
    {
        SetConsoleTextAttribute(out, 0x0A);
        if(i < pos)
        cout << "=";
        else if(i == pos)
        cout << ">";
        else
        cout << " ";
    }
    SetConsoleTextAttribute(out, 0x0B);
    cout << "] " << fixed << setprecision(2) << progress * 100.0 << " %\r";
    SetConsoleTextAttribute(out, 0x07);
    cout.flush();
}

int main()
{
    WSADATA wsaData;
    SOCKET serverSocket;
    struct sockaddr_in serverAddr;
    char buff[1024], hostname[1024];
    int bytesRec;
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    cout << "Copyright (c) 2024 Kushanava Ghosh. All Rights Reserved." << endl;

        cout << "  .--.--.                             ,-.     ,---,.  ,--,                          " << endl; 
    cout << " /  /    '.                       ,--/ /|   ,'  .' |,--.'|                          " << endl; 
    cout << "|  :  /`. /    ,---.            ,--. :/ | ,---.'   ||  | :     ,---.           .---." << endl; 
    cout << ";  |  |--`    '   ,'\\           :  : ' /  |   |   .':  : '    '   ,'\\         /. ./|" << endl; 
    cout << "|  :  ;_     /   /   |   ,---.  |  '  /   :   :  :  |  ' |   /   /   |     .-'-. ' |" << endl; 
    cout << " \\  \\    `. .   ; ,. :  /     \\ '  |  :   :   |  |-,'  | |  .   ; ,. :    /___/ \\: |" << endl; 
    cout << "  `----.   \'    | |: : /    / ' |  |   \\  |   :  ;/||  | :  '   | |: : .-'.. '   ' ." << endl; 
    cout << "  __ \\  \\  |'   | .; :.    ' /  '  : |. \\ |   |   .''  : |__'   | .; :/___/ \\:     '" << endl; 
    cout << " /  /`--'  /|   :    |'   ; :__ |  | ' \\ \'   :  '  |  | '.'|   :    |.   \\  ' .\\   " << endl; 
    cout << "'--'.     /  \\   \\  / '   | '.'|'  : |--' |   |  |  ;  :    ;\\   \\  /  \\   \\   ' \\ |" << endl; 
    cout << "  `--'---'    `----'  |   :    :;  |,'    |   :  \\  |  ,   /  `----'    \\   \\  |--\" " << endl; 
    cout << "                       \\   \\  / '--'      |   | ,'   ---`-'              \\   \\ |    " << endl; 
    cout << "                        `----'            `----'                          '---\"     " << endl; 

    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        SetConsoleTextAttribute(out, 0x04);
        cerr << "[-] WSAStartup Failed" << endl;
        SetConsoleTextAttribute(out, 0x07);
        return 1;
    }
    
    if(gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
    {
        SetConsoleTextAttribute(out, 0x04);
        cerr << "[-] Failed to get host name.\n";
        SetConsoleTextAttribute(out, 0x07);
        WSACleanup();
        return 1;
    }

    addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* addrInfo;
    if(getaddrinfo(hostname, nullptr, &hints, &addrInfo) != 0) 
    {
        SetConsoleTextAttribute(out, 0x04);
        cerr << "[-] Failed to resolve host name to IP.\n";
        SetConsoleTextAttribute(out, 0x07);
        WSACleanup();
        return 1;
    }

    for(addrinfo* ptr = addrInfo; ptr != nullptr; ptr = ptr->ai_next) 
    {
        sockaddr_in* sockaddrIPv4 = reinterpret_cast<sockaddr_in*>(ptr->ai_addr);
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sockaddrIPv4->sin_addr, ipStr, sizeof(ipStr));
        IP = ipStr;
    }

    freeaddrinfo(addrInfo);

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket == INVALID_SOCKET)
    {
        SetConsoleTextAttribute(out, 0x04);
        cerr << "[-] Socket Creation Failed" << endl;
        SetConsoleTextAttribute(out, 0x07);
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP.c_str());
    serverAddr.sin_port = htons(PORT);

    if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        SetConsoleTextAttribute(out, 0x04);
        cerr << "[-] Bind Failed" << endl;
        SetConsoleTextAttribute(out, 0x07);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if(listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        SetConsoleTextAttribute(out, 0x04);
        cerr << "[-] Listen Failed" << endl;
        SetConsoleTextAttribute(out, 0x07);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    SetConsoleTextAttribute(out, 0x0A);
    cout << "[+] Server listening on ";
    SetConsoleTextAttribute(out, 0x0B);
    cout << IP << ":" << PORT;
    SetConsoleTextAttribute(out, 0x07);
    cout << "..." << endl;
    SetConsoleTextAttribute(out, 0x01);
    cout << "[*] Enter this <IP:PORT> into the client machine to connect and share files" << endl;
    SetConsoleTextAttribute(out, 0x07);

    while(1)
    {
        cout << "[.] Waiting for Client to Connect...." << endl;
        sockaddr_in clientAddr;
        int addrlen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrlen);

        if(clientSocket == INVALID_SOCKET)
        {
            SetConsoleTextAttribute(out, 0x04);
            cerr << "[-] Accept Failed" << endl;
            SetConsoleTextAttribute(out, 0x07);
            continue;
        }

        SetConsoleTextAttribute(out, 0x0A);
        cout << "[+] Client Connected..." << endl;
        SetConsoleTextAttribute(out, 0x07);

        while(1)
        {
            char fileDetail[1024], fileName[1024], fileSize[1024];
            int init = recv(clientSocket, fileDetail, 1024, 0);

            if(init == 0)
            {
                SetConsoleTextAttribute(out, 0x04);
                cerr << "[-] Failed to receive file name & size." << endl;
                SetConsoleTextAttribute(out, 0x07);
                break;
            }
            else if(init < 0)
            {
                SetConsoleTextAttribute(out, 0x04);
                cerr << "[-] Disconnected from Client..." << endl;
                SetConsoleTextAttribute(out, 0x07);
                break;
            }

            char* sep = strchr(fileDetail, '$');
            *sep = '\0';
            strcpy(fileSize, fileDetail);
            strcpy(fileName, sep+1);

            SetConsoleTextAttribute(out, 0x0A);
            cout << "[+] Receiving File: ";
            SetConsoleTextAttribute(out, 0x06);
            cout << fileName << endl;
            SetConsoleTextAttribute(out, 0x07);

            FILE* outFile = fopen(fileName, "wb");
            if(!outFile)
            {
                SetConsoleTextAttribute(out, 0x04);
                cerr << "[-] Failed to open file for writing." << endl;
                SetConsoleTextAttribute(out, 0x07);
                continue;
            }

            long totalBytesRec = 0, totalFileSize = strtol(fileSize, NULL, 10);
            int seq = 0, update = 0;
            ShowConsoleCursor(false);
            while(1)
            {
                bytesRec = recv(clientSocket, buff, 1024, 0);
                if(bytesRec <= 0)
                break;

                if(bytesRec >= 3 && memcmp(buff + bytesRec - 3, "EOF", 3) == 0)
                {
                    fwrite(buff, 1, bytesRec - 3, outFile);
                    totalBytesRec += bytesRec - 3;
                    double progress = (double)totalBytesRec / totalFileSize;
                    if(totalBytesRec == totalFileSize)
                    seq = 4;
                    progbar(progress, seq);
                    break;
                }
                fwrite(buff, 1, bytesRec, outFile);
                totalBytesRec += bytesRec;
                double progress = (double)totalBytesRec / totalFileSize;
                if(totalBytesRec == totalFileSize)
                seq = 4;
                progbar(progress, seq);
                update = (update + 1)%7;
                if(!update)
                seq = (seq + 1)%4;
            }
            fclose(outFile);
            ShowConsoleCursor(true);

            if(bytesRec == SOCKET_ERROR)
            {
                SetConsoleTextAttribute(out, 0x04);
                cerr << "[-] Receive Failed" << endl;
                SetConsoleTextAttribute(out, 0x07);
                continue;
            }

            SetConsoleTextAttribute(out, 0x0A);
            cout << endl << "[+] File received successfully: ";
            SetConsoleTextAttribute(out, 0x06);
            cout << fileName << endl;
            SetConsoleTextAttribute(out, 0x07);
        }
        closesocket(clientSocket);
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
