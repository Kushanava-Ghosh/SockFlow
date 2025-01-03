#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <chrono>
#include <thread>
#include <string>
#include <mutex>
#include <filesystem>
#include <sys/stat.h>
#include <iomanip>
#include <cstdio>
#include <signal.h>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

// #define PORT 8080
string IP = "";
int PORT;

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

string rem_quotes(string filePath)
{
    if(filePath.size() > 1 && filePath.front() == '"' && filePath.back() == '"')
    return filePath.substr(1, filePath.size()-2);
    return filePath;
}

int main()
{
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buff[1024];
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

    string IPP;
    while(1)
    {
        cout << "[.] Enter the IP address and Port of the server (format: <IP>:<Port>): ";
        SetConsoleTextAttribute(out, 0x0A);
        cin >> IPP;
        SetConsoleTextAttribute(out, 0x07);
        size_t colonPos = IPP.find(':');
        if(colonPos == string::npos) 
        {
            SetConsoleTextAttribute(out, 0x04);
            cerr << "[-] Invalid format. Use <IP>:<Port>\n";
            SetConsoleTextAttribute(out, 0x07);
            continue;
        }
        IP = IPP.substr(0, colonPos);
        PORT = stoi(IPP.substr(colonPos + 1));
        break;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(clientSocket == INVALID_SOCKET)
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

    if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        SetConsoleTextAttribute(out, 0x04);
        cerr << "[-] Connection to server failed" << endl;
        SetConsoleTextAttribute(out, 0x07);
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    SetConsoleTextAttribute(out, 0x0A);
    cout << "[+] Connected to Server [";
    SetConsoleTextAttribute(out, 0x0B);
    cout << IP << ":" << PORT;
    SetConsoleTextAttribute(out, 0x0A);
    cout << "]" << endl;
    SetConsoleTextAttribute(out, 0x07);
    getchar();

    while(1)
    {
        string filePath;
        cout << "[.] Enter the path of the file to send or simply drag the file: ";
        SetConsoleTextAttribute(out, 0x06);
        getline(cin, filePath);
        SetConsoleTextAttribute(out, 0x07);
        filePath = rem_quotes(filePath);

        struct stat fileStat;
        if(stat(filePath.c_str(), &fileStat) != 0) 
        {
            SetConsoleTextAttribute(out, 0x04);
            cerr << "[-] Failed to get file size." << endl;
            SetConsoleTextAttribute(out, 0x07);
            closesocket(clientSocket);
            WSACleanup();
            continue;
        }

        long totalFileSize = fileStat.st_size, totalBytesSent = 0, SIZE = 1024;
        string fs = to_string(totalFileSize);

        string fileName = filesystem::path(filePath).filename().string();
        fs = fs + "$" + fileName;

        if(send(clientSocket, fs.c_str(), fs.size() + 1, 0) == SOCKET_ERROR)
        {
            SetConsoleTextAttribute(out, 0x04);
            cerr << "[-] Failed to send file name & size." << endl;
            SetConsoleTextAttribute(out, 0x07);
            break;
        }

        FILE* inFile = fopen(filePath.c_str(), "rb");
        if(!inFile) 
        {
            SetConsoleTextAttribute(out, 0x04);
            cerr << "[-] Failed to open file for reading." << endl;
            SetConsoleTextAttribute(out, 0x07);
            continue;
        }
    
        size_t bytesRead;
        ShowConsoleCursor(false);
        int seq = 0, update = 0;
        while((bytesRead = fread(buff, 1, 1024, inFile)) > 0)
        {
            if(send(clientSocket, buff, bytesRead, 0) == SOCKET_ERROR)
            {
                SetConsoleTextAttribute(out, 0x04);
                cerr << "[-] Send failed." << endl;
                SetConsoleTextAttribute(out, 0x07);
                break;
            }
            totalBytesSent += bytesRead;
            double progress = (double)totalBytesSent / totalFileSize;
            if(totalBytesSent == totalFileSize)
            seq = 4;
            progbar(progress, seq);
            update = (update + 1)%7;
            if(!update)
            seq = (seq + 1)%4;
        }

        ShowConsoleCursor(true);
        const char* Marker = "EOF";
        if(send(clientSocket, Marker, strlen(Marker), 0) == SOCKET_ERROR)
        {
            SetConsoleTextAttribute(out, 0x04);
            cerr << "[-] Failed to send EOF marker." << endl;
            SetConsoleTextAttribute(out, 0x07);
        }

        SetConsoleTextAttribute(out, 0x0A);
        cout << endl << "[+] File sent successfully: ";
        SetConsoleTextAttribute(out, 0x06);
        cout << fileName << endl;
        SetConsoleTextAttribute(out, 0x07);
        fclose(inFile);
    }
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
